//===-- SuperHInstrInfo.cpp - SuperH Instruction Information ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the SuperH implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "SuperHInstrInfo.h"

#include "SuperHTargetMachine.h"
#include "SuperHMachineFunction.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "superh-instrinfo"

#define GET_INSTRINFO_CTOR_DTOR
#include "SuperHGenInstrInfo.inc"

SuperHInstrInfo::SuperHInstrInfo(const SuperHSubtarget &STI)
    : SuperHGenInstrInfo(),
      Subtarget(STI)
{
}

void SuperHInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator MI,
                              const DebugLoc &DL, MCRegister DestReg,
                              MCRegister SrcReg, bool KillSrc) const {
  unsigned Opc = SuperH::MOV32rr;

  printf("Checking to see if all regs are GP\n");
  if (SuperH::GPRRegClass.contains(DestReg) && SuperH::GPRRegClass.contains(SrcReg)) {
    printf("All Regs are GP\n");
    BuildMI(MBB, MI, DL, get(Opc), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
    return;
      }

  if (!SuperH::GPRRegClass.contains(SrcReg)) {
    Opc = [&]() -> unsigned {
      switch (SrcReg) {
#define CASE(Type, X)                                                          \
  case SuperH::X:                                                                  \
    return SuperH::ST##Type##X
        CASE(S, MACL);
        CASE(S, MACH);
        CASE(S, PR);
        CASE(C, SR);
        CASE(C, GBR);
        CASE(C, VBR);
#undef CASE
      default:
        llvm_unreachable("Special register not handled.");
      }
    }();
    BuildMI(MBB, MI, DL, get(Opc), DestReg);
  } else if (!SuperH::GPRRegClass.contains(DestReg)) {
    Opc = [&]() -> unsigned {
      switch (SrcReg) {
#define CASE(Type, X)                                                          \
  case SuperH::X:                                                                  \
    return SuperH::LD##Type##X
        CASE(S, MACL);
        CASE(S, MACH);
        CASE(S, PR);
        CASE(C, SR);
        CASE(C, GBR);
        CASE(C, VBR);
#undef CASE
      default:
        llvm_unreachable("Special register not handled.");
      }
    }();
    BuildMI(MBB, MI, DL, get(Opc)).addReg(SrcReg, getKillRegState(KillSrc));
  }

}

void SuperHInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                      MachineBasicBlock::iterator MI,
                                      Register SrcReg, bool isKill,
                                      int FrameIndex,
                                      const TargetRegisterClass *RC,
                                      const TargetRegisterInfo *TRI) const {
  DebugLoc DL = MI != MBB.end() ? MI->getDebugLoc() : DebugLoc();

  BuildMI(MBB, MI, DL, get(SuperH::MOV32rm))
      .addReg(SrcReg, getKillRegState(isKill))
      .addFrameIndex(FrameIndex);
}

void SuperHInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                       MachineBasicBlock::iterator MI,
                                       Register DstReg, int FrameIndex,
                                       const TargetRegisterClass *RC,
                                       const TargetRegisterInfo *TRI) const {
  DebugLoc DL = MI != MBB.end() ? MI->getDebugLoc() : DebugLoc();

  BuildMI(MBB, MI, DL, get(SuperH::MOV32mr), DstReg).addFrameIndex(FrameIndex);
}

unsigned SuperHInstrInfo::insertBranch(MachineBasicBlock &MBB,
                                   MachineBasicBlock *TBB,
                                   MachineBasicBlock *FBB,
                                   ArrayRef<MachineOperand> Cond,
                                 const DebugLoc &DL, int *BytesAdded) const {
assert(!BytesAdded && "code size not handled");

  // Unconditional branch
  if (Cond.empty()) {
    assert(!FBB && "Unconditional branch with multiple successors!");
    BuildMI(&MBB, DL, get(SuperH::BRA)).addMBB(TBB);
    return 1;
  }

  llvm_unreachable("Unexpected conditional branch");
}

unsigned SuperHInstrInfo::removeBranch(MachineBasicBlock &MBB,
                                   int *BytesRemoved) const {
  assert(!BytesRemoved && "code size not handled");

  MachineBasicBlock::iterator I = MBB.end();
  unsigned Count = 0;

  while (I != MBB.begin()) {
    --I;
    if (I->getOpcode() != SuperH::BRA)
      break;
    // Remove the branch.
    I->eraseFromParent();
    I = MBB.end();
    ++Count;
  }

  return Count;
}
