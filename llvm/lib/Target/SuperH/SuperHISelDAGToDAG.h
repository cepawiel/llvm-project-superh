//===---- SuperHISelDAGToDAG.h - A Dag to Dag Inst Selector for SuperH ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines an instruction selector for the SuperH target.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SUPERH_SUPERHISELDAGTODAG_H
#define LLVM_LIB_TARGET_SUPERH_SUPERHISELDAGTODAG_H

#include "SuperHSubtarget.h"
#include "SuperHTargetMachine.h"
#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {
class SuperHDAGToDAGISel : public SelectionDAGISel {
public:
  explicit SuperHDAGToDAGISel(SuperHTargetMachine &TM, CodeGenOpt::Level OL)
      : SelectionDAGISel(TM, OL), Subtarget(nullptr) {}

  // Pass Name
  StringRef getPassName() const override {
    return "SuperH DAG->DAG Pattern Instruction Selection";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void Select(SDNode *Node) override;

  template <size_t N>
  bool SelectAddr(SDValue Addr, SDValue &Base, SDValue &Offset);

#include "SuperHGenDAGISel.inc"

private:
  const SuperHSubtarget *Subtarget;
};
}

#endif // end LLVM_LIB_TARGET_SUPERH_SUPERHISELDAGTODAG_H