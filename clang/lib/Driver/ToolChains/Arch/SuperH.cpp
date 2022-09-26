//===--- SuperH.cpp - SuperH ToolChain Implementations ------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SuperH.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Options.h"
#include "llvm/Option/ArgList.h"

using namespace clang::driver;
using namespace clang::driver::toolchains;
using namespace clang;
using namespace llvm::opt;

SuperHToolChain::SuperHToolChain(const Driver &D, const llvm::Triple &Triple,
                               const ArgList &Args)
    : ToolChain(D, Triple, Args) {
  // ProgramPaths are found via 'PATH' environment variable.
}

bool SuperHToolChain::isPICDefault() const { return true; }

bool SuperHToolChain::isPIEDefault(const llvm::opt::ArgList &Args) const { return false; }

bool SuperHToolChain::isPICDefaultForced() const { return true; }

bool SuperHToolChain::SupportsProfiling() const { return false; }

bool SuperHToolChain::hasBlocksRuntime() const { return false; }