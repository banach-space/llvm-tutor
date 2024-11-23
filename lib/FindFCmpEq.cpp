//=============================================================================
// FILE:
//    FindFCmpEq.cpp
//
// DESCRIPTION:
//    Visits all instructions in a function and returns all equality-based
//    floating point comparisons. The results can be printed through the use of
//    a printing pass.
//
//    This example demonstrates how to separate printing logic into a separate
//    printing pass, how to register it along with an analysis pass at the same
//    time, and how to parse pass pipeline elements to conditionally register a
//    pass. This is achieved using a combination of llvm::formatv() (not
//    strictly required),
//    llvm::PassBuilder::registerAnalysisRegistrationCallback(), and
//    llvm::PassBuilder::registerPipelineParsingCallback().
//
//    Originally developed for [1].
//
//    [1] "Writing an LLVM Optimization" by Jonathan Smith
//
// USAGE:
//      opt --load-pass-plugin libFindFCmpEq.dylib `\`
//        --passes='print<find-fcmp-eq>' --disable-output <input-llvm-file>
//
// License: MIT
//=============================================================================
#include "FindFCmpEq.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/ModuleSlotTracker.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/raw_ostream.h"

#include <string>

using namespace llvm;

static void
printFCmpEqInstructions(raw_ostream &OS, Function &Func,
                        const FindFCmpEq::Result &FCmpEqInsts) noexcept {
  if (FCmpEqInsts.empty())
    return;

  OS << "Floating-point equality comparisons in \"" << Func.getName()
     << "\":\n";

  // Using a ModuleSlotTracker for printing makes it so full function analysis
  // for slot numbering only occurs once instead of every time an instruction
  // is printed.
  ModuleSlotTracker Tracker(Func.getParent());

  for (FCmpInst *FCmpEq : FCmpEqInsts) {
    FCmpEq->print(OS, Tracker);
    OS << '\n';
  }
}

static constexpr char PassArg[] = "find-fcmp-eq";
static constexpr char PassName[] =
    "Floating-point equality comparisons locator";
static constexpr char PluginName[] = "FindFCmpEq";

//------------------------------------------------------------------------------
// FindFCmpEq implementation
//------------------------------------------------------------------------------
FindFCmpEq::Result FindFCmpEq::run(Function &Func,
                                   FunctionAnalysisManager &FAM) {
  return run(Func);
}

FindFCmpEq::Result FindFCmpEq::run(Function &Func) {
  Result Comparisons;
  for (Instruction &Inst : instructions(Func)) {
    // We're only looking for 'fcmp' instructions here.
    if (auto *FCmp = dyn_cast<FCmpInst>(&Inst)) {
      // We've found an 'fcmp' instruction; we need to make sure it's an
      // equality comparison.
      if (FCmp->isEquality()) {
        Comparisons.push_back(FCmp);
      }
    }
  }

  return Comparisons;
}

PreservedAnalyses FindFCmpEqPrinter::run(Function &Func,
                                         FunctionAnalysisManager &FAM) {
  auto &Comparisons = FAM.getResult<FindFCmpEq>(Func);
  printFCmpEqInstructions(OS, Func, Comparisons);
  return PreservedAnalyses::all();
}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::AnalysisKey FindFCmpEq::Key;

PassPluginLibraryInfo getFindFCmpEqPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, PluginName, LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            // #1 REGISTRATION FOR "FAM.getResult<FindFCmpEq>(Function)"
            PB.registerAnalysisRegistrationCallback(
                [](FunctionAnalysisManager &FAM) {
                  FAM.registerPass([&] { return FindFCmpEq(); });
                });
            // #2 REGISTRATION FOR "opt -passes=print<find-fcmp-eq>"
            // Printing passes format their pipeline element argument to the
            // pattern `print<pass-name>`. This is the pattern we're checking
            // for here.
            PB.registerPipelineParsingCallback(
                [&](StringRef Name, FunctionPassManager &FPM,
                    ArrayRef<PassBuilder::PipelineElement>) {
                  std::string PrinterPassElement =
                      formatv("print<{0}>", PassArg);
                  if (!Name.compare(PrinterPassElement)) {
                    FPM.addPass(FindFCmpEqPrinter(llvm::outs()));
                    return true;
                  }

                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getFindFCmpEqPluginInfo();
}
