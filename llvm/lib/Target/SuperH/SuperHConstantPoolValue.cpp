//===-- SuperHConstantPoolValue.cpp - SuperH constant-pool value ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "SuperHConstantPoolValue.h"
#include "llvm/ADT/FoldingSet.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

SuperHConstantPoolValue::SuperHConstantPoolValue(const Constant *value)
    : MachineConstantPoolValue(value->getType()), Value(value) {}

SuperHConstantPoolValue *SuperHConstantPoolValue::Create(const Constant *Value) {
  return new SuperHConstantPoolValue(Value);
}

int SuperHConstantPoolValue::getExistingMachineCPValue(MachineConstantPool *CP,
                                                   Align Alignment) {
  unsigned AlignMask = Alignment.value() - 1;
  const std::vector<MachineConstantPoolEntry> &Constants = CP->getConstants();
  for (unsigned I = 0, E = Constants.size(); I != E; ++I) {
    if (Constants[I].isMachineConstantPoolEntry() &&
        (Constants[I].getAlign().value() & AlignMask) == 0) {
      auto *CPV =
          static_cast<SuperHConstantPoolValue *>(Constants[I].Val.MachineCPVal);
      if (CPV->Value == Value)
        return I;
    }
  }
  return -1;
}

void SuperHConstantPoolValue::addSelectionDAGCSEId(FoldingSetNodeID &ID) {
  ID.AddPointer(Value);
}

void SuperHConstantPoolValue::print(raw_ostream &O) const { O << Value; }
