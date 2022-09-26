//===-- SuperHTargetMachine.cpp - Define TargetMachine for SuperH -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implements the info about SuperH target spec.
//
//===----------------------------------------------------------------------===//

#include "SuperHTargetMachine.h"
#include "SuperHISelDAGToDAG.h"
#include "SuperHSubtarget.h"
#include "SuperHTargetObjectFile.h"
#include "TargetInfo/SuperHTargetInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSuperHTarget() {
  // Register the target.
  //- Little endian Target Machine
  RegisterTargetMachine<SuperHTargetMachine> X(getTheSuperHTarget());
}

static std::string computeDataLayout() {
  std::string Ret = "";

  // Little endian
  Ret += "e";

  // ELF name mangling
  Ret += "-m:e";

  // 32-bit pointers, 32-bit aligned
  Ret += "-p:32:32";

  // 64-bit integers, 64 bit aligned
  Ret += "-i64:64";

  // 32-bit native integer width i.e register are 32-bit
  Ret += "-n32";

  // 128-bit natural stack alignment
  Ret += "-S128";

  return Ret;
}

static Reloc::Model getEffectiveRelocModel(Optional<CodeModel::Model> CM,
                                           Optional<Reloc::Model> RM) {
  if (!RM.has_value())
    return Reloc::Static;
  return *RM;
}

SuperHTargetMachine::SuperHTargetMachine(const Target &T, const Triple &TT,
                                       StringRef CPU, StringRef FS,
                                       const TargetOptions &Options,
                                       Optional<Reloc::Model> RM,
                                       Optional<CodeModel::Model> CM,
                                       CodeGenOpt::Level OL,
                                       bool JIT)
    : LLVMTargetMachine(T, computeDataLayout(), TT, CPU, FS, Options,
                        getEffectiveRelocModel(CM, RM),
                        getEffectiveCodeModel(CM, CodeModel::Medium), OL),
      TLOF(std::make_unique<SuperHTargetObjectFile>()) {
  // initAsmInfo will display features by llc -march=SuperH on 3.7
  initAsmInfo();
}

const SuperHSubtarget *
SuperHTargetMachine::getSubtargetImpl(const Function &F) const {
  Attribute CPUAttr = F.getFnAttribute("target-cpu");
  Attribute FSAttr = F.getFnAttribute("target-features");

  std::string CPU = !CPUAttr.hasAttribute(Attribute::None)
                        ? CPUAttr.getValueAsString().str()
                        : TargetCPU;
  std::string FS = !FSAttr.hasAttribute(Attribute::None)
                       ? FSAttr.getValueAsString().str()
                       : TargetFS;

  auto &I = SubtargetMap[CPU + FS];
  if (!I) {
    // This needs to be done before we create a new subtarget since any
    // creation will depend on the TM and the code generation flags on the
    // function that reside in TargetOptions.
    resetTargetOptions(F);
    I = std::make_unique<SuperHSubtarget>(TargetTriple, CPU, FS, *this);
  }
  return I.get();
}

namespace {
class SuperHPassConfig : public TargetPassConfig {
public:
  SuperHPassConfig(SuperHTargetMachine &TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  SuperHTargetMachine &getSuperHTargetMachine() const {
    return getTM<SuperHTargetMachine>();
  }

  bool addInstSelector() override;
  void addPreEmitPass() override;
};
}

TargetPassConfig *SuperHTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new SuperHPassConfig(*this, PM);
}

// Install an instruction selector pass using
// the ISelDag to gen SuperH code.
bool SuperHPassConfig::addInstSelector() {
  addPass(new SuperHDAGToDAGISel(getSuperHTargetMachine(), getOptLevel()));
  return false;
}

// Implemented by targets that want to run passes immediately before
// machine code is emitted. return true if -print-machineinstrs should
// print out the code after the passes.
void SuperHPassConfig::addPreEmitPass() {
  addPass(createSuperHDelaySlotFillerPass(getSuperHTargetMachine()));
  addPass(createSuperHConstantIslandPass());
}