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
//      opt -load libHelloWorld.dylib -hello-world -disable-output <input-llvm-file>
//      # `HelloWorld` will be executed as part of the optimisation pipelines
//      opt -load libHelloWorld.dylib -O{0|1|2|3} -disable-output <input-llvm-file>
//    2. New pass manager
//      # Define your pass pipeline via the '-passes' flag
//      opt -load-pass-plugin=libHelloWorld.dylib -passes="HelloWorld" -disable-output <input-llvm-file>
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

// Legacy PM implementation
struct LegacyHelloWorld : public FunctionPass {
  static char ID;
  LegacyHelloWorld() : FunctionPass(ID) {}
  bool runOnFunction(Function &F) override {
    visitor(F);
    return false;
  }
};

// New PM implementation
struct HelloWorld : PassInfoMixin<HelloWorld> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    visitor(F);
    return PreservedAnalyses::all();
  }
};

} // namespace

//-----------------------------------------------------------------------------
// Legacy PM Registration
//-----------------------------------------------------------------------------
char LegacyHelloWorld::ID = 0;

static RegisterPass<LegacyHelloWorld>
    X("hello-world", "Hello World Pass",
      true, // doesn't modify the CFG => true
      true  // not a pure analysis pass => false
    );

#ifndef LT_TRAVIS_DISABLE
// This trips Travis. This is a somewhat know issues:
//    https://github.com/sampsyo/llvm-pass-skeleton/issues/7
// I've tried all of the suggestions, but Travis continues to seg-fault.
static llvm::RegisterStandardPasses RegisterHelloWorld(
    llvm::PassManagerBuilder::EP_EarlyAsPossible,
    [](const llvm::PassManagerBuilder &Builder,
       llvm::legacy::PassManagerBase &PM) { PM.add(new LegacyHelloWorld()); });
#endif

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
// With this `opt` will be able to recognize HelloWorld when added to the pass
// pipeline on the command line, e.g. via `-passes="HelloWorld"
llvm::PassPluginLibraryInfo getHelloWorldPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "HelloWorld", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "HelloWorld") {
                    FPM.addPass(HelloWorld());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getHelloWorldPluginInfo();
}
