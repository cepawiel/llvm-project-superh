//===-- SuperHRegisterInfo.cpp - SuperH Register Information ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the SuperH implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "SuperHRegisterInfo.h"
#include "SuperHSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/Support/Debug.h"

#define GET_REGINFO_TARGET_DESC
#include "SuperHGenRegisterInfo.inc"

#define DEBUG_TYPE "superh-reginfo"

using namespace llvm;

SuperHRegisterInfo::SuperHRegisterInfo(const SuperHSubtarget &ST)
  : SuperHGenRegisterInfo(SuperH::R1, /*DwarfFlavour*/0, /*EHFlavor*/0,
                         /*PC*/0), Subtarget(ST) {}

const MCPhysReg *
SuperHRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return SuperH_CSR_SaveList;
}

const TargetRegisterClass *SuperHRegisterInfo::intRegClass(unsigned Size) const {
  return &SuperH::GPRRegClass;
}

const uint32_t *
SuperHRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                        CallingConv::ID) const {
  return SuperH_CSR_RegMask;
}

BitVector SuperHRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());

  markSuperRegs(Reserved, SuperH::R15);
  markSuperRegs(Reserved, SuperH::SR);
  markSuperRegs(Reserved, SuperH::GBR);
  markSuperRegs(Reserved, SuperH::VBR);
  markSuperRegs(Reserved, SuperH::MACH);
  markSuperRegs(Reserved, SuperH::MACL);
  markSuperRegs(Reserved, SuperH::PR);
  markSuperRegs(Reserved, SuperH::PC);

  return Reserved;
}

bool SuperHRegisterInfo::isCalleeSavedIndex(int FrameIdx,
                                        MachineFrameInfo &MFI) const {
  auto &CSI = MFI.getCalleeSavedInfo();

  if (CSI.empty())
    return false;

  return FrameIdx >= CSI.front().getFrameIdx() &&
         FrameIdx <= CSI.back().getFrameIdx();
}

void SuperHRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                           int SPAdj,
                                           unsigned FIOperandNum,
                                           RegScavenger *RS) const {
  MachineInstr &MI = *II;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  DebugLoc DL = MI.getDebugLoc();
  int FrameIdx = MI.getOperand(FIOperandNum).getIndex();
  auto isCalleeSaved = isCalleeSavedIndex(FrameIdx, MF.getFrameInfo());

  // Use SP for callee-saved registers.
  unsigned BaseReg = isCalleeSaved ? SuperH::R15 : getFrameRegister(MF);

  MI.getOperand(FIOperandNum).ChangeToRegister(BaseReg, false);
  // FIXME: Callee saved registers.
}

bool
SuperHRegisterInfo::requiresRegisterScavenging(const MachineFunction &MF) const {
  return true;
}

bool
SuperHRegisterInfo::requiresFrameIndexScavenging(
                                            const MachineFunction &MF) const {
  return true;
}

bool
SuperHRegisterInfo::requiresFrameIndexReplacementScavenging(
                                            const MachineFunction &MF) const {
  return true;
}

bool
SuperHRegisterInfo::trackLivenessAfterRegAlloc(const MachineFunction &MF) const {
  return true;
}

Register SuperHRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return SuperH::R14;
}