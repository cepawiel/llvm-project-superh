//=== SuperHMachineFunctionInfo.h - Private data used for SuperH ----*- C++ -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the SuperH specific subclass of MachineFunctionInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SUPERH_SUPERHMACHINEFUNCTION_H
#define LLVM_LIB_TARGET_SUPERH_SUPERHMACHINEFUNCTION_H

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

/// SuperHFunctionInfo - This class is derived from MachineFunction private
/// SuperH target-specific information for each MachineFunction.
class SuperHFunctionInfo : public MachineFunctionInfo {
private:
  MachineFunction &MF;

public:
  SuperHFunctionInfo(MachineFunction &MF) : MF(MF) {}
};

} // end of namespace llvm

#endif // end LLVM_LIB_TARGET_SUPERH_SUPERHMACHINEFUNCTION_H