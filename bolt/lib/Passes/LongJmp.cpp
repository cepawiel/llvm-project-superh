//===- bolt/Passes/LongJmp.cpp --------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the LongJmpPass class.
//
//===----------------------------------------------------------------------===//

#include "bolt/Passes/LongJmp.h"
#include "bolt/Core/ParallelUtilities.h"
#include "bolt/Utils/CommandLineOpts.h"
#include "llvm/Support/MathExtras.h"

#define DEBUG_TYPE "longjmp"

using namespace llvm;

namespace opts {
extern cl::OptionCategory BoltCategory;
extern cl::OptionCategory BoltOptCategory;
extern llvm::cl::opt<unsigned> AlignText;
extern cl::opt<unsigned> AlignFunctions;
extern cl::opt<bool> UseOldText;
extern cl::opt<bool> HotFunctionsAtEnd;

static cl::opt<bool> GroupStubs("group-stubs",
                                cl::desc("share stubs across functions"),
                                cl::init(true), cl::cat(BoltOptCategory));
}

namespace llvm {
namespace bolt {

constexpr unsigned ColdFragAlign = 16;

static void relaxStubToShortJmp(BinaryBasicBlock &StubBB, const MCSymbol *Tgt) {
  const BinaryContext &BC = StubBB.getFunction()->getBinaryContext();
  InstructionListType Seq;
  BC.MIB->createShortJmp(Seq, Tgt, BC.Ctx.get());
  StubBB.clear();
  StubBB.addInstructions(Seq.begin(), Seq.end());
}

static void relaxStubToLongJmp(BinaryBasicBlock &StubBB, const MCSymbol *Tgt) {
  const BinaryContext &BC = StubBB.getFunction()->getBinaryContext();
  InstructionListType Seq;
  BC.MIB->createLongJmp(Seq, Tgt, BC.Ctx.get());
  StubBB.clear();
  StubBB.addInstructions(Seq.begin(), Seq.end());
}

static BinaryBasicBlock *getBBAtHotColdSplitPoint(BinaryFunction &Func) {
  if (!Func.isSplit() || Func.empty())
    return nullptr;

  assert(!(*Func.begin()).isCold() && "Entry cannot be cold");
  for (auto I = Func.getLayout().block_begin(),
            E = Func.getLayout().block_end();
       I != E; ++I) {
    auto Next = std::next(I);
    if (Next != E && (*Next)->isCold())
      return *I;
  }
  llvm_unreachable("No hot-cold split point found");
}

static bool mayNeedStub(const BinaryContext &BC, const MCInst &Inst) {
  return (BC.MIB->isBranch(Inst) || BC.MIB->isCall(Inst)) &&
         !BC.MIB->isIndirectBranch(Inst) && !BC.MIB->isIndirectCall(Inst);
}

std::pair<std::unique_ptr<BinaryBasicBlock>, MCSymbol *>
LongJmpPass::createNewStub(BinaryBasicBlock &SourceBB, const MCSymbol *TgtSym,
                           bool TgtIsFunc, uint64_t AtAddress) {
  BinaryFunction &Func = *SourceBB.getFunction();
  const BinaryContext &BC = Func.getBinaryContext();
  const bool IsCold = SourceBB.isCold();
  MCSymbol *StubSym = BC.Ctx->createNamedTempSymbol("Stub");
  std::unique_ptr<BinaryBasicBlock> StubBB = Func.createBasicBlock(StubSym);
  MCInst Inst;
  BC.MIB->createUncondBranch(Inst, TgtSym, BC.Ctx.get());
  if (TgtIsFunc)
    BC.MIB->convertJmpToTailCall(Inst);
  StubBB->addInstruction(Inst);
  StubBB->setExecutionCount(0);

  // Register this in stubs maps
  auto registerInMap = [&](StubGroupsTy &Map) {
    StubGroupTy &StubGroup = Map[TgtSym];
    StubGroup.insert(
        llvm::lower_bound(
            StubGroup, std::make_pair(AtAddress, nullptr),
            [&](const std::pair<uint64_t, BinaryBasicBlock *> &LHS,
                const std::pair<uint64_t, BinaryBasicBlock *> &RHS) {
              return LHS.first < RHS.first;
            }),
        std::make_pair(AtAddress, StubBB.get()));
  };

  Stubs[&Func].insert(StubBB.get());
  StubBits[StubBB.get()] = BC.MIB->getUncondBranchEncodingSize();
  if (IsCold) {
    registerInMap(ColdLocalStubs[&Func]);
    if (opts::GroupStubs && TgtIsFunc)
      registerInMap(ColdStubGroups);
    ++NumColdStubs;
  } else {
    registerInMap(HotLocalStubs[&Func]);
    if (opts::GroupStubs && TgtIsFunc)
      registerInMap(HotStubGroups);
    ++NumHotStubs;
  }

  return std::make_pair(std::move(StubBB), StubSym);
}

BinaryBasicBlock *LongJmpPass::lookupStubFromGroup(
    const StubGroupsTy &StubGroups, const BinaryFunction &Func,
    const MCInst &Inst, const MCSymbol *TgtSym, uint64_t DotAddress) const {
  const BinaryContext &BC = Func.getBinaryContext();
  auto CandidatesIter = StubGroups.find(TgtSym);
  if (CandidatesIter == StubGroups.end())
    return nullptr;
  const StubGroupTy &Candidates = CandidatesIter->second;
  if (Candidates.empty())
    return nullptr;
  auto Cand = llvm::lower_bound(
      Candidates, std::make_pair(DotAddress, nullptr),
      [&](const std::pair<uint64_t, BinaryBasicBlock *> &LHS,
          const std::pair<uint64_t, BinaryBasicBlock *> &RHS) {
        return LHS.first < RHS.first;
      });
  if (Cand == Candidates.end()) {
    Cand = std::prev(Cand);
  } else if (Cand != Candidates.begin()) {
    const StubTy *LeftCand = std::prev(Cand);
    if (Cand->first - DotAddress > DotAddress - LeftCand->first)
      Cand = LeftCand;
  }
  int BitsAvail = BC.MIB->getPCRelEncodingSize(Inst) - 1;
  assert(BitsAvail < 63 && "PCRelEncodingSize is too large to use int64_t to"
                           "check for out-of-bounds.");
  int64_t MaxVal = (1ULL << BitsAvail) - 1;
  int64_t MinVal = -(1ULL << BitsAvail);
  uint64_t PCRelTgtAddress = Cand->first;
  int64_t PCOffset = (int64_t)(PCRelTgtAddress - DotAddress);

  LLVM_DEBUG({
    if (Candidates.size() > 1)
      dbgs() << "Considering stub group with " << Candidates.size()
             << " candidates. DotAddress is " << Twine::utohexstr(DotAddress)
             << ", chosen candidate address is "
             << Twine::utohexstr(Cand->first) << "\n";
  });
  return (PCOffset < MinVal || PCOffset > MaxVal) ? nullptr : Cand->second;
}

BinaryBasicBlock *
LongJmpPass::lookupGlobalStub(const BinaryBasicBlock &SourceBB,
                              const MCInst &Inst, const MCSymbol *TgtSym,
                              uint64_t DotAddress) const {
  const BinaryFunction &Func = *SourceBB.getFunction();
  const StubGroupsTy &StubGroups =
      SourceBB.isCold() ? ColdStubGroups : HotStubGroups;
  return lookupStubFromGroup(StubGroups, Func, Inst, TgtSym, DotAddress);
}

BinaryBasicBlock *LongJmpPass::lookupLocalStub(const BinaryBasicBlock &SourceBB,
                                               const MCInst &Inst,
                                               const MCSymbol *TgtSym,
                                               uint64_t DotAddress) const {
  const BinaryFunction &Func = *SourceBB.getFunction();
  const DenseMap<const BinaryFunction *, StubGroupsTy> &StubGroups =
      SourceBB.isCold() ? ColdLocalStubs : HotLocalStubs;
  const auto Iter = StubGroups.find(&Func);
  if (Iter == StubGroups.end())
    return nullptr;
  return lookupStubFromGroup(Iter->second, Func, Inst, TgtSym, DotAddress);
}

std::unique_ptr<BinaryBasicBlock>
LongJmpPass::replaceTargetWithStub(BinaryBasicBlock &BB, MCInst &Inst,
                                   uint64_t DotAddress,
                                   uint64_t StubCreationAddress) {
  const BinaryFunction &Func = *BB.getFunction();
  const BinaryContext &BC = Func.getBinaryContext();
  std::unique_ptr<BinaryBasicBlock> NewBB;
  const MCSymbol *TgtSym = BC.MIB->getTargetSymbol(Inst);
  assert(TgtSym && "getTargetSymbol failed");

  BinaryBasicBlock::BinaryBranchInfo BI{0, 0};
  BinaryBasicBlock *TgtBB = BB.getSuccessor(TgtSym, BI);
  auto LocalStubsIter = Stubs.find(&Func);

  // If already using stub and the stub is from another function, create a local
  // stub, since the foreign stub is now out of range
  if (!TgtBB) {
    auto SSIter = SharedStubs.find(TgtSym);
    if (SSIter != SharedStubs.end()) {
      TgtSym = BC.MIB->getTargetSymbol(*SSIter->second->begin());
      --NumSharedStubs;
    }
  } else if (LocalStubsIter != Stubs.end() &&
             LocalStubsIter->second.count(TgtBB)) {
    // The TgtBB and TgtSym now are the local out-of-range stub and its label.
    // So, we are attempting to restore BB to its previous state without using
    // this stub.
    TgtSym = BC.MIB->getTargetSymbol(*TgtBB->begin());
    assert(TgtSym &&
           "First instruction is expected to contain a target symbol.");
    BinaryBasicBlock *TgtBBSucc = TgtBB->getSuccessor(TgtSym, BI);

    // TgtBB might have no successor. e.g. a stub for a function call.
    if (TgtBBSucc) {
      BB.replaceSuccessor(TgtBB, TgtBBSucc, BI.Count, BI.MispredictedCount);
      assert(TgtBB->getExecutionCount() >= BI.Count &&
             "At least equal or greater than the branch count.");
      TgtBB->setExecutionCount(TgtBB->getExecutionCount() - BI.Count);
    }

    TgtBB = TgtBBSucc;
  }

  BinaryBasicBlock *StubBB = lookupLocalStub(BB, Inst, TgtSym, DotAddress);
  // If not found, look it up in globally shared stub maps if it is a function
  // call (TgtBB is not set)
  if (!StubBB && !TgtBB) {
    StubBB = lookupGlobalStub(BB, Inst, TgtSym, DotAddress);
    if (StubBB) {
      SharedStubs[StubBB->getLabel()] = StubBB;
      ++NumSharedStubs;
    }
  }
  MCSymbol *StubSymbol = StubBB ? StubBB->getLabel() : nullptr;

  if (!StubBB) {
    std::tie(NewBB, StubSymbol) =
        createNewStub(BB, TgtSym, /*is func?*/ !TgtBB, StubCreationAddress);
    StubBB = NewBB.get();
  }

  // Local branch
  if (TgtBB) {
    uint64_t OrigCount = BI.Count;
    uint64_t OrigMispreds = BI.MispredictedCount;
    BB.replaceSuccessor(TgtBB, StubBB, OrigCount, OrigMispreds);
    StubBB->setExecutionCount(StubBB->getExecutionCount() + OrigCount);
    if (NewBB) {
      StubBB->addSuccessor(TgtBB, OrigCount, OrigMispreds);
      StubBB->setIsCold(BB.isCold());
    }
    // Call / tail call
  } else {
    StubBB->setExecutionCount(StubBB->getExecutionCount() +
                              BB.getExecutionCount());
    if (NewBB) {
      assert(TgtBB == nullptr);
      StubBB->setIsCold(BB.isCold());
      // Set as entry point because this block is valid but we have no preds
      StubBB->getFunction()->addEntryPoint(*StubBB);
    }
  }
  BC.MIB->replaceBranchTarget(Inst, StubSymbol, BC.Ctx.get());

  return NewBB;
}

void LongJmpPass::updateStubGroups() {
  auto update = [&](StubGroupsTy &StubGroups) {
    for (auto &KeyVal : StubGroups) {
      for (StubTy &Elem : KeyVal.second)
        Elem.first = BBAddresses[Elem.second];
      llvm::sort(KeyVal.second, llvm::less_first());
    }
  };

  for (auto &KeyVal : HotLocalStubs)
    update(KeyVal.second);
  for (auto &KeyVal : ColdLocalStubs)
    update(KeyVal.second);
  update(HotStubGroups);
  update(ColdStubGroups);
}

void LongJmpPass::tentativeBBLayout(const BinaryFunction &Func) {
  const BinaryContext &BC = Func.getBinaryContext();
  uint64_t HotDot = HotAddresses[&Func];
  uint64_t ColdDot = ColdAddresses[&Func];
  bool Cold = false;
  for (const BinaryBasicBlock *BB : Func.getLayout().blocks()) {
    if (Cold || BB->isCold()) {
      Cold = true;
      BBAddresses[BB] = ColdDot;
      ColdDot += BC.computeCodeSize(BB->begin(), BB->end());
    } else {
      BBAddresses[BB] = HotDot;
      HotDot += BC.computeCodeSize(BB->begin(), BB->end());
    }
  }
}

uint64_t LongJmpPass::tentativeLayoutRelocColdPart(
    const BinaryContext &BC, std::vector<BinaryFunction *> &SortedFunctions,
    uint64_t DotAddress) {
  DotAddress = alignTo(DotAddress, llvm::Align(opts::AlignFunctions));
  for (BinaryFunction *Func : SortedFunctions) {
    if (!Func->isSplit())
      continue;
    DotAddress = alignTo(DotAddress, Func->getMinAlignment());
    uint64_t Pad =
        offsetToAlignment(DotAddress, llvm::Align(Func->getAlignment()));
    if (Pad <= Func->getMaxColdAlignmentBytes())
      DotAddress += Pad;
    ColdAddresses[Func] = DotAddress;
    LLVM_DEBUG(dbgs() << Func->getPrintName() << " cold tentative: "
                      << Twine::utohexstr(DotAddress) << "\n");
    DotAddress += Func->estimateColdSize();
    DotAddress = alignTo(DotAddress, Func->getConstantIslandAlignment());
    DotAddress += Func->estimateConstantIslandSize();
  }
  return DotAddress;
}

uint64_t LongJmpPass::tentativeLayoutRelocMode(
    const BinaryContext &BC, std::vector<BinaryFunction *> &SortedFunctions,
    uint64_t DotAddress) {
  // Compute hot cold frontier
  int64_t LastHotIndex = -1u;
  uint32_t CurrentIndex = 0;
  if (opts::HotFunctionsAtEnd) {
    for (BinaryFunction *BF : SortedFunctions) {
      if (BF->hasValidIndex()) {
        LastHotIndex = CurrentIndex;
        break;
      }

      ++CurrentIndex;
    }
  } else {
    for (BinaryFunction *BF : SortedFunctions) {
      if (!BF->hasValidIndex()) {
        LastHotIndex = CurrentIndex;
        break;
      }

      ++CurrentIndex;
    }
  }

  // Hot
  CurrentIndex = 0;
  bool ColdLayoutDone = false;
  auto runColdLayout = [&]() {
    DotAddress = tentativeLayoutRelocColdPart(BC, SortedFunctions, DotAddress);
    ColdLayoutDone = true;
    if (opts::HotFunctionsAtEnd)
      DotAddress = alignTo(DotAddress, opts::AlignText);
  };
  for (BinaryFunction *Func : SortedFunctions) {
    if (!BC.shouldEmit(*Func)) {
      HotAddresses[Func] = Func->getAddress();
      continue;
    }

    if (!ColdLayoutDone && CurrentIndex >= LastHotIndex)
      runColdLayout();

    DotAddress = alignTo(DotAddress, Func->getMinAlignment());
    uint64_t Pad =
        offsetToAlignment(DotAddress, llvm::Align(Func->getAlignment()));
    if (Pad <= Func->getMaxAlignmentBytes())
      DotAddress += Pad;
    HotAddresses[Func] = DotAddress;
    LLVM_DEBUG(dbgs() << Func->getPrintName() << " tentative: "
                      << Twine::utohexstr(DotAddress) << "\n");
    if (!Func->isSplit())
      DotAddress += Func->estimateSize();
    else
      DotAddress += Func->estimateHotSize();

    DotAddress = alignTo(DotAddress, Func->getConstantIslandAlignment());
    DotAddress += Func->estimateConstantIslandSize();
    ++CurrentIndex;
  }

  // Ensure that tentative code layout always runs for cold blocks.
  if (!ColdLayoutDone)
    runColdLayout();

  // BBs
  for (BinaryFunction *Func : SortedFunctions)
    tentativeBBLayout(*Func);

  return DotAddress;
}

void LongJmpPass::tentativeLayout(
    const BinaryContext &BC, std::vector<BinaryFunction *> &SortedFunctions) {
  uint64_t DotAddress = BC.LayoutStartAddress;

  if (!BC.HasRelocations) {
    for (BinaryFunction *Func : SortedFunctions) {
      HotAddresses[Func] = Func->getAddress();
      DotAddress = alignTo(DotAddress, ColdFragAlign);
      ColdAddresses[Func] = DotAddress;
      if (Func->isSplit())
        DotAddress += Func->estimateColdSize();
      tentativeBBLayout(*Func);
    }

    return;
  }

  // Relocation mode
  uint64_t EstimatedTextSize = 0;
  if (opts::UseOldText) {
    EstimatedTextSize = tentativeLayoutRelocMode(BC, SortedFunctions, 0);

    // Initial padding
    if (EstimatedTextSize <= BC.OldTextSectionSize) {
      DotAddress = BC.OldTextSectionAddress;
      uint64_t Pad =
          offsetToAlignment(DotAddress, llvm::Align(opts::AlignText));
      if (Pad + EstimatedTextSize <= BC.OldTextSectionSize) {
        DotAddress += Pad;
      }
    }
  }

  if (!EstimatedTextSize || EstimatedTextSize > BC.OldTextSectionSize)
    DotAddress = alignTo(BC.LayoutStartAddress, opts::AlignText);

  tentativeLayoutRelocMode(BC, SortedFunctions, DotAddress);
}

bool LongJmpPass::usesStub(const BinaryFunction &Func,
                           const MCInst &Inst) const {
  const MCSymbol *TgtSym = Func.getBinaryContext().MIB->getTargetSymbol(Inst);
  const BinaryBasicBlock *TgtBB = Func.getBasicBlockForLabel(TgtSym);
  auto Iter = Stubs.find(&Func);
  if (Iter != Stubs.end())
    return Iter->second.count(TgtBB);
  return false;
}

uint64_t LongJmpPass::getSymbolAddress(const BinaryContext &BC,
                                       const MCSymbol *Target,
                                       const BinaryBasicBlock *TgtBB) const {
  if (TgtBB) {
    auto Iter = BBAddresses.find(TgtBB);
    assert(Iter != BBAddresses.end() && "Unrecognized BB");
    return Iter->second;
  }
  uint64_t EntryID = 0;
  const BinaryFunction *TargetFunc = BC.getFunctionForSymbol(Target, &EntryID);
  auto Iter = HotAddresses.find(TargetFunc);
  if (Iter == HotAddresses.end() || (TargetFunc && EntryID)) {
    // Look at BinaryContext's resolution for this symbol - this is a symbol not
    // mapped to a BinaryFunction
    ErrorOr<uint64_t> ValueOrError = BC.getSymbolValue(*Target);
    assert(ValueOrError && "Unrecognized symbol");
    return *ValueOrError;
  }
  return Iter->second;
}

Error LongJmpPass::relaxStub(BinaryBasicBlock &StubBB, bool &Modified) {
  const BinaryFunction &Func = *StubBB.getFunction();
  const BinaryContext &BC = Func.getBinaryContext();
  const int Bits = StubBits[&StubBB];
  // Already working with the largest range?
  if (Bits == static_cast<int>(BC.AsmInfo->getCodePointerSize() * 8))
    return Error::success();

  const static int RangeShortJmp = BC.MIB->getShortJmpEncodingSize();
  const static int RangeSingleInstr = BC.MIB->getUncondBranchEncodingSize();
  const static uint64_t ShortJmpMask = ~((1ULL << RangeShortJmp) - 1);
  const static uint64_t SingleInstrMask =
      ~((1ULL << (RangeSingleInstr - 1)) - 1);

  const MCSymbol *RealTargetSym = BC.MIB->getTargetSymbol(*StubBB.begin());
  const BinaryBasicBlock *TgtBB = Func.getBasicBlockForLabel(RealTargetSym);
  uint64_t TgtAddress = getSymbolAddress(BC, RealTargetSym, TgtBB);
  uint64_t DotAddress = BBAddresses[&StubBB];
  uint64_t PCRelTgtAddress = DotAddress > TgtAddress ? DotAddress - TgtAddress
                                                     : TgtAddress - DotAddress;
  // If it fits in one instruction, do not relax
  if (!(PCRelTgtAddress & SingleInstrMask))
    return Error::success();

  // Fits short jmp
  if (!(PCRelTgtAddress & ShortJmpMask)) {
    if (Bits >= RangeShortJmp)
      return Error::success();

    LLVM_DEBUG(dbgs() << "Relaxing stub to short jump. PCRelTgtAddress = "
                      << Twine::utohexstr(PCRelTgtAddress)
                      << " RealTargetSym = " << RealTargetSym->getName()
                      << "\n");
    relaxStubToShortJmp(StubBB, RealTargetSym);
    StubBits[&StubBB] = RangeShortJmp;
    Modified = true;
    return Error::success();
  }

  // The long jmp uses absolute address on AArch64
  // So we could not use it for PIC binaries
  if (BC.isAArch64() && !BC.HasFixedLoadAddress)
    return createFatalBOLTError(
        "BOLT-ERROR: Unable to relax stub for PIC binary\n");

  LLVM_DEBUG(dbgs() << "Relaxing stub to long jump. PCRelTgtAddress = "
                    << Twine::utohexstr(PCRelTgtAddress)
                    << " RealTargetSym = " << RealTargetSym->getName() << "\n");
  relaxStubToLongJmp(StubBB, RealTargetSym);
  StubBits[&StubBB] = static_cast<int>(BC.AsmInfo->getCodePointerSize() * 8);
  Modified = true;
  return Error::success();
}

bool LongJmpPass::needsStub(const BinaryBasicBlock &BB, const MCInst &Inst,
                            uint64_t DotAddress) const {
  const BinaryFunction &Func = *BB.getFunction();
  const BinaryContext &BC = Func.getBinaryContext();
  const MCSymbol *TgtSym = BC.MIB->getTargetSymbol(Inst);
  assert(TgtSym && "getTargetSymbol failed");

  const BinaryBasicBlock *TgtBB = Func.getBasicBlockForLabel(TgtSym);
  // Check for shared stubs from foreign functions
  if (!TgtBB) {
    auto SSIter = SharedStubs.find(TgtSym);
    if (SSIter != SharedStubs.end())
      TgtBB = SSIter->second;
  }

  int BitsAvail = BC.MIB->getPCRelEncodingSize(Inst) - 1;
  assert(BitsAvail < 63 && "PCRelEncodingSize is too large to use int64_t to"
                           "check for out-of-bounds.");
  int64_t MaxVal = (1ULL << BitsAvail) - 1;
  int64_t MinVal = -(1ULL << BitsAvail);

  uint64_t PCRelTgtAddress = getSymbolAddress(BC, TgtSym, TgtBB);
  int64_t PCOffset = (int64_t)(PCRelTgtAddress - DotAddress);

  return PCOffset < MinVal || PCOffset > MaxVal;
}

Error LongJmpPass::relax(BinaryFunction &Func, bool &Modified) {
  const BinaryContext &BC = Func.getBinaryContext();

  assert(BC.isAArch64() && "Unsupported arch");
  constexpr int InsnSize = 4; // AArch64
  std::vector<std::pair<BinaryBasicBlock *, std::unique_ptr<BinaryBasicBlock>>>
      Insertions;

  BinaryBasicBlock *Frontier = getBBAtHotColdSplitPoint(Func);
  uint64_t FrontierAddress = Frontier ? BBAddresses[Frontier] : 0;
  if (FrontierAddress)
    FrontierAddress += Frontier->getNumNonPseudos() * InsnSize;

  // Add necessary stubs for branch targets we know we can't fit in the
  // instruction
  for (BinaryBasicBlock &BB : Func) {
    uint64_t DotAddress = BBAddresses[&BB];
    // Stubs themselves are relaxed on the next loop
    if (Stubs[&Func].count(&BB))
      continue;

    for (MCInst &Inst : BB) {
      if (BC.MIB->isPseudo(Inst))
        continue;

      if (!mayNeedStub(BC, Inst)) {
        DotAddress += InsnSize;
        continue;
      }

      // Check and relax direct branch or call
      if (!needsStub(BB, Inst, DotAddress)) {
        DotAddress += InsnSize;
        continue;
      }
      Modified = true;

      // Insert stubs close to the patched BB if call, but far away from the
      // hot path if a branch, since this branch target is the cold region
      // (but first check that the far away stub will be in range).
      BinaryBasicBlock *InsertionPoint = &BB;
      if (Func.isSimple() && !BC.MIB->isCall(Inst) && FrontierAddress &&
          !BB.isCold()) {
        int BitsAvail = BC.MIB->getPCRelEncodingSize(Inst) - 1;
        uint64_t Mask = ~((1ULL << BitsAvail) - 1);
        assert(FrontierAddress > DotAddress &&
               "Hot code should be before the frontier");
        uint64_t PCRelTgt = FrontierAddress - DotAddress;
        if (!(PCRelTgt & Mask))
          InsertionPoint = Frontier;
      }
      // Always put stubs at the end of the function if non-simple. We can't
      // change the layout of non-simple functions because it has jump tables
      // that we do not control.
      if (!Func.isSimple())
        InsertionPoint = &*std::prev(Func.end());

      // Create a stub to handle a far-away target
      Insertions.emplace_back(InsertionPoint,
                              replaceTargetWithStub(BB, Inst, DotAddress,
                                                    InsertionPoint == Frontier
                                                        ? FrontierAddress
                                                        : DotAddress));

      DotAddress += InsnSize;
    }
  }

  // Relax stubs if necessary
  for (BinaryBasicBlock &BB : Func) {
    if (!Stubs[&Func].count(&BB) || !BB.isValid())
      continue;

    if (auto E = relaxStub(BB, Modified))
      return Error(std::move(E));
  }

  for (std::pair<BinaryBasicBlock *, std::unique_ptr<BinaryBasicBlock>> &Elmt :
       Insertions) {
    if (!Elmt.second)
      continue;
    std::vector<std::unique_ptr<BinaryBasicBlock>> NewBBs;
    NewBBs.emplace_back(std::move(Elmt.second));
    Func.insertBasicBlocks(Elmt.first, std::move(NewBBs), true);
  }

  return Error::success();
}

void LongJmpPass::relaxLocalBranches(BinaryFunction &BF) {
  BinaryContext &BC = BF.getBinaryContext();
  auto &MIB = BC.MIB;

  // Quick path.
  if (!BF.isSplit() && BF.estimateSize() < ShortestJumpSpan)
    return;

  auto isBranchOffsetInRange = [&](const MCInst &Inst, int64_t Offset) {
    const unsigned Bits = MIB->getPCRelEncodingSize(Inst);
    return isIntN(Bits, Offset);
  };

  auto isBlockInRange = [&](const MCInst &Inst, uint64_t InstAddress,
                            const BinaryBasicBlock &BB) {
    const int64_t Offset = BB.getOutputStartAddress() - InstAddress;
    return isBranchOffsetInRange(Inst, Offset);
  };

  // Keep track of *all* function trampolines that are going to be added to the
  // function layout at the end of relaxation.
  std::vector<std::pair<BinaryBasicBlock *, std::unique_ptr<BinaryBasicBlock>>>
      FunctionTrampolines;

  // Function fragments are relaxed independently.
  for (FunctionFragment &FF : BF.getLayout().fragments()) {
    // Fill out code size estimation for the fragment. Use output BB address
    // ranges to store offsets from the start of the function fragment.
    uint64_t CodeSize = 0;
    for (BinaryBasicBlock *BB : FF) {
      BB->setOutputStartAddress(CodeSize);
      CodeSize += BB->estimateSize();
      BB->setOutputEndAddress(CodeSize);
    }

    // Dynamically-updated size of the fragment.
    uint64_t FragmentSize = CodeSize;

    // Size of the trampoline in bytes.
    constexpr uint64_t TrampolineSize = 4;

    // Trampolines created for the fragment. DestinationBB -> TrampolineBB.
    // NB: here we store only the first trampoline created for DestinationBB.
    DenseMap<const BinaryBasicBlock *, BinaryBasicBlock *> FragmentTrampolines;

    // Create a trampoline code after \p BB or at the end of the fragment if BB
    // is nullptr. If \p UpdateOffsets is true, update FragmentSize and offsets
    // for basic blocks affected by the insertion of the trampoline.
    auto addTrampolineAfter = [&](BinaryBasicBlock *BB,
                                  BinaryBasicBlock *TargetBB, uint64_t Count,
                                  bool UpdateOffsets = true) {
      FunctionTrampolines.emplace_back(BB ? BB : FF.back(),
                                       BF.createBasicBlock());
      BinaryBasicBlock *TrampolineBB = FunctionTrampolines.back().second.get();

      MCInst Inst;
      {
        auto L = BC.scopeLock();
        MIB->createUncondBranch(Inst, TargetBB->getLabel(), BC.Ctx.get());
      }
      TrampolineBB->addInstruction(Inst);
      TrampolineBB->addSuccessor(TargetBB, Count);
      TrampolineBB->setExecutionCount(Count);
      const uint64_t TrampolineAddress =
          BB ? BB->getOutputEndAddress() : FragmentSize;
      TrampolineBB->setOutputStartAddress(TrampolineAddress);
      TrampolineBB->setOutputEndAddress(TrampolineAddress + TrampolineSize);
      TrampolineBB->setFragmentNum(FF.getFragmentNum());

      if (!FragmentTrampolines.lookup(TargetBB))
        FragmentTrampolines[TargetBB] = TrampolineBB;

      if (!UpdateOffsets)
        return TrampolineBB;

      FragmentSize += TrampolineSize;

      // If the trampoline was added at the end of the fragment, offsets of
      // other fragments should stay intact.
      if (!BB)
        return TrampolineBB;

      // Update offsets for blocks after BB.
      for (BinaryBasicBlock *IBB : FF) {
        if (IBB->getOutputStartAddress() >= TrampolineAddress) {
          IBB->setOutputStartAddress(IBB->getOutputStartAddress() +
                                     TrampolineSize);
          IBB->setOutputEndAddress(IBB->getOutputEndAddress() + TrampolineSize);
        }
      }

      // Update offsets for trampolines in this fragment that are placed after
      // the new trampoline. Note that trampoline blocks are not part of the
      // function/fragment layout until we add them right before the return
      // from relaxLocalBranches().
      for (auto &Pair : FunctionTrampolines) {
        BinaryBasicBlock *IBB = Pair.second.get();
        if (IBB->getFragmentNum() != TrampolineBB->getFragmentNum())
          continue;
        if (IBB == TrampolineBB)
          continue;
        if (IBB->getOutputStartAddress() >= TrampolineAddress) {
          IBB->setOutputStartAddress(IBB->getOutputStartAddress() +
                                     TrampolineSize);
          IBB->setOutputEndAddress(IBB->getOutputEndAddress() + TrampolineSize);
        }
      }

      return TrampolineBB;
    };

    // Pre-populate trampolines by splitting unconditional branches from the
    // containing basic block.
    for (BinaryBasicBlock *BB : FF) {
      MCInst *Inst = BB->getLastNonPseudoInstr();
      if (!Inst || !MIB->isUnconditionalBranch(*Inst))
        continue;

      const MCSymbol *TargetSymbol = MIB->getTargetSymbol(*Inst);
      BB->eraseInstruction(BB->findInstruction(Inst));
      BB->setOutputEndAddress(BB->getOutputEndAddress() - TrampolineSize);

      BinaryBasicBlock::BinaryBranchInfo BI;
      BinaryBasicBlock *TargetBB = BB->getSuccessor(TargetSymbol, BI);

      BinaryBasicBlock *TrampolineBB =
          addTrampolineAfter(BB, TargetBB, BI.Count, /*UpdateOffsets*/ false);
      BB->replaceSuccessor(TargetBB, TrampolineBB, BI.Count);
    }

    /// Relax the branch \p Inst in basic block \p BB that targets \p TargetBB.
    /// \p InstAddress contains offset of the branch from the start of the
    /// containing function fragment.
    auto relaxBranch = [&](BinaryBasicBlock *BB, MCInst &Inst,
                           uint64_t InstAddress, BinaryBasicBlock *TargetBB) {
      BinaryFunction *BF = BB->getParent();

      // Use branch taken count for optimal relaxation.
      const uint64_t Count = BB->getBranchInfo(*TargetBB).Count;
      assert(Count != BinaryBasicBlock::COUNT_NO_PROFILE &&
             "Expected valid branch execution count");

      // Try to reuse an existing trampoline without introducing any new code.
      BinaryBasicBlock *TrampolineBB = FragmentTrampolines.lookup(TargetBB);
      if (TrampolineBB && isBlockInRange(Inst, InstAddress, *TrampolineBB)) {
        BB->replaceSuccessor(TargetBB, TrampolineBB, Count);
        TrampolineBB->setExecutionCount(TrampolineBB->getExecutionCount() +
                                        Count);
        auto L = BC.scopeLock();
        MIB->replaceBranchTarget(Inst, TrampolineBB->getLabel(), BC.Ctx.get());
        return;
      }

      // For cold branches, check if we can introduce a trampoline at the end
      // of the fragment that is within the branch reach. Note that such
      // trampoline may change address later and become unreachable in which
      // case we will need further relaxation.
      const int64_t OffsetToEnd = FragmentSize - InstAddress;
      if (Count == 0 && isBranchOffsetInRange(Inst, OffsetToEnd)) {
        TrampolineBB = addTrampolineAfter(nullptr, TargetBB, Count);
        BB->replaceSuccessor(TargetBB, TrampolineBB, Count);
        auto L = BC.scopeLock();
        MIB->replaceBranchTarget(Inst, TrampolineBB->getLabel(), BC.Ctx.get());

        return;
      }

      // Insert a new block after the current one and use it as a trampoline.
      TrampolineBB = addTrampolineAfter(BB, TargetBB, Count);

      // If the other successor is a fall-through, invert the condition code.
      const BinaryBasicBlock *const NextBB =
          BF->getLayout().getBasicBlockAfter(BB, /*IgnoreSplits*/ false);
      if (BB->getConditionalSuccessor(false) == NextBB) {
        BB->swapConditionalSuccessors();
        auto L = BC.scopeLock();
        MIB->reverseBranchCondition(Inst, NextBB->getLabel(), BC.Ctx.get());
      } else {
        auto L = BC.scopeLock();
        MIB->replaceBranchTarget(Inst, TrampolineBB->getLabel(), BC.Ctx.get());
      }
      BB->replaceSuccessor(TargetBB, TrampolineBB, Count);
    };

    bool MayNeedRelaxation;
    uint64_t NumIterations = 0;
    do {
      MayNeedRelaxation = false;
      ++NumIterations;
      for (auto BBI = FF.begin(); BBI != FF.end(); ++BBI) {
        BinaryBasicBlock *BB = *BBI;
        uint64_t NextInstOffset = BB->getOutputStartAddress();
        for (MCInst &Inst : *BB) {
          const size_t InstAddress = NextInstOffset;
          if (!MIB->isPseudo(Inst))
            NextInstOffset += 4;

          if (!mayNeedStub(BF.getBinaryContext(), Inst))
            continue;

          const size_t BitsAvailable = MIB->getPCRelEncodingSize(Inst);

          // Span of +/-128MB.
          if (BitsAvailable == LongestJumpBits)
            continue;

          const MCSymbol *TargetSymbol = MIB->getTargetSymbol(Inst);
          BinaryBasicBlock *TargetBB = BB->getSuccessor(TargetSymbol);
          assert(TargetBB &&
                 "Basic block target expected for conditional branch.");

          // Check if the relaxation is needed.
          if (TargetBB->getFragmentNum() == FF.getFragmentNum() &&
              isBlockInRange(Inst, InstAddress, *TargetBB))
            continue;

          relaxBranch(BB, Inst, InstAddress, TargetBB);

          MayNeedRelaxation = true;
        }
      }

      // We may have added new instructions, but the whole fragment is less than
      // the minimum branch span.
      if (FragmentSize < ShortestJumpSpan)
        MayNeedRelaxation = false;

    } while (MayNeedRelaxation);

    LLVM_DEBUG({
      if (NumIterations > 2) {
        dbgs() << "BOLT-DEBUG: relaxed fragment " << FF.getFragmentNum().get()
               << " of " << BF << " in " << NumIterations << " iterations\n";
      }
    });
    (void)NumIterations;
  }

  // Add trampoline blocks from all fragments to the layout.
  DenseMap<BinaryBasicBlock *, std::vector<std::unique_ptr<BinaryBasicBlock>>>
      Insertions;
  for (std::pair<BinaryBasicBlock *, std::unique_ptr<BinaryBasicBlock>> &Pair :
       FunctionTrampolines) {
    if (!Pair.second)
      continue;
    Insertions[Pair.first].emplace_back(std::move(Pair.second));
  }

  for (auto &Pair : Insertions) {
    BF.insertBasicBlocks(Pair.first, std::move(Pair.second),
                         /*UpdateLayout*/ true, /*UpdateCFI*/ true,
                         /*RecomputeLPs*/ false);
  }
}

Error LongJmpPass::runOnFunctions(BinaryContext &BC) {

  if (opts::CompactCodeModel) {
    BC.outs()
        << "BOLT-INFO: relaxing branches for compact code model (<128MB)\n";

    ParallelUtilities::WorkFuncTy WorkFun = [&](BinaryFunction &BF) {
      relaxLocalBranches(BF);
    };

    ParallelUtilities::PredicateTy SkipPredicate =
        [&](const BinaryFunction &BF) {
          return !BC.shouldEmit(BF) || !BF.isSimple();
        };

    ParallelUtilities::runOnEachFunction(
        BC, ParallelUtilities::SchedulingPolicy::SP_INST_LINEAR, WorkFun,
        SkipPredicate, "RelaxLocalBranches");

    return Error::success();
  }

  BC.outs() << "BOLT-INFO: Starting stub-insertion pass\n";
  std::vector<BinaryFunction *> Sorted = BC.getSortedFunctions();
  bool Modified;
  uint32_t Iterations = 0;
  do {
    ++Iterations;
    Modified = false;
    tentativeLayout(BC, Sorted);
    updateStubGroups();
    for (BinaryFunction *Func : Sorted) {
      if (auto E = relax(*Func, Modified))
        return Error(std::move(E));
      // Don't ruin non-simple functions, they can't afford to have the layout
      // changed.
      if (Modified && Func->isSimple())
        Func->fixBranches();
    }
  } while (Modified);
  BC.outs() << "BOLT-INFO: Inserted " << NumHotStubs
            << " stubs in the hot area and " << NumColdStubs
            << " stubs in the cold area. Shared " << NumSharedStubs
            << " times, iterated " << Iterations << " times.\n";
  return Error::success();
}
} // namespace bolt
} // namespace llvm
