//===-- SuperHTargetInfo.cpp - SuperH Target Implementation -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/SuperHTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

Target &llvm::getTheSuperHTarget() {
  static Target TheSuperHTarget;
  return TheSuperHTarget;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSuperHTargetInfo() {
  RegisterTarget<Triple::superh> X(getTheSuperHTarget(), "superh",
                                  "32-bit SuperH", "SuperH");
}