//===-- SuperHMCTargetDesc.h - SuperH Target Descriptions ---------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_RISCW_MCTARGETDESC_SUPERHMCTARGETDESC_H
#define LLVM_LIB_TARGET_RISCW_MCTARGETDESC_SUPERHMCTARGETDESC_H

#include <memory>

namespace llvm {
class Target;
class MCAsmBackend;
class MCCodeEmitter;
class MCInstrInfo;
class MCSubtargetInfo;
class MCRegisterInfo;
class MCContext;
class MCTargetOptions;
class MCObjectTargetWriter;
class MCStreamer;
class MCTargetStreamer;

    MCCodeEmitter *createSuperHMCCodeEmitter(const MCInstrInfo &MCII,
                                             MCContext &Ctx);  

    MCAsmBackend *createSuperHMCAsmBackend(const Target &T,
                                       const MCSubtargetInfo &STI,
                                       const MCRegisterInfo &MRI,
                                       const MCTargetOptions &Options);


    MCTargetStreamer *
        createSuperHObjectTargetStreamer(MCStreamer &S, 
                                         const MCSubtargetInfo &STI);

    std::unique_ptr<MCObjectTargetWriter>
        createSuperHELFObjectWriter(uint8_t OSABI);
};

// Defines symbolic names for SuperH registers. This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "SuperHGenRegisterInfo.inc"

// Defines symbolic names for the SuperH instructions.
#define GET_INSTRINFO_ENUM
#include "SuperHGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "SuperHGenSubtargetInfo.inc"

#endif // end LLVM_LIB_TARGET_RISCW_MCTARGETDESC_SUPERHMCTARGETDESC_H