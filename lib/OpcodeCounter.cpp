//=============================================================================
// FILE:
//    OpcodeCounter.cpp
//
// DESCRIPTION:
//    Visits all instructions in a function and counts how many times every
//    LLVM IR opcode was used. The results are printed unconditionally. It's
//    an analysis pass (i.e. the input functions are not modified). However, in
//    order to keep things simple, it's implemented as a transformation pass.
//    In this respect it's similar to HelloWorld.
//
//    This example demonstrates how to insert your pass at one of the
//    predefined extension points, e.g. before any other transformations are
//    run (`EP_EarlyAsPossible`). This is achieved by creating an instance of
//    llvm::RegisterStandardPasses as demonstrated below.
//
// USAGE:
//    1. Legacy PM
//      opt -load libOpcodeCounter.dylib -legacy-opcode-counter\
//        -disable-output <input-llvm-file>
//    2. Automatically through an optimisation pipeline - legacy PM
//      opt -load libOpcodeCounter.dylib -O{0|1|2|3|s} -disable-output \
//        <input-llvm-file>
//    3. Automatically through an optimisation pipeline - new PM
//      opt -load-pass-plugin libOpcodeCounter.dylib --passes='default<O1>'\
//        -disable-output <input-llvm-file>
//
// License: MIT
//=============================================================================
#include "OpcodeCounter.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

//-----------------------------------------------------------------------------
// OpcodeCounter implementation
//-----------------------------------------------------------------------------
OpcodeCounter::Result OpcodeCounter::generateOpcodeMap(Function &Func) {
  OpcodeCounter::Result OpcodeMap;

  for (auto &BB : Func) {
    for (auto &Inst : BB) {
      StringRef Name = Inst.getOpcodeName();

      if (OpcodeMap.find(Name) == OpcodeMap.end()) {
        OpcodeMap[Inst.getOpcodeName()] = 1;
      } else {
        OpcodeMap[Inst.getOpcodeName()]++;
      }
    }
  }

  return OpcodeMap;
}

PreservedAnalyses OpcodeCounter::run(llvm::Function &Func,
                                     llvm::FunctionAnalysisManager &) {
  OpcodeCounter::Result OpcodeMap = generateOpcodeMap(Func);
  printOpcodeCounterResult(OpcodeMap, Func.getName());

  return llvm::PreservedAnalyses::all();
}

bool LegacyOpcodeCounter::runOnFunction(llvm::Function &Func) {
  OpcodeCounter::Result OpcodeMap = Impl.generateOpcodeMap(Func);
  printOpcodeCounterResult(OpcodeMap, Func.getName());

  return false;
}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
// Register OpcodeCounter as a step of an existing pipeline. The insertion
// point is specified by the 'registerVectorizerStartEPCallback' callback.
// More specifically, OpcodeCounter will be run automatically whenever the
// vectoriser is used (i.e. when using '-O{1|2|3|s}'.
llvm::PassPluginLibraryInfo getOpcodeCounterPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "OpcodeCounter", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerVectorizerStartEPCallback(
                [](llvm::FunctionPassManager &PM,
                   llvm::PassBuilder::OptimizationLevel Level) {
                  PM.addPass(OpcodeCounter());
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getOpcodeCounterPluginInfo();
}

//-----------------------------------------------------------------------------
// Legacy PM Registration
//-----------------------------------------------------------------------------
char LegacyOpcodeCounter::ID = 0;

static RegisterPass<LegacyOpcodeCounter> X(/*PassArg=*/"legacy-opcode-counter",
                                           /*Name=*/"OpcodeCounter Pass",
                                           /*CFGOnly=*/true,
                                           /*is_analysis=*/false);

// Register LegacyOpcodeCounter as a step of an existing pipeline. The insertion
// point is set to 'EP_EarlyAsPossible', which means that LegacyOpcodeCounter
// will be run automatically at '-O{0|1|2|3}'.
static llvm::RegisterStandardPasses
    RegisterOpcodeCounter(llvm::PassManagerBuilder::EP_EarlyAsPossible,
                          [](const llvm::PassManagerBuilder &Builder,
                             llvm::legacy::PassManagerBase &PM) {
                            PM.add(new LegacyOpcodeCounter());
                          });

//------------------------------------------------------------------------------
// Helper functions
//------------------------------------------------------------------------------
void printOpcodeCounterResult(const ResultOpcodeCounter &OpcodeMap,
                              StringRef FuncName) {
  llvm::errs() << "================================================="
               << "\n";
  llvm::errs() << "LLVM-TUTOR: OpcodeCounter results for `" << FuncName
               << "`\n";
  llvm::errs() << "=================================================\n";
  const char *str1 = "OPCODE";
  const char *str2 = "#N TIMES USED";
  llvm::errs() << format("%-20s %-10s\n", str1, str2);
  llvm::errs() << "-------------------------------------------------"
               << "\n";
  for (auto &Inst : OpcodeMap) {
    llvm::errs() << format("%-20s %-10lu\n", Inst.first().str().c_str(),
                           Inst.second);
  }
  llvm::errs() << "-------------------------------------------------"
               << "\n\n";
}
