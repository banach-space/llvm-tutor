//=============================================================================
// FILE:
//    HelloWorld.cpp
//
// DESCRIPTION:
//    Visits all functions in a module, prints their names and the number of
//    arguments via stderr. Strictly speaking, this is an analysis pass (i.e.
//    the functions are not modified). However, in order to keep things simple
//    there's no 'print' method here, which every analysis pass should
//    implement.
//
// USAGE:
//    1. Legacy pass manager
//      # Request `HelloWorld` via a dedicated flag:
//      opt -load libHelloWorld.dylib -legacy-hello-world -disable-output <input-llvm-file>
//      # `HelloWorld` will be executed as part of the optimisation pipelines
//      opt -load libHelloWorld.dylib -O{0|1|2|3} -disable-output <input-llvm-file>
//    2. New pass manager
//      # Define your pass pipeline via the '-passes' flag
//      opt -load-pass-plugin=libHelloWorld.dylib -passes="hello-world" -disable-output <input-llvm-file>
//
//
// License: MIT
//=============================================================================
#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

//-----------------------------------------------------------------------------
// HelloWorld implementation
//-----------------------------------------------------------------------------
// No need to expose the internals of the pass to the outside world - keep
// everything in an anonymous namespace.
namespace {

// This method implements what the pass does
void visitor(Function &F) {
    errs() << "Visiting: ";
    errs() << F.getName() << " (takes ";
    errs() << F.arg_size() << " args)\n";
}

// New PM implementation
struct HelloWorld : PassInfoMixin<HelloWorld> {
  // Main entry point, takes IR unit to run the pass on (&F) and the
  // corresponding pass manager (to be queried if need be)
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    visitor(F);
    return PreservedAnalyses::all();
  }
};

// Legacy PM implementation
struct LegacyHelloWorld : public FunctionPass {
  static char ID;
  LegacyHelloWorld() : FunctionPass(ID) {}
  // Main entry point - the name conveys what unit of IR this is to be run on.
  bool runOnFunction(Function &F) override {
    visitor(F);
    // Doesn't modify the input unit of IR, hence 'false'
    return false;
  }
};
} // namespace

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getHelloWorldPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "HelloWorld", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "hello-world") {
                    FPM.addPass(HelloWorld());
                    return true;
                  }
                  return false;
                });
          }};
}

// This is the core interface for pass plugins - with this 'opt' will be able
// to recognize HelloWorld when added to the pass pipeline on the command line,
// i.e. via '-passes=hello-world'
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getHelloWorldPluginInfo();
}

//-----------------------------------------------------------------------------
// Legacy PM Registration
//-----------------------------------------------------------------------------
// The address of this variable is used to identify the pass. The actual value
// doesn't matter.
char LegacyHelloWorld::ID = 0;

// Register the pass - with this 'opt' will be able to recognize
// LegacyHelloWorld when added to the pass pipeline on the command line, i.e.
// via '--legacy-hello-world'
static RegisterPass<LegacyHelloWorld>
    X("legacy-hello-world", "Hello World Pass",
      true,  // This pass doesn't modify the CFG => true
      false  // This pass is not a pure analysis pass => false
    );

// Register LegacyHelloWorld as a step of an existing pipeline. The insertion
// point is set to 'EP_EarlyAsPossible', which means that LegacyHelloWorld will
// be run automatically at '-O{0|1|2|3}'.
#ifndef LT_LEGACY_SKIP_PIPELINE_REGISTRATION
// This trips 'opt' installed via HomeBrew. It's a known issues:
//    https://github.com/sampsyo/llvm-pass-skeleton/issues/7
// I've tried all of the suggestions, but no luck. Locally I recommend either
// building from sources or commenting this out.
// Note: AFAIK, this is Mac OS only problem.
static llvm::RegisterStandardPasses RegisterHelloWorld(
    llvm::PassManagerBuilder::EP_EarlyAsPossible,
    [](const llvm::PassManagerBuilder &Builder,
       llvm::legacy::PassManagerBase &PM) { PM.add(new LegacyHelloWorld()); });
#endif
