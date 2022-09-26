add_llvm_component_group(SuperH)

set(LLVM_TARGET_DEFINITIONS SuperH.td)

tablegen(LLVM SuperHGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM SuperHGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM SuperHGenCodeEmitter.inc -gen-emitter)
tablegen(LLVM SuperHGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM SuperHGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM SuperHGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM SuperHGenCallingConv.inc -gen-callingconv)
tablegen(LLVM SuperHGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM SuperHGenMCPseudoLowering.inc -gen-pseudo-lowering)

add_public_tablegen_target(SuperHCommonTableGen)

add_llvm_target(SuperHCodeGen
    SuperHAsmPrinter.cpp
    SuperHFrameLowering.cpp
    SuperHInstrInfo.cpp
    SuperHISelLowering.cpp
    SuperHRegisterInfo.cpp
    SuperHSubtarget.cpp
    SuperHTargetObjectFile.cpp
    SuperHISelDAGToDAG.cpp
    SuperHTargetMachine.cpp
    SuperHConstantPoolValue.cpp
    SuperHConstantIslandPass.cpp
    SuperHDelaySlotFiller.cpp
)

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
