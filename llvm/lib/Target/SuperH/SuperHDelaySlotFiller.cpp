//===-- SuperHDelaySlotFiller.cpp - SuperH delay slot filler ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Simple pass to fills delay slots with useful instructions.
//
//===----------------------------------------------------------------------===//

#include "SuperHTargetMachine.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

using namespace llvm;

#define DEBUG_TYPE "delay-slot-filler"

STATISTIC(FilledSlots, "Number of delay slots filled");

// #define GET_INSTRINFO_ENUM
// #include "SuperHGenInstrInfo.inc"

namespace {
struct Filler : public MachineFunctionPass {
  const TargetInstrInfo *TII;

  static char ID;
  explicit Filler() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override { return "SuperH Delay Slot Filler"; }

  bool runOnMachineBasicBlock(MachineBasicBlock &MBB);

  bool runOnMachineFunction(MachineFunction &MF) override {
    TII = MF.getSubtarget<SuperHSubtarget>().getInstrInfo();

    bool Changed = false;
    for (auto FI = MF.begin(), FE = MF.end(); FI != FE; ++FI)
      Changed |= runOnMachineBasicBlock(*FI);
    return Changed;
  }

  MachineFunctionProperties getRequiredProperties() const override {
    return MachineFunctionProperties().set(
        MachineFunctionProperties::Property::NoVRegs);
  }
};
char Filler::ID = 0;
} // end of anonymous namespace

// createSuperHDelaySlotFillerPass - Returns a pass that fills in delay
// slots in SuperH MachineFunctions
FunctionPass *
llvm::createSuperHDelaySlotFillerPass(const SuperHTargetMachine & /*tm*/) {
  return new Filler();
}

bool Filler::runOnMachineBasicBlock(MachineBasicBlock &MBB) {
  bool Changed = false;

  for (auto I = MBB.instr_begin(); I != MBB.instr_end(); ++I) {
    if (I->hasDelaySlot() && !I->isBundledWithSucc()) {
      BuildMI(MBB, std::next(I), I->getDebugLoc(), TII->get(SuperH::NOP));
      ++FilledSlots;

      Changed = true;

      // Bundle the delay slot filler to the instruction so that the machine
      // verifier doesn't expect this instruction to be a terminator.
      MIBundleBuilder(MBB, I, std::next(I, 2));
    }
  }
  return Changed;
}
