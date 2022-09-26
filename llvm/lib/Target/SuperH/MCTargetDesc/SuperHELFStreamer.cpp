//===-- SuperHELFStreamer.cpp - SuperH ELF Target Streamer Methods --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides SuperH specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "SuperHMCTargetDesc.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"

using namespace llvm;
// using namespace llvm::SuperHAttrs;

namespace llvm {

class SuperHTargetELFStreamer : public MCTargetStreamer {
public:
  MCELFStreamer &getStreamer();
  SuperHTargetELFStreamer(MCStreamer &S, const MCSubtargetInfo &STI);
};

// This part is for ELF object output.
SuperHTargetELFStreamer::SuperHTargetELFStreamer(MCStreamer &S,
                                                 const MCSubtargetInfo &STI)
    : MCTargetStreamer(S) {
  // MCAssembler &MCA = getStreamer().getAssembler();
  // unsigned EFlags = MCA.getELFHeaderEFlags();
  // MCA.setELFHeaderEFlags(EFlags);

  // Emit build attributes section according to
  // SuperH EABI (slaa534.pdf, part 13).
  MCSection *AttributeSection = getStreamer().getContext().getELFSection(
      ".SuperH.attributes", ELF::EM_SH, 0);
  Streamer.switchSection(AttributeSection);

  // Format version.
  Streamer.emitInt8(0x41);
  // Subsection length.
  Streamer.emitInt32(22);
  // Vendor name string, zero-terminated.
  Streamer.emitBytes("mspabi");
  Streamer.emitInt8(0);

  // Attribute vector scope tag. 1 stands for the entire file.
  Streamer.emitInt8(1);
  // Attribute vector length.
  Streamer.emitInt32(11);

  // Streamer.emitInt8(TagISA);
  // Streamer.emitInt8(STI.hasFeature(SuperH::FeatureX) ? ISASuperHX : ISASuperH);
  // Streamer.emitInt8(TagCodeModel);
  // Streamer.emitInt8(CMSmall);
  // Streamer.emitInt8(TagDataModel);
  // Streamer.emitInt8(DMSmall);
  // Don't emit TagEnumSize, for full GCC compatibility.
}

MCELFStreamer &SuperHTargetELFStreamer::getStreamer() {
  return static_cast<MCELFStreamer &>(Streamer);
}

MCTargetStreamer *
createSuperHObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
  const Triple &TT = STI.getTargetTriple();
  if (TT.isOSBinFormatELF())
    return new SuperHTargetELFStreamer(S, STI);
  return nullptr;
}

} // namespace llvm
