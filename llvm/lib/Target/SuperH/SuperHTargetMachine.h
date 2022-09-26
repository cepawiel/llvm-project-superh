//===-- SuperHTargetMachine.h - Define TargetMachine for SuperH ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the SuperH specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SUPERH_SUPERHTARGETMACHINE_H
#define LLVM_LIB_TARGET_SUPERH_SUPERHTARGETMACHINE_H

#include "SuperHSubtarget.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class SuperHTargetMachine : public LLVMTargetMachine {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  mutable StringMap<std::unique_ptr<SuperHSubtarget>> SubtargetMap;

public:
  SuperHTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                    StringRef FS, const TargetOptions &Options,
                    Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
                    CodeGenOpt::Level OL, bool JIT);

  const SuperHSubtarget *getSubtargetImpl(const Function &F) const override;
  const SuperHSubtarget *getSubtargetImpl() const = delete;

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
};

FunctionPass *createSuperHDelaySlotFillerPass(const SuperHTargetMachine & /*tm*/);

FunctionPass *createSuperHConstantIslandPass();

}

#endif // end LLVM_LIB_TARGET_SUPERH_SUPERHTARGETMACHINE_H