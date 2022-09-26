//===-- SuperHMCTargetDesc.cpp - SuperH Target Descriptions -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides SuperH specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "SuperHMCTargetDesc.h"
#include "SuperHInstPrinter.h"
#include "SuperHMCAsmInfo.h"
#include "TargetInfo/SuperHTargetInfo.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCInstrAnalysis.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#include "SuperHGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "SuperHGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "SuperHGenRegisterInfo.inc"

static MCInstrInfo *createSuperHMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitSuperHMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createSuperHMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  return X;
}

static MCSubtargetInfo *
createSuperHMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  return createSuperHMCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
}

static MCInstPrinter *createSuperHMCInstPrinter(const Triple &T,
                                               unsigned SyntaxVariant,
                                               const MCAsmInfo &MAI,
                                               const MCInstrInfo &MII,
                                               const MCRegisterInfo &MRI) {
  return new SuperHInstPrinter(MAI, MII, MRI);
}

static MCAsmInfo *createSuperHMCAsmInfo(const MCRegisterInfo &MRI,
                                       const Triple &TT,
                                       const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new SuperHMCAsmInfo(TT);

  unsigned WP = MRI.getDwarfRegNum(SuperH::R2, true);
  MCCFIInstruction Inst = MCCFIInstruction::cfiDefCfa(nullptr, WP, 0);
  MAI->addInitialFrameState(Inst);

  return MAI;
}

extern "C" void LLVMInitializeSuperHTargetMC() {
  for (Target *T : {&getTheSuperHTarget()}) {
    // Register the MC asm info.
    TargetRegistry::RegisterMCAsmInfo(*T, createSuperHMCAsmInfo);

    // Register the MC instruction info.
    TargetRegistry::RegisterMCInstrInfo(*T, createSuperHMCInstrInfo);

    // Register the MC register info.
    TargetRegistry::RegisterMCRegInfo(*T, createSuperHMCRegisterInfo);

    // Register the MC subtarget info.
    TargetRegistry::RegisterMCSubtargetInfo(*T, createSuperHMCSubtargetInfo);

    // Register the MCInstPrinter.
    TargetRegistry::RegisterMCInstPrinter(*T, createSuperHMCInstPrinter);
    
    TargetRegistry::RegisterMCCodeEmitter(*T, createSuperHMCCodeEmitter);
    TargetRegistry::RegisterObjectTargetStreamer(*T, createSuperHObjectTargetStreamer);
    TargetRegistry::RegisterMCAsmBackend(*T, createSuperHMCAsmBackend);
  }
}