//===-- SuperHInstPrinter.cpp - Convert SuperH MCInst to assembly syntax ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class prints an SuperH MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#include "SuperHInstPrinter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
using namespace llvm;

#define PRINT_ALIAS_INSTR
#include "SuperHGenAsmWriter.inc"

// #define GET_INSTRINFO_ENUM
// #include "SuperHGenInstrInfo.inc"

void SuperHInstPrinter::printInst(const MCInst *MI, uint64_t Address, StringRef Annot,
                         const MCSubtargetInfo &STI, raw_ostream &O) {
  printInstruction(MI, Address, O);
}

void SuperHInstPrinter::printRegName(raw_ostream &OS, unsigned RegNo) const {
  OS << '%' << StringRef(getRegisterName(RegNo)).lower();
}

void SuperHInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                 raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isReg()) {
    printRegName(O, Op.getReg());
    // O << StringRef(getRegisterName(Op.getReg())).lower();
  } else if (Op.isImm()) {
    O << "#" << formatImm((int8_t)Op.getImm());
  } else {
    assert(Op.isExpr() && "unknown operand kind");
    O << *Op.getExpr();
  }
}

void SuperHInstPrinter::printMemOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O,
                      const char *Modifier) {
  const MCOperand &RegOp = MI->getOperand(OpNo);
  const MCOperand &OffsetOp = MI->getOperand(OpNo + 1);

  assert(OffsetOp.isImm() && "Expected an immediate");
  assert(RegOp.isReg() && "Register operand not a register");

  O << '(' << formatImm(OffsetOp.getImm()) << ", ";
  // << getRegisterName(RegOp.getReg()) << ')';
  printRegName(O, OffsetOp.getReg());
  O << ')';
}
