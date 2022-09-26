//=== SuperH.td - Describe the SuperH Target Machine ---------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Top tablegen file that pulls in code from the LLVM framework along with
// code specific to this target.
//
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// Calling Conv, Instruction Descriptions, Register information, etc
//===----------------------------------------------------------------------===//

include "SuperHSchedule.td"
include "SuperHRegisterInfo.td"
include "SuperHCallingConv.td"
include "SuperHInstrInfo.td"

def SuperHInstrInfo : InstrInfo {
  let decodePositionallyEncodedOperands = 1;
}

//===----------------------------------------------------------------------===//
// SuperH processors supported
//===----------------------------------------------------------------------===//

def : ProcessorModel<"SuperH",  NoSchedModel, []>;

//===----------------------------------------------------------------------===//
// SuperH Target declaration
//===----------------------------------------------------------------------===//

def SuperH : Target {
  let InstructionSet = SuperHInstrInfo;
}