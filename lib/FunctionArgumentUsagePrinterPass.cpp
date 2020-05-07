//========================================================================
// FILE:
//    FunctionArgumentUsagePass.cpp
//
// DESCRIPTION:
// This is a user of the FunctionArgumentUsagePass. It requires to trigger
// the analysis pass using the modern pass manager.
//
// USAGE:
//    1. New pass manager:
//      $ opt -load-pass-plugin <BUILD_DIR>/lib/libFunctionArgumentUsage.so \
//        -passes="fnargusage-user" -disable-output <bitcode-file>
//
// EXAMPLE OUTPUT:
// Printing analysis 'Function Argument Usage Pass' for function 'demo':
// Function 'main' call on line '16': argument type mismatch. Argument #1
//   Expected 'i32*' but argument is of type 'i32**'
// Function 'callee' call on line '6': argument type mismatch. Argument #1
//   Expected 'i32*' but argument is of type 'i8*'
//
// License: MIT
//========================================================================

#include "FunctionArgumentUsagePass.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;

namespace {

class FunctionArgumentUsagePrinterPass
    : public PassInfoMixin<FunctionArgumentUsagePass> {
  raw_ostream &OS;

public:
  explicit FunctionArgumentUsagePrinterPass(raw_ostream &OS) : OS(OS) {};

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

PreservedAnalyses
FunctionArgumentUsagePrinterPass::run(Function &F,
                                      FunctionAnalysisManager &AM) {
  OS << "Printing analysis 'Function Argument Usage Pass' for function '";
  OS.write_escaped(F.getName());
  OS << "'\n";
  FunctionArgumentUsagePass::Result Result = AM.getResult<FunctionArgumentUsagePass>(F);
  printTypeMismatches(OS, Result);
  return PreservedAnalyses::all();
}

} // end anonymous namespace

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
PassPluginLibraryInfo getFunctionArgumentUsagePluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "fnargusage", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerAnalysisRegistrationCallback(
                [](FunctionAnalysisManager &FAM) {
                  FAM.registerPass([&] { return FunctionArgumentUsagePass(); });
                });
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "fnargusage-user") {
                    FPM.addPass(FunctionArgumentUsagePrinterPass(errs()));
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getFunctionArgumentUsagePluginInfo();
}
