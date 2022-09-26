//===-- SuperHAsmBackend.cpp - SuperH Assembler Backend -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/SuperHMCTargetDesc.h"
#include "llvm/ADT/APInt.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDirectives.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
class SuperHAsmBackend : public MCAsmBackend {
  uint8_t OSABI;

  uint64_t adjustFixupValue(const MCFixup &Fixup, uint64_t Value,
                            MCContext &Ctx) const;

public:
  SuperHAsmBackend(const MCSubtargetInfo &STI, uint8_t OSABI)
      : MCAsmBackend(support::little), OSABI(OSABI) {}
  ~SuperHAsmBackend() override {}

  void applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                  const MCValue &Target, MutableArrayRef<char> Data,
                  uint64_t Value, bool IsResolved,
                  const MCSubtargetInfo *STI) const override;

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override {
    return createSuperHELFObjectWriter(OSABI);
  }

  bool fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
                            const MCRelaxableFragment *DF,
                            const MCAsmLayout &Layout) const override {
    return false;
  }

  bool fixupNeedsRelaxationAdvanced(const MCFixup &Fixup, bool Resolved,
                                    uint64_t Value,
                                    const MCRelaxableFragment *DF,
                                    const MCAsmLayout &Layout,
                                    const bool WasForced) const override {
    return false;
  }

  unsigned getNumFixupKinds() const override {
    // return SuperH::NumTargetFixupKinds;
    llvm_unreachable("getNumFixupKinds()");
  }

  const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const override {
    // const static MCFixupKindInfo Infos[SuperH::NumTargetFixupKinds] = {
    //   // This table must be in the same order of enum in SuperHFixupKinds.h.
    //   //
    //   // name            offset bits flags
    //   {"fixup_32",            0, 32, 0},
    //   {"fixup_10_pcrel",      0, 10, MCFixupKindInfo::FKF_IsPCRel},
    //   {"fixup_16",            0, 16, 0},
    //   {"fixup_16_pcrel",      0, 16, MCFixupKindInfo::FKF_IsPCRel},
    //   {"fixup_16_byte",       0, 16, 0},
    //   {"fixup_16_pcrel_byte", 0, 16, MCFixupKindInfo::FKF_IsPCRel},
    //   {"fixup_2x_pcrel",      0, 10, MCFixupKindInfo::FKF_IsPCRel},
    //   {"fixup_rl_pcrel",      0, 16, MCFixupKindInfo::FKF_IsPCRel},
    //   {"fixup_8",             0,  8, 0},
    //   {"fixup_sym_diff",      0, 32, 0},
    // };
    // static_assert((array_lengthof(Infos)) == SuperH::NumTargetFixupKinds,
    //               "Not all fixup kinds added to Infos array");
  
    // if (Kind < FirstTargetFixupKind)
    //   return MCAsmBackend::getFixupKindInfo(Kind);
  
    // return Infos[Kind - FirstTargetFixupKind];
    const static MCFixupKindInfo Infos = {};
    return Infos;
  }

  bool writeNopData(raw_ostream &OS, uint64_t Count,
                    const MCSubtargetInfo *STI) const override;
};

uint64_t SuperHAsmBackend::adjustFixupValue(const MCFixup &Fixup,
                                            uint64_t Value,
                                            MCContext &Ctx) const {
  // unsigned Kind = Fixup.getKind();
  // switch (Kind) {
  // case SuperH::fixup_10_pcrel: {
  //   if (Value & 0x1)
  //     Ctx.reportError(Fixup.getLoc(), "fixup value must be 2-byte aligned");

  //   // Offset is signed
  //   int16_t Offset = Value;
  //   // Jumps are in words
  //   Offset >>= 1;
  //   // PC points to the next instruction so decrement by one
  //   --Offset;

  //   if (Offset < -512 || Offset > 511)
  //     Ctx.reportError(Fixup.getLoc(), "fixup value out of range");

  //   // Mask 10 bits
  //   Offset &= 0x3ff;

  //   return Offset;
  // }
  // default:
  //   return Value;
  // }
  llvm_unreachable("adjustFixupValue()");
}

void SuperHAsmBackend::applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                                  const MCValue &Target,
                                  MutableArrayRef<char> Data,
                                  uint64_t Value, bool IsResolved,
                                  const MCSubtargetInfo *STI) const {
  Value = adjustFixupValue(Fixup, Value, Asm.getContext());
  MCFixupKindInfo Info = getFixupKindInfo(Fixup.getKind());
  if (!Value)
    return; // Doesn't change encoding.

  // Shift the value into position.
  Value <<= Info.TargetOffset;

  unsigned Offset = Fixup.getOffset();
  unsigned NumBytes = alignTo(Info.TargetSize + Info.TargetOffset, 8) / 8;

  assert(Offset + NumBytes <= Data.size() && "Invalid fixup offset!");

  // For each byte of the fragment that the fixup touches, mask in the
  // bits from the fixup value.
  for (unsigned i = 0; i != NumBytes; ++i) {
    Data[Offset + i] |= uint8_t((Value >> (i * 8)) & 0xff);
  }
}

bool SuperHAsmBackend::writeNopData(raw_ostream &OS, uint64_t Count,
                                    const MCSubtargetInfo *STI) const {
  if ((Count % 2) != 0)
    return false;

  // The canonical nop on SuperH is mov #0, r3
  uint64_t NopCount = Count / 2;
  while (NopCount--)
    OS.write("\x03\x43", 2);

  return true;
}

} // end anonymous namespace

MCAsmBackend *llvm::createSuperHMCAsmBackend(const Target &T,
                                             const MCSubtargetInfo &STI,
                                             const MCRegisterInfo &MRI,
                                             const MCTargetOptions &Options) {
  return new SuperHAsmBackend(STI, ELF::ELFOSABI_STANDALONE);
}
