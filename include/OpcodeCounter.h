//========================================================================
// FILE:
//    OpcodeCounter.h
//
// DESCRIPTION:
//    Declares the OpcodeCounter Pass
//
// License: MIT
//========================================================================
#ifndef LLVM_TUTOR_OPCODECOUNTER_H
#define LLVM_TUTOR_OPCODECOUNTER_H

#include "llvm/ADT/StringMap.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

using ResultOpcodeCounter = llvm::StringMap<unsigned>;

//------------------------------------------------------------------------------
// New PM interface
//------------------------------------------------------------------------------
struct OpcodeCounter : public llvm::PassInfoMixin<OpcodeCounter> {
  using Result = ResultOpcodeCounter;
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &);
  OpcodeCounter::Result generateOpcodeMap(llvm::Function &F);
};

//------------------------------------------------------------------------------
// Legacy PM interface
//------------------------------------------------------------------------------
struct LegacyOpcodeCounter : public llvm::FunctionPass {
  static char ID;
  LegacyOpcodeCounter() : llvm::FunctionPass(ID) {}
  bool runOnFunction(llvm::Function &F) override;

  OpcodeCounter Impl;
};

//------------------------------------------------------------------------------
// Helper functions
//------------------------------------------------------------------------------
// Pretty-prints the result of this analysis
void printOpcodeCounterResult(const ResultOpcodeCounter &OC,
                              llvm::StringRef FuncName);
#endif
