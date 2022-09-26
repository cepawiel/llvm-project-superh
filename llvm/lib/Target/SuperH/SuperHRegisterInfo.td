//===-- SuperHRegisterInfo.td ----------------------------------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// Describe registers
//===----------------------------------------------------------------------===//

// SuperH registers.
class SuperHReg<bits<16> enc, string n, list<string> altNames = [],
            list<Register> subregs = []> : Register<n, altNames> {
  let HWEncoding = enc;
  let Namespace = "SuperH";
  let SubRegs = subregs;
  // All bits of SuperH registers with sub-registers are covered by sub-registers.
  let CoveredBySubRegs = 1;
}

let Namespace = "SuperH" in {
  def sub_16bit : SubRegIndex<16>;
  def sub_16bit_hi : SubRegIndex<16, 16>;
}

//===----------------------------------------------------------------------===//
// Registers
//===----------------------------------------------------------------------===//

let Namespace = "SuperH" in {
  // GPR
  def R0 : SuperHReg<0, "r0">, DwarfRegNum<[ 0 ]>;
  def R1 : SuperHReg<1, "r1">, DwarfRegNum<[ 1 ]>;
  def R2 : SuperHReg<2, "r2">, DwarfRegNum<[ 2 ]>;
  def R3 : SuperHReg<3, "r3">, DwarfRegNum<[ 3 ]>;
  def R4 : SuperHReg<4, "r4">, DwarfRegNum<[ 4 ]>;
  def R5 : SuperHReg<5, "r5">, DwarfRegNum<[ 5 ]>;
  def R6 : SuperHReg<6, "r6">, DwarfRegNum<[ 6 ]>;
  def R7 : SuperHReg<7, "r7">, DwarfRegNum<[ 7 ]>;
  def R8 : SuperHReg<8, "r8">, DwarfRegNum<[ 8 ]>;
  def R9 : SuperHReg<9, "r9">, DwarfRegNum<[ 9 ]>;
  def R10 : SuperHReg<10, "r10">, DwarfRegNum<[ 10 ]>;
  def R11 : SuperHReg<11, "r11">, DwarfRegNum<[ 11 ]>;
  def R12 : SuperHReg<12, "r12">, DwarfRegNum<[ 12 ]>;
  def R13 : SuperHReg<13, "r13">, DwarfRegNum<[ 13 ]>;
  def R14 : SuperHReg<14, "r14", [ "fp" ]>, DwarfRegNum<[ 14 ]>;
  def R15 : SuperHReg<15, "r15", [ "sp" ]>, DwarfRegNum<[ 15 ]>;

  // Control registers
  def SR : SuperHReg<16, "sr">, DwarfRegNum<[ 22 ]>;
  def GBR : SuperHReg<17, "gbr">, DwarfRegNum<[ 18 ]>;
  def VBR : SuperHReg<18, "vbr">, DwarfRegNum<[ 19 ]>;

  // System registers
  def MACH : SuperHReg<19, "mach">, DwarfRegNum<[ 21 ]>;
  def MACL : SuperHReg<20, "macl">, DwarfRegNum<[ 21 ]>;
let SubRegIndices = [ sub_16bit, sub_16bit_hi ] in {
  def MAC : SuperHReg<21, "mac", [], [ MACL, MACH ]>;
}
  def PR : SuperHReg<22, "pr">, DwarfRegNum<[ 17 ]>;
  def PC : SuperHReg<23, "pc">, DwarfRegNum<[ 16 ]>;
}

//===----------------------------------------------------------------------===//
// Register classes
//===----------------------------------------------------------------------===//

// General purpose registers.
def GPR : RegisterClass<"SuperH", [ i32 ], 32, (sequence "R%u", 0, 15)>;

// Control registers.
def CR : RegisterClass<"SuperH", [ i32 ], 32, (add SR, GBR, VBR)> {
  let CopyCost = -1;
  let isAllocatable = 0;
}

// System registers.
def SYSR : RegisterClass<"SuperH", [ i32 ], 32, (add MACH, MACL, PR, PC)> {
  let CopyCost = -1;
  let isAllocatable = 0;
}

// MAC register.
def MACR : RegisterClass<"SuperH", [ i64 ], 64, (add MAC)> {
  let CopyCost = -1;
  let isAllocatable = 0;
}

def PCR : RegisterClass<"SuperH", [ i32 ], 32, (add PC)> {
  let CopyCost = -1;
  let isAllocatable = 0;
}
