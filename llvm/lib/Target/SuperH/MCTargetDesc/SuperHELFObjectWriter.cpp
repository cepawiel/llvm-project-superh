//===-- SuperHELFObjectWriter.cpp - SuperH ELF Writer ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// #include "MCTargetDesc/SuperHFixupKinds.h"
#include "MCTargetDesc/SuperHMCTargetDesc.h"

#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace {
class SuperHELFObjectWriter : public MCELFObjectTargetWriter {
public:
  SuperHELFObjectWriter(uint8_t OSABI)
    : MCELFObjectTargetWriter(false, OSABI, ELF::EM_SH,
                              /*HasRelocationAddend*/ true) {}

  ~SuperHELFObjectWriter() override {}

protected:
  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsPCRel) const override {
    // Translate fixup kind to ELF relocation type.
    switch (Fixup.getTargetKind()) {
    // case FK_Data_1:                   return ELF::R_SuperH_8;
    // case FK_Data_2:                   return ELF::R_SuperH_16_BYTE;
    // case FK_Data_4:                   return ELF::R_SuperH_32;
    // case SuperH::fixup_32:            return ELF::R_SuperH_32;
    // case SuperH::fixup_10_pcrel:      return ELF::R_SuperH_10_PCREL;
    // case SuperH::fixup_16:            return ELF::R_SuperH_16;
    // case SuperH::fixup_16_pcrel:      return ELF::R_SuperH_16_PCREL;
    // case SuperH::fixup_16_byte:       return ELF::R_SuperH_16_BYTE;
    // case SuperH::fixup_16_pcrel_byte: return ELF::R_SuperH_16_PCREL_BYTE;
    // case SuperH::fixup_2x_pcrel:      return ELF::R_SuperH_2X_PCREL;
    // case SuperH::fixup_rl_pcrel:      return ELF::R_SuperH_RL_PCREL;
    // case SuperH::fixup_8:             return ELF::R_SuperH_8;
    // case SuperH::fixup_sym_diff:      return ELF::R_SuperH_SYM_DIFF;
    default:
      llvm_unreachable("Invalid fixup kind");
    }
  }
};
} // end of anonymous namespace

std::unique_ptr<MCObjectTargetWriter>
llvm::createSuperHELFObjectWriter(uint8_t OSABI) {
  return std::make_unique<SuperHELFObjectWriter>(OSABI);
}
