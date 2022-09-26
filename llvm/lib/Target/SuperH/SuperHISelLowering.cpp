//===-- SuperHISelLowering.cpp - SuperH DAG Lowering Implementation -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that SuperH uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//
#include "SuperHISelLowering.h"
#include "SuperHSubtarget.h"
#include "SuperHTargetMachine.h"
#include "SuperHConstantPoolValue.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/Support/Debug.h"
#include <cassert>

using namespace llvm;

#define DEBUG_TYPE "superh-isellower"

#include "SuperHGenCallingConv.inc"

SuperHTargetLowering::SuperHTargetLowering(const TargetMachine &TM,
                                         const SuperHSubtarget &STI)
    : TargetLowering(TM), Subtarget(STI)
{
  // Add GPR class as i32 registers.
  addRegisterClass(MVT::i32, &SuperH::GPRRegClass);
  computeRegisterProperties(STI.getRegisterInfo());
  setStackPointerRegisterToSaveRestore(SuperH::R15);

  setOperationAction(ISD::GlobalAddress, MVT::i32, Custom);
  setOperationAction(ISD::BR_CC, MVT::i32, Expand);
  setOperationAction(ISD::SHL, MVT::i32, Custom);
  setOperationAction(ISD::SRL, MVT::i32, Custom);
  setOperationAction(ISD::SMUL_LOHI, MVT::i32, Expand);
  setOperationAction(ISD::UMUL_LOHI, MVT::i32, Expand);

  // setOperationAction(ISD::SETEQ, MVT::i32, Custom);
  // setOperationAction(ISD::SETNE, MVT::i32, Expand);

  setOperationAction(ISD::SETCC, MVT::i32, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::i32, Expand);
 
  // setTargetDAGCombine(ISD::SETCC);

  // // Must, computeRegisterProperties - Once all of the register classes are
  // // added, this allows us to compute derived properties we expose.
  // computeRegisterProperties(Subtarget.getRegisterInfo());

  // setStackPointerRegisterToSaveRestore(SuperH::R2);

  // // Set scheduling preference. There are a few options:
  // //    - None: No preference
  // //    - Source: Follow source order
  // //    - RegPressure: Scheduling for lowest register pressure
  // //    - Hybrid: Scheduling for both latency and register pressure
  // // Source (the option used by XCore) is no good when there are few registers
  // // because the compiler will try to keep a lot more things into the register
  // // which eventually results in a lot of stack spills for no good reason. So
  // // use either RegPressure or Hybrid
  // setSchedulingPreference(Sched::RegPressure);

  // Use i32 for setcc operations results (slt, sgt, ...).
  setBooleanContents(ZeroOrOneBooleanContent);
  setBooleanVectorContents(ZeroOrOneBooleanContent);

  // setOperationAction(ISD::GlobalAddress, MVT::i32, Custom);
  // setOperationAction(ISD::BlockAddress,  MVT::i32, Custom);
  // setOperationAction(ISD::ConstantPool,  MVT::i32, Custom);

  // // Set minimum and preferred function alignment (log2)
  // setMinFunctionAlignment(Align(1));
  // setPrefFunctionAlignment(Align(1));

  // // Set preferred loop alignment (log2)
  // setPrefLoopAlignment(Align(1));
}

const char *SuperHTargetLowering::getTargetNodeName(unsigned Opcode) const {
    switch ((SuperHISD::NodeType)Opcode) {
  case SuperHISD::FIRST_NUMBER:
    break;
#define CASE(X)                                                                \
  case SuperHISD::X:                                                               \
    return #X

    CASE(Ret);
    CASE(Call);
    CASE(Wrapper);
    CASE(SHL);
    CASE(SRL);

#undef CASE
  }

  return nullptr;
}

void SuperHTargetLowering::ReplaceNodeResults(SDNode *N,
                                             SmallVectorImpl<SDValue> &Results,
                                             SelectionDAG &DAG) const {
  switch (N->getOpcode()) {
  default:
    llvm_unreachable("Don't know how to custom expand this!");
  }
}

//===----------------------------------------------------------------------===//
//@            Formal Arguments Calling Convention Implementation
//===----------------------------------------------------------------------===//

// The BeyondRISC calling convention parameter registers.
static const MCPhysReg GPRArgRegs[] = {
  SuperH::R0, SuperH::R1, SuperH::R2, SuperH::R3
};

/// LowerFormalArguments - transform physical registers into virtual registers
/// and generate load operations for arguments places on the stack.
SDValue SuperHTargetLowering::LowerFormalArguments(
                                    SDValue Chain,
                                    CallingConv::ID CallConv,
                                    bool IsVarArg,
                                    const SmallVectorImpl<ISD::InputArg> &Ins,
                                    const SDLoc &DL, SelectionDAG &DAG,
                                    SmallVectorImpl<SDValue> &InVals) const
{
    assert(!IsVarArg && "Variable arguments not supported.");

  auto &MF = DAG.getMachineFunction();

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_SuperH);

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    MVT RegVT = VA.getLocVT();
    const TargetRegisterClass *RC = getRegClassFor(RegVT);
    unsigned Reg = MF.addLiveIn(VA.getLocReg(), RC);
    SDValue ArgValue = DAG.getCopyFromReg(Chain, DL, Reg, RegVT);
    InVals.push_back(ArgValue);
  }

  return Chain;
}

//===----------------------------------------------------------------------===//
//@              Return Value Calling Convention Implementation
//===----------------------------------------------------------------------===//

bool SuperHTargetLowering::CanLowerReturn(CallingConv::ID CallConv,
                                MachineFunction &MF, bool isVarArg,
                                const SmallVectorImpl<ISD::OutputArg> &Outs,
                                LLVMContext &Context) const
{
  // SmallVector<CCValAssign, 16> RVLocs;
  // CCState CCInfo(CallConv, isVarArg, MF, RVLocs, Context);
  // return CCInfo.CheckReturn(Outs, SuperH_CRetConv);
  return true;
}

/// LowerMemOpCallTo - Store the argument to the stack.
SDValue SuperHTargetLowering::LowerMemOpCallTo(SDValue Chain,
                                              SDValue Arg, const SDLoc &dl,
                                              SelectionDAG &DAG,
                                              const CCValAssign &VA,
                                              ISD::ArgFlagsTy Flags) const {
  llvm_unreachable("Cannot store arguments to stack");
}

/// LowerCallResult - Lower the result values of a call into the
/// appropriate copies out of appropriate physical registers.
// SDValue
// SuperHTargetLowering::LowerCallResult(SDValue Chain, SDValue InFlag,
//                                      CallingConv::ID CallConv, bool isVarArg,
//                                      const SmallVectorImpl<ISD::InputArg> &Ins,
//                                      const SDLoc &dl, SelectionDAG &DAG,
//                                      SmallVectorImpl<SDValue> &InVals,
//                                      bool isThisReturn, SDValue ThisVal) const {
//   // Assign locations to each value returned by this call.
//   SmallVector<CCValAssign, 16> RVLocs;
//   CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
//                  *DAG.getContext());
//   CCInfo.AnalyzeCallResult(Ins, SuperH_CRetConv);

//   // Copy all of the result registers out of their specified physreg.
//   for (unsigned i = 0; i != RVLocs.size(); ++i) {
//     CCValAssign VA = RVLocs[i];

//     // Pass 'this' value directly from the argument to return value, to avoid
//     // reg unit interference
//     if (i == 0 && isThisReturn) {
//       assert(!VA.needsCustom() && VA.getLocVT() == MVT::i32 &&
//              "unexpected return calling convention register assignment");
//       InVals.push_back(ThisVal);
//       continue;
//     }

//     SDValue Val;
//     if (VA.needsCustom()) {
//         llvm_unreachable("Vector and floating point values not supported yet");
//     } else {
//       Val = DAG.getCopyFromReg(Chain, dl, VA.getLocReg(), VA.getLocVT(),
//                                InFlag);
//       Chain = Val.getValue(1);
//       InFlag = Val.getValue(2);
//     }

//     switch (VA.getLocInfo()) {
//     default: llvm_unreachable("Unknown loc info!");
//     case CCValAssign::Full: break;
//     case CCValAssign::BCvt:
//       Val = DAG.getNode(ISD::BITCAST, dl, VA.getValVT(), Val);
//       break;
//     }

//     InVals.push_back(Val);
//   }

//   return Chain;
// }

SDValue SuperHTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                       SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  auto &MF = DAG.getMachineFunction();
  SDLoc &DL = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  CallingConv::ID CallConv = CLI.CallConv;
  bool isVarArg = CLI.IsVarArg;

  CLI.IsTailCall = false;

  assert(!isVarArg && "Variable arguments not supported.");

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, MF, ArgLocs,
                 *DAG.getContext());
  CCInfo.AnalyzeCallOperands(Outs, CC_SuperH);

  SmallVector<std::pair<unsigned, SDValue>, 4> RegsToPass;
  // FIXME: Handle more than 4 arguments.

  // But also, we need to glue them in order to avoid inserting other
  // instructions in the middle of the lowering.
  SDValue Glue;
  for (size_t i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    SDValue Arg = OutVals[i];

    if (VA.isRegLoc()) {
      // Copy the argument to the argument register. Update chain and glue.
      Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), Arg, Glue);
      Glue = Chain.getValue(1);
      RegsToPass.emplace_back(VA.getLocReg(), Arg);
    }

    if (isa<GlobalAddressSDNode>(Callee))
      Callee = LowerGlobalAddress(Callee, DAG);
    else if (ExternalSymbolSDNode *E = dyn_cast<ExternalSymbolSDNode>(Callee))
      Callee = DAG.getTargetExternalSymbol(E->getSymbol(), MVT::i32);
  }

  SmallVector<SDValue, 8> Ops{Chain, Callee};
  std::transform(RegsToPass.begin(), RegsToPass.end(), std::back_inserter(Ops),
                 [&](std::pair<unsigned, SDValue> &elt) {
                   return DAG.getRegister(elt.first, elt.second.getValueType());
                 });

  // FIXME: Caller save registers.

  if (Glue.getNode()) // Push the glue, if it's present.
    Ops.push_back(Glue);

  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  Chain = DAG.getNode(SuperHISD::Call, DL, NodeTys, Ops);
  Glue = Chain.getValue(1);

  {
    // Return value.
    SmallVector<CCValAssign, 16> RVLocs;
    CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                   *DAG.getContext());
    CCInfo.AnalyzeCallResult(Ins, RetCC_SuperH);

    for (auto &Loc : RVLocs) {
      auto RetValue =
          DAG.getCopyFromReg(Chain, DL, Loc.getLocReg(), Loc.getValVT(), Glue);
      Chain = RetValue.getValue(1);
      Glue = RetValue.getValue(2);
      InVals.push_back(Chain.getValue(0));
    }
  }

  return Chain;
}

/// HandleByVal - Every parameter *after* a byval parameter is passed
/// on the stack.  Remember the next parameter register to allocate,
/// and then confiscate the rest of the parameter registers to insure
/// this.
void SuperHTargetLowering::HandleByVal(CCState *State, unsigned &Size,
                                      Align Alignment) const {
  // Byval (as with any stack) slots are always at least 4 byte aligned.
  Alignment = std::max(Alignment, Align(4));

  unsigned Reg = State->AllocateReg(GPRArgRegs);
  if (!Reg)
    return;

  unsigned AlignInRegs = Alignment.value() / 4;
  unsigned Waste = (SuperH::R4 - Reg) % AlignInRegs;
  for (unsigned i = 0; i < Waste; ++i)
    Reg = State->AllocateReg(GPRArgRegs);

  if (!Reg)
    return;

  unsigned Excess = 4 * (SuperH::R4 - Reg);

  // Special case when NSAA != SP and parameter size greater than size of
  // all remained GPR regs. In that case we can't split parameter, we must
  // send it to stack. We also must set NCRN to X4, so waste all
  // remained registers.
  const unsigned NSAAOffset = State->getNextStackOffset();
  if (NSAAOffset != 0 && Size > Excess) {
    while (State->AllocateReg(GPRArgRegs))
      ;
    return;
  }

  // First register for byval parameter is the first register that wasn't
  // allocated before this method call, so it would be "reg".
  // If parameter is small enough to be saved in range [reg, r4), then
  // the end (first after last) register would be reg + param-size-in-regs,
  // else parameter would be splitted between registers and stack,
  // end register would be r4 in this case.
  unsigned ByValRegBegin = Reg;
  unsigned ByValRegEnd = std::min<unsigned>(Reg + Size / 4, SuperH::R4);
  State->addInRegsParamInfo(ByValRegBegin, ByValRegEnd);
  // Note, first register is allocated in the beginning of function already,
  // allocate remained amount of registers we need.
  for (unsigned i = Reg + 1; i != ByValRegEnd; ++i)
    State->AllocateReg(GPRArgRegs);
  // A byval parameter that is split between registers and memory needs its
  // size truncated here.
  // In the case where the entire structure fits in registers, we set the
  // size in memory to zero.
  Size = std::max<int>(Size - Excess, 0);
}

SDValue
SuperHTargetLowering::LowerReturn(SDValue Chain,
                                 CallingConv::ID CallConv, bool isVarArg,
                                 const SmallVectorImpl<ISD::OutputArg> &Outs,
                                 const SmallVectorImpl<SDValue> &OutVals,
                                 const SDLoc &DL, SelectionDAG &DAG) const {
  assert(!isVarArg && "Variable arguments not supported.");

  auto &MF = DAG.getMachineFunction();

  SmallVector<CCValAssign, 4> RVLocs;
  CCState CCInfo(CallConv, isVarArg, MF, RVLocs, *DAG.getContext());
  CCInfo.AnalyzeReturn(Outs, RetCC_SuperH);

  // We need to chain instructions together.
  SmallVector<SDValue, 4> RetOps{Chain};
  // But also, we need to glue them in order to avoid inserting other
  // instructions in the middle of the lowering.
  SDValue Glue;

  for (size_t i = 0; i < RVLocs.size(); ++i) {
    // Copy the result values into the output registers.
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Return in memory is not supported!");
    SDValue ValToCopy = OutVals[i];

    // Copy return value to the return register. Update chain and glue.
    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), ValToCopy, Glue);
    Glue = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  // Update the initial chain.
  RetOps[0] = Chain;
  if (Glue.getNode())
    RetOps.push_back(Glue);

  return DAG.getNode(SuperHISD::Ret, DL, MVT::Other, RetOps);
}

//===----------------------------------------------------------------------===//
//  Lower helper functions
//===----------------------------------------------------------------------===//

SDValue SuperHTargetLowering::getGlobalAddressWrapper(SDValue GA,
                                                     const GlobalValue *GV,
                                                     SelectionDAG &DAG) const {
  llvm_unreachable("Unhandled global variable");
}

//===----------------------------------------------------------------------===//
//  Misc Lower Operation implementation
//===----------------------------------------------------------------------===//

SDValue SuperHTargetLowering::
LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const {
  auto &MF = DAG.getMachineFunction();
  SDLoc DL{Op};
  auto PtrVT = getPointerTy(DAG.getDataLayout());
  const GlobalValue *GV = cast<GlobalAddressSDNode>(Op)->getGlobal();
  auto *CPV = SuperHConstantPoolValue::Create(GV);
  auto CP = DAG.getTargetConstantPool(CPV, PtrVT, Align(2));
  CP = DAG.getNode(SuperHISD::Wrapper, DL, MVT::i32, CP);

  return DAG.getLoad(PtrVT, DL, DAG.getEntryNode(), CP,
                     MachinePointerInfo::getConstantPool(MF), 4);
}

SDValue SuperHTargetLowering::
LowerConstantPool(SDValue Op, SelectionDAG &DAG) const {
  EVT PtrVT = Op.getValueType();
  SDLoc DL(Op);
  auto *CP = cast<ConstantPoolSDNode>(Op);
  SDValue Res = [&] {
  if (CP->isMachineConstantPoolEntry())
    return DAG.getTargetConstantPool(CP->getMachineCPVal(), PtrVT,
                                     CP->getAlign());
  else
    return DAG.getTargetConstantPool(CP->getConstVal(), PtrVT,
                                     CP->getAlign());
  }();
  return DAG.getNode(SuperHISD::Wrapper, DL, MVT::i32, Res);
}

// SuperH supports logical shifts of 1, 2, 8 and 16 bits. In order to generate
// a logical shift of N, split the combination of instructions needed.
template <enum SuperHISD::NodeType Opcode>
SDValue SuperHTargetLowering::LowerShift(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL{Op};
  auto LHS = Op.getOperand(0);
  auto RHS = Op.getOperand(1);

  if (auto Con = dyn_cast<ConstantSDNode>(RHS)) {
    static constexpr ssize_t Ls[] = {16, 8, 2, 1};
    auto Shift = Con->getSExtValue();

    auto InitShift = [&]() -> uint8_t {
      // No shift needed, instruction exists.
      if (!Shift ||
          std::find(std::begin(Ls), std::end(Ls), Shift) != std::end(Ls)) {
        auto Ret = Shift;
        Shift = 0;
        return Ret;
      }

      // Even, start with 2.
      if (Shift % 2 == 0) {
        Shift -= 2;
        return 2;
      }

      // Start with 1.
      Shift -= 1;
      return 1;
    }();

    SDValue Res = DAG.getNode(Opcode, DL, MVT::i32, LHS,
                              DAG.getConstant(InitShift, DL, MVT::i32));

    size_t CurrentSize = 0;
    while (Shift != 0) {
      // This should never trigger, since size 1 should suffice to extend it to
      // the maximum.
      assert(CurrentSize < array_lengthof(Ls) && "Array out of bounds.");

      // If there is till space, use more of the same size.
      if (Shift - Ls[CurrentSize] >= 0) {
        Shift -= Ls[CurrentSize];
        Res = DAG.getNode(Opcode, DL, MVT::i32, Res,
                          DAG.getConstant(Ls[CurrentSize], DL, MVT::i32));
      } else
        ++CurrentSize;
    }

    return Res;
  }

  llvm_unreachable("Shift with no constants.");
}

SDValue SuperHTargetLowering::
LowerSETCC(SDValue Op, SelectionDAG &DAG) const {
  // EVT PtrVT = Op.getValueType();
  // SDLoc DL(Op);
  // auto *CP = cast<ConstantPoolSDNode>(Op);
  // SDValue Res = [&] {
  // if (CP->isMachineConstantPoolEntry())
  //   return DAG.getTargetConstantPool(CP->getMachineCPVal(), PtrVT,
  //                                    CP->getAlign());
  // else
  //   return DAG.getTargetConstantPool(CP->getConstVal(), PtrVT,
  //                                    CP->getAlign());
  // }();
  // return DAG.getNode(SuperHISD::Wrapper, DL, MVT::i32, Res);
  return SDValue();
}

SDValue
SuperHTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  case ISD::GlobalAddress:
    return LowerGlobalAddress(Op, DAG);
  case ISD::ConstantPool:
    return LowerConstantPool(Op, DAG);
  case ISD::SHL:
    return LowerShift<SuperHISD::SHL>(Op, DAG);
  case ISD::SRL:
    return LowerShift<SuperHISD::SRL>(Op, DAG);
  case ISD::SETCC:
    return LowerSETCC(Op, DAG);
  default:
    llvm_unreachable("unimplemented operation");
  }
}

MachineBasicBlock *
SuperHTargetLowering::EmitInstrWithCustomInserter(MachineInstr &MI,
                                              MachineBasicBlock *MBB) const {
  auto MF = MBB->getParent();
  auto &Ctx = MF->getFunction().getContext();
  auto &TTI =
      *static_cast<const SuperHSubtarget &>(MF->getSubtarget()).getInstrInfo();
  switch (MI.getOpcode()) {
  case SuperH::MOV32ir: {
    switch (MI.getOperand(1).getType()) {
    case MachineOperand::MO_Immediate: {
      auto MBBI = MI.getIterator();
      auto Pool = MF->getConstantPool();
      auto Imm = MI.getOperand(1).getImm();
      auto Constant = ConstantInt::getSigned(IntegerType::get(Ctx, 32), Imm);
      auto CPI = Pool->getConstantPoolIndex(Constant, Align(4));
      BuildMI(*MBB, MBBI, MBBI->getDebugLoc(), TTI.get(SuperH::MOV32PCR),
              MI.getOperand(0).getReg())
          .addConstantPoolIndex(CPI);
      (MBBI++)->eraseFromParent();
      return MBB;
    }
    default:
      llvm_unreachable("Unexpected operand type for MOV32ir");
    }
    break;
  }
  default:
    llvm_unreachable("Unexpected instr type to insert");
  }
}