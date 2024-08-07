// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -mcpu=power8 -std=c++11 %s 2>&1 | FileCheck %s \
// RUN: -check-prefix=CHECK-DEFAULT

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -mcpu=power8 -std=c++11 -mno-vsx -mpower8-vector %s 2>&1 | \
// RUN: FileCheck %s -check-prefix=CHECK-NVSX-P8V

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -mcpu=power8 -std=c++11 -mno-vsx -mdirect-move %s 2>&1 | FileCheck %s \
// RUN: -check-prefix=CHECK-NVSX-DMV

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -mcpu=power8 -std=c++11 -mno-vsx -mpower8-vector -mvsx %s 2>&1 | \
// RUN: FileCheck %s -check-prefix=CHECK-DEFAULT

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -mcpu=power8 -std=c++11 -mno-vsx -mdirect-move -mvsx %s 2>&1 | \
// RUN: FileCheck %s -check-prefix=CHECK-DEFAULT

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -mcpu=power8 -std=c++11 -mpower8-vector -mno-vsx %s 2>&1 | \
// RUN: FileCheck %s -check-prefix=CHECK-NVSX-P8V

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -mcpu=power8 -std=c++11 -mdirect-move -mno-vsx %s 2>&1 | FileCheck %s \
// RUN: -check-prefix=CHECK-NVSX-DMV

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -mcpu=power8 -std=c++11 -mno-vsx %s 2>&1 | FileCheck %s \
// RUN: -check-prefix=CHECK-NVSX

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -mcpu=power6 -std=c++11 %s 2>&1 | FileCheck %s -check-prefix=CHECK-NVSX

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -mcpu=power6 -std=c++11 -mpower8-vector %s 2>&1 | FileCheck %s \
// RUN: -check-prefix=CHECK-DEFAULT

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -mcpu=power6 -std=c++11 -mdirect-move %s 2>&1 | FileCheck %s \
// RUN: -check-prefix=CHECK-VSX

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -mcpu=power9 -std=c++11 %s 2>&1 | FileCheck %s \
// RUN: -check-prefix=CHECK-DEFAULT-P9

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -mcpu=power9 -std=c++11 -mno-vsx -mpower9-vector %s 2>&1 | \
// RUN: FileCheck %s -check-prefix=CHECK-NVSX-P9V

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -mcpu=power9 -std=c++11 -mno-vsx -mfloat128 %s 2>&1 | \
// RUN: FileCheck %s -check-prefix=CHECK-NVSX-FLT128

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -mcpu=power10 -std=c++11 -mno-vsx -mpaired-vector-memops %s 2>&1 | \
// RUN: FileCheck %s -check-prefix=CHECK-NVSX-PAIRED-VEC-MEMOPS

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -mcpu=power10 -std=c++11 -mno-vsx -mmma %s 2>&1 | \
// RUN: FileCheck %s -check-prefix=CHECK-NVSX-MMA

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -mcpu=future -std=c++11 -mno-vsx -mmma %s 2>&1 | \
// RUN: FileCheck %s -check-prefix=CHECK-NVSX-MMA

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -mcpu=power9 -std=c++11 -mno-vsx -mfloat128 -mpower9-vector %s 2>&1 | \
// RUN: FileCheck %s -check-prefix=CHECK-NVSX-MULTI

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -mcpu=power10 -std=c++11 %s 2>&1 | FileCheck %s \
// RUN: -check-prefix=CHECK-DEFAULT-P10

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -mcpu=power10 -std=c++11 -mno-vsx -mpower10-vector %s 2>&1 | \
// RUN: FileCheck %s -check-prefix=CHECK-NVSX-P10V

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -std=c++11 -mvsx -mno-altivec %s 2>&1 | \
// RUN: FileCheck %s -check-prefix=CHECK-NALTI-VSX

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -std=c++11 -msoft-float -maltivec %s 2>&1 | \
// RUN: FileCheck %s -check-prefix=CHECK-SOFTFLT-ALTI

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -std=c++11 -msoft-float -mvsx %s 2>&1 | \
// RUN: FileCheck %s -check-prefix=CHECK-SOFTFLT-VSX

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -std=c++11 -msoft-float -mpower8-vector %s 2>&1 | \
// RUN: FileCheck %s -check-prefix=CHECK-SOFTFLT-P8VEC

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -std=c++11 -msoft-float -mpower9-vector %s 2>&1 | \
// RUN: FileCheck %s -check-prefix=CHECK-SOFTFLT-P9VEC

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -std=c++11 -msoft-float -mpower10-vector %s 2>&1 | \
// RUN: FileCheck %s -check-prefix=CHECK-SOFTFLT-P10VEC

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -std=c++11 -msoft-float -mdirect-move %s 2>&1 | \
// RUN: FileCheck %s -check-prefix=CHECK-SOFTFLT-DIRECTMOVE

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -std=c++11 -msoft-float -mmma %s 2>&1 | \
// RUN: FileCheck %s -check-prefix=CHECK-SOFTFLT-MMA

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -std=c++11 -msoft-float -mpaired-vector-memops %s 2>&1 | \
// RUN: FileCheck %s -check-prefix=CHECK-SOFTFLT-PAIREDVECMEMOP

// RUN: not %clang -target powerpc64le-unknown-unknown -fsyntax-only \
// RUN: -std=c++11 -msoft-float -mcrypto %s 2>&1 | \
// RUN: FileCheck %s -check-prefix=CHECK-SOFTFLT-CRYPTO

#ifdef __VSX__
static_assert(false, "VSX enabled");
#endif

#ifdef __POWER8_VECTOR__
static_assert(false, "P8V enabled");
#endif

#ifdef __POWER9_VECTOR__
static_assert(false, "P9V enabled");
#endif

#ifdef __POWER10_VECTOR__
static_assert(false, "P10V enabled");
#endif

#if !defined(__VSX__) && !defined(__POWER8_VECTOR__) && \
    !defined(__POWER9_VECTOR__)
static_assert(false, "Neither enabled");
#endif

// CHECK-DEFAULT: VSX enabled
// CHECK-DEFAULT: P8V enabled
// CHECK-DEFAULT-P9: P9V enabled
// CHECK-DEFAULT-P10: P10V enabled
// CHECK-NVSX-P8V: error: option '-mpower8-vector' cannot be specified with '-mno-vsx'
// CHECK-NVSX-P9V: error: option '-mpower9-vector' cannot be specified with '-mno-vsx'
// CHECK-NVSX-P10V: error: option '-mpower10-vector' cannot be specified with '-mno-vsx'
// CHECK-NVSX-FLT128: error: option '-mfloat128' cannot be specified with '-mno-vsx'
// CHECK-NVSX-DMV: error: option '-mdirect-move' cannot be specified with '-mno-vsx'
// CHECK-NVSX-PAIRED-VEC-MEMOPS: error: option '-mpaired-vector-memops' cannot be specified with '-mno-vsx'
// CHECK-NVSX-MULTI: error: option '-mfloat128' cannot be specified with '-mno-vsx'
// CHECK-NVSX-MULTI: error: option '-mpower9-vector' cannot be specified with '-mno-vsx'
// CHECK-NVSX-MMA: error: option '-mmma' cannot be specified with '-mno-vsx'
// CHECK-NVSX: Neither enabled
// CHECK-VSX: VSX enabled
// CHECK-NALTI-VSX: error: option '-mvsx' cannot be specified with '-mno-altivec'
// CHECK-SOFTFLT-ALTI: error: option '-maltivec' cannot be specified with '-msoft-float'
// CHECK-SOFTFLT-VSX: error: option '-mvsx' cannot be specified with '-msoft-float'
// CHECK-SOFTFLT-FLOAT128: error: option '-mfloat128' cannot be specified with '-msoft-float'
// CHECK-SOFTFLT-P8VEC: error: option '-mpower8-vector' cannot be specified with '-msoft-float'
// CHECK-SOFTFLT-P9VEC: error: option '-mpower9-vector' cannot be specified with '-msoft-float'
// CHECK-SOFTFLT-P10VEC: error: option '-mpower10-vector' cannot be specified with '-msoft-float'
// CHECK-SOFTFLT-DIRECTMOVE: error: option '-mdirect-move' cannot be specified with '-msoft-float'
// CHECK-SOFTFLT-MMA: error: option '-mmma' cannot be specified with '-msoft-float'
// CHECK-SOFTFLT-PAIREDVECMEMOP: error: option '-mpaired-vector-memops' cannot be specified with '-msoft-float'
// CHECK-SOFTFLT-CRYPTO: error: option '-mcrypto' cannot be specified with '-msoft-float'
