//===- SuperHConstantPoolValue.h - SuperH constant-pool value -*- C++ -----------*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SuperH_SuperHCONSTANTPOOLVALUE_H
#define LLVM_LIB_TARGET_SuperH_SuperHCONSTANTPOOLVALUE_H

#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"

namespace llvm {

class GlobalValue;

/// A SuperH-specific constant pool value.
class SuperHConstantPoolValue : public MachineConstantPoolValue {
  const Constant *Value;

protected:
  SuperHConstantPoolValue(const Constant *Value);

public:
  static SuperHConstantPoolValue *Create(const Constant *Value);

  int getExistingMachineCPValue(MachineConstantPool *CP,
                                Align Alignment) override;
  void addSelectionDAGCSEId(FoldingSetNodeID &ID) override;
  void print(raw_ostream &O) const override;

  // Access SuperH-specific fields.
  const Constant *getValue() const { return Value; }
  const GlobalValue *getGlobalValue() const { return cast<GlobalValue>(Value); }
};

} // end namespace llvm

#endif
