; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -mtriple=riscv32 -mattr=+zbb -verify-machineinstrs < %s \
; RUN:   | FileCheck %s -check-prefixes=RV32ZB
; RUN: llc -mtriple=riscv64 -mattr=+zbb -verify-machineinstrs < %s \
; RUN:   | FileCheck %s -check-prefixes=RV64ZB
; RUN: llc -mtriple=riscv32 -mattr=+zbkb -verify-machineinstrs < %s \
; RUN:   | FileCheck %s -check-prefixes=RV32ZB
; RUN: llc -mtriple=riscv64 -mattr=+zbkb -verify-machineinstrs < %s \
; RUN:   | FileCheck %s -check-prefixes=RV64ZB

declare i16 @llvm.bswap.i16(i16)
declare i32 @llvm.bswap.i32(i32)
declare i64 @llvm.bswap.i64(i64)

define i16 @test_bswap_srli_7_bswap_i16(i16 %a) nounwind {
; RV32ZB-LABEL: test_bswap_srli_7_bswap_i16:
; RV32ZB:       # %bb.0:
; RV32ZB-NEXT:    rev8 a0, a0
; RV32ZB-NEXT:    srli a0, a0, 23
; RV32ZB-NEXT:    rev8 a0, a0
; RV32ZB-NEXT:    srli a0, a0, 16
; RV32ZB-NEXT:    ret
;
; RV64ZB-LABEL: test_bswap_srli_7_bswap_i16:
; RV64ZB:       # %bb.0:
; RV64ZB-NEXT:    rev8 a0, a0
; RV64ZB-NEXT:    srli a0, a0, 55
; RV64ZB-NEXT:    rev8 a0, a0
; RV64ZB-NEXT:    srli a0, a0, 48
; RV64ZB-NEXT:    ret
    %1 = call i16 @llvm.bswap.i16(i16 %a)
    %2 = lshr i16 %1, 7
    %3 = call i16 @llvm.bswap.i16(i16 %2)
    ret i16 %3
}

define i16 @test_bswap_srli_8_bswap_i16(i16 %a) nounwind {
; RV32ZB-LABEL: test_bswap_srli_8_bswap_i16:
; RV32ZB:       # %bb.0:
; RV32ZB-NEXT:    slli a0, a0, 8
; RV32ZB-NEXT:    ret
;
; RV64ZB-LABEL: test_bswap_srli_8_bswap_i16:
; RV64ZB:       # %bb.0:
; RV64ZB-NEXT:    slli a0, a0, 8
; RV64ZB-NEXT:    ret
    %1 = call i16 @llvm.bswap.i16(i16 %a)
    %2 = lshr i16 %1, 8
    %3 = call i16 @llvm.bswap.i16(i16 %2)
    ret i16 %3
}

define i32 @test_bswap_srli_8_bswap_i32(i32 %a) nounwind {
; RV32ZB-LABEL: test_bswap_srli_8_bswap_i32:
; RV32ZB:       # %bb.0:
; RV32ZB-NEXT:    slli a0, a0, 8
; RV32ZB-NEXT:    ret
;
; RV64ZB-LABEL: test_bswap_srli_8_bswap_i32:
; RV64ZB:       # %bb.0:
; RV64ZB-NEXT:    slliw a0, a0, 8
; RV64ZB-NEXT:    ret
    %1 = call i32 @llvm.bswap.i32(i32 %a)
    %2 = lshr i32 %1, 8
    %3 = call i32 @llvm.bswap.i32(i32 %2)
    ret i32 %3
}

define i32 @test_bswap_srli_16_bswap_i32(i32 %a) nounwind {
; RV32ZB-LABEL: test_bswap_srli_16_bswap_i32:
; RV32ZB:       # %bb.0:
; RV32ZB-NEXT:    slli a0, a0, 16
; RV32ZB-NEXT:    ret
;
; RV64ZB-LABEL: test_bswap_srli_16_bswap_i32:
; RV64ZB:       # %bb.0:
; RV64ZB-NEXT:    slliw a0, a0, 16
; RV64ZB-NEXT:    ret
    %1 = call i32 @llvm.bswap.i32(i32 %a)
    %2 = lshr i32 %1, 16
    %3 = call i32 @llvm.bswap.i32(i32 %2)
    ret i32 %3
}

define i32 @test_bswap_srli_24_bswap_i32(i32 %a) nounwind {
; RV32ZB-LABEL: test_bswap_srli_24_bswap_i32:
; RV32ZB:       # %bb.0:
; RV32ZB-NEXT:    slli a0, a0, 24
; RV32ZB-NEXT:    ret
;
; RV64ZB-LABEL: test_bswap_srli_24_bswap_i32:
; RV64ZB:       # %bb.0:
; RV64ZB-NEXT:    slliw a0, a0, 24
; RV64ZB-NEXT:    ret
    %1 = call i32 @llvm.bswap.i32(i32 %a)
    %2 = lshr i32 %1, 24
    %3 = call i32 @llvm.bswap.i32(i32 %2)
    ret i32 %3
}

define i64 @test_bswap_srli_48_bswap_i64(i64 %a) nounwind {
; RV32ZB-LABEL: test_bswap_srli_48_bswap_i64:
; RV32ZB:       # %bb.0:
; RV32ZB-NEXT:    slli a1, a0, 16
; RV32ZB-NEXT:    li a0, 0
; RV32ZB-NEXT:    ret
;
; RV64ZB-LABEL: test_bswap_srli_48_bswap_i64:
; RV64ZB:       # %bb.0:
; RV64ZB-NEXT:    slli a0, a0, 48
; RV64ZB-NEXT:    ret
    %1 = call i64 @llvm.bswap.i64(i64 %a)
    %2 = lshr i64 %1, 48
    %3 = call i64 @llvm.bswap.i64(i64 %2)
    ret i64 %3
}

define i16 @test_bswap_shli_7_bswap_i16(i16 %a) nounwind {
; RV32ZB-LABEL: test_bswap_shli_7_bswap_i16:
; RV32ZB:       # %bb.0:
; RV32ZB-NEXT:    rev8 a0, a0
; RV32ZB-NEXT:    srli a0, a0, 16
; RV32ZB-NEXT:    slli a0, a0, 7
; RV32ZB-NEXT:    rev8 a0, a0
; RV32ZB-NEXT:    srli a0, a0, 16
; RV32ZB-NEXT:    ret
;
; RV64ZB-LABEL: test_bswap_shli_7_bswap_i16:
; RV64ZB:       # %bb.0:
; RV64ZB-NEXT:    rev8 a0, a0
; RV64ZB-NEXT:    srli a0, a0, 48
; RV64ZB-NEXT:    slli a0, a0, 7
; RV64ZB-NEXT:    rev8 a0, a0
; RV64ZB-NEXT:    srli a0, a0, 48
; RV64ZB-NEXT:    ret
    %1 = call i16 @llvm.bswap.i16(i16 %a)
    %2 = shl i16 %1, 7
    %3 = call i16 @llvm.bswap.i16(i16 %2)
    ret i16 %3
}

define i16 @test_bswap_shli_8_bswap_i16(i16 %a) nounwind {
; RV32ZB-LABEL: test_bswap_shli_8_bswap_i16:
; RV32ZB:       # %bb.0:
; RV32ZB-NEXT:    slli a0, a0, 16
; RV32ZB-NEXT:    srli a0, a0, 24
; RV32ZB-NEXT:    ret
;
; RV64ZB-LABEL: test_bswap_shli_8_bswap_i16:
; RV64ZB:       # %bb.0:
; RV64ZB-NEXT:    slli a0, a0, 48
; RV64ZB-NEXT:    srli a0, a0, 56
; RV64ZB-NEXT:    ret
    %1 = call i16 @llvm.bswap.i16(i16 %a)
    %2 = shl i16 %1, 8
    %3 = call i16 @llvm.bswap.i16(i16 %2)
    ret i16 %3
}

define i32 @test_bswap_shli_8_bswap_i32(i32 %a) nounwind {
; RV32ZB-LABEL: test_bswap_shli_8_bswap_i32:
; RV32ZB:       # %bb.0:
; RV32ZB-NEXT:    srli a0, a0, 8
; RV32ZB-NEXT:    ret
;
; RV64ZB-LABEL: test_bswap_shli_8_bswap_i32:
; RV64ZB:       # %bb.0:
; RV64ZB-NEXT:    srliw a0, a0, 8
; RV64ZB-NEXT:    ret
    %1 = call i32 @llvm.bswap.i32(i32 %a)
    %2 = shl i32 %1, 8
    %3 = call i32 @llvm.bswap.i32(i32 %2)
    ret i32 %3
}

define i32 @test_bswap_shli_16_bswap_i32(i32 %a) nounwind {
; RV32ZB-LABEL: test_bswap_shli_16_bswap_i32:
; RV32ZB:       # %bb.0:
; RV32ZB-NEXT:    srli a0, a0, 16
; RV32ZB-NEXT:    ret
;
; RV64ZB-LABEL: test_bswap_shli_16_bswap_i32:
; RV64ZB:       # %bb.0:
; RV64ZB-NEXT:    srliw a0, a0, 16
; RV64ZB-NEXT:    ret
    %1 = call i32 @llvm.bswap.i32(i32 %a)
    %2 = shl i32 %1, 16
    %3 = call i32 @llvm.bswap.i32(i32 %2)
    ret i32 %3
}

define i32 @test_bswap_shli_24_bswap_i32(i32 %a) nounwind {
; RV32ZB-LABEL: test_bswap_shli_24_bswap_i32:
; RV32ZB:       # %bb.0:
; RV32ZB-NEXT:    srli a0, a0, 24
; RV32ZB-NEXT:    ret
;
; RV64ZB-LABEL: test_bswap_shli_24_bswap_i32:
; RV64ZB:       # %bb.0:
; RV64ZB-NEXT:    srliw a0, a0, 24
; RV64ZB-NEXT:    ret
    %1 = call i32 @llvm.bswap.i32(i32 %a)
    %2 = shl i32 %1, 24
    %3 = call i32 @llvm.bswap.i32(i32 %2)
    ret i32 %3
}

define i64 @test_bswap_shli_48_bswap_i64(i64 %a) nounwind {
; RV32ZB-LABEL: test_bswap_shli_48_bswap_i64:
; RV32ZB:       # %bb.0:
; RV32ZB-NEXT:    srli a0, a1, 16
; RV32ZB-NEXT:    li a1, 0
; RV32ZB-NEXT:    ret
;
; RV64ZB-LABEL: test_bswap_shli_48_bswap_i64:
; RV64ZB:       # %bb.0:
; RV64ZB-NEXT:    srli a0, a0, 48
; RV64ZB-NEXT:    ret
    %1 = call i64 @llvm.bswap.i64(i64 %a)
    %2 = shl i64 %1, 48
    %3 = call i64 @llvm.bswap.i64(i64 %2)
    ret i64 %3
}
