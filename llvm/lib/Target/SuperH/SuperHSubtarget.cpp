//===-- SuperHSubtarget.cpp - SuperH Subtarget Information --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the SuperH specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "SuperH.h"
#include "SuperHSubtarget.h"
#include "SuperHMachineFunction.h"
#include "SuperHRegisterInfo.h"
#include "SuperHTargetMachine.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "superh-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "SuperHGenSubtargetInfo.inc"

SuperHSubtarget::SuperHSubtarget(const Triple &TT, StringRef CPU, StringRef FS,
                               const TargetMachine &TM)
    : SuperHGenSubtargetInfo(TT, CPU, /*TuneCPU*/ CPU, FS),
      TSInfo(),
      InstrInfo(initializeSubtargetDependencies(TT, CPU, FS, TM)),
      FrameLowering(*this),
      TLInfo(TM, *this),
      RegInfo(*this) 
      { }


SuperHSubtarget &
SuperHSubtarget::initializeSubtargetDependencies(const Triple &TT, StringRef CPU,
                                                StringRef FS,
                                                const TargetMachine &TM) {

  // Parse features string.
  ParseSubtargetFeatures(CPU, /*TuneCPU*/ CPU, FS);

  return *this;
}