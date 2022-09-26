
//===-- SuperHMCAsmInfo.h - SuperH Asm Info ------------------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the RISCWMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_RISCW_MCTARGETDESC_SUPERHMCASMINFO_H
#define LLVM_LIB_TARGET_RISCW_MCTARGETDESC_SUPERHMCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {
  class Triple;

class SuperHMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit SuperHMCAsmInfo(const Triple &TheTriple);
};

} // namespace llvm

#endif // end LLVM_LIB_TARGET_RISCW_MCTARGETDESC_SUPERHMCASMINFO_H