//========================================================================
// FILE:
//    StaticMain.cpp
//
// DESCRIPTION:
//    A command-line tool that counts all the static calls (i.e. calls as seen
//    in the source code) in the input LLVM file. Internally it uses the
//    StaticCallCounter pass.
//
// USAGE:
//    # First, generate an LLVM file
//    clang -emit-llvm <input-file> -o <output-llvm-file>
//    # Run this tool
//    <BUILD/DIR>/bin/static <output-llvm-file>
//
// License: MIT
//========================================================================
#include "StaticCallCounter.h"
#include "config.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using llvm::legacy::PassManager;

//===----------------------------------------------------------------------===//
// Command line options
//===----------------------------------------------------------------------===//
static cl::OptionCategory CallCounterCategory{"call counter options"};

static cl::opt<std::string> InputModule{cl::Positional,
                                   cl::desc{"<Module to analyze>"},
                                   cl::value_desc{"bitcode filename"},
                                   cl::init(""),
                                   cl::Required,
                                   cl::cat{CallCounterCategory}};

//===----------------------------------------------------------------------===//
// StaticCountWrapper pass
//
// Runs StaticCallCounter and prints the result
//===----------------------------------------------------------------------===//
struct StaticCountWrapper : public ModulePass {
  static char ID;
  raw_ostream &OutS;

  explicit StaticCountWrapper(raw_ostream &OS) : ModulePass(ID), OutS(OS) {}

  bool runOnModule(Module &M) override {
    // Prints the result of StaticCallCounter
    getAnalysis<lt::StaticCallCounter>().print(OutS, &M);
    return false;
  }

  // Set dependencies
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<lt::StaticCallCounter>();
    AU.setPreservesAll();
  }
};

char StaticCountWrapper::ID = 0;

static void countStaticCalls(Module &M) {
  // Build up all of the passes that we want to run on the module.
  legacy::PassManager PM;
  PM.add(new lt::StaticCallCounter());
  PM.add(new StaticCountWrapper(outs()));

  // Run them
  PM.run(M);
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//
int main(int Argc, char **Argv) {
  // Hide all options apart from the ones specific to this tool
  cl::HideUnrelatedOptions(CallCounterCategory);

  cl::ParseCommandLineOptions(Argc, Argv,
                              "Counts the number of static function "
                              "calls in the input IR file\n");

  // Makes shure llvm_shutdown() is called (which cleans up LLVM objects)
  //  http://llvm.org/docs/ProgrammersManual.html#ending-execution-with-llvm-shutdown
  llvm_shutdown_obj SDO;

  // Parse the IR file passed on the command line.
  SMDiagnostic Err;
  LLVMContext Ctx;
  std::unique_ptr<Module> M = parseIRFile(InputModule.getValue(), Err, Ctx);

  if (!M) {
    errs() << "Error reading bitcode file: " << InputModule << "\n";
    Err.print(Argv[0], errs());
    return -1;
  }

  // Run the analysis and print the results
  countStaticCalls(*M);

  return 0;
}
