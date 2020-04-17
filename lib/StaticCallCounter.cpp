//==============================================================================
// FILE:
//    StaticCallCounter.cpp
//
// DESCRIPTION:
//    Counts the number of static function calls in the input module. `Static`
//    refers to the fact that the analysed functions calls are compile-time
//    calls (as opposed to `dynamic`, i.e. run-time). Only direct function
//    calls are considered. Calls via functions pointers are not taken into
//    account.
//
//    This is a reference analysis pass:
//      * it inherits from llvm::AnalysisInfoMixin (New PM API)
//      * it implements a print method (Legacy PM API)
//
//    The `print` method (Legacy PM) is called when running opt with the
//    `-analyze` flag. As the new PM has no equivalent of the `print method, it
//    is currently not possible to print the results of this pass when:
//      * running StaticCalCounter through opt and using the new PM.
//    However, StaticCallCounter does implement the new PM interface.
//    It is used in `static`, a tool implemented in tools/StaticMain.cpp that
//    is a wrapper around StaticCallCounter and that can be used as a
//    standalone tool. `static` always prints the results of the analysis.
//
// USAGE:
//    1. Legacy PM - run through opt:
//      opt -load <BUILD/DIR>/lib/libStaticCallCounter.so --legacy-static-cc
//      -analyze <input-llvm-file>
//    2. New PM - run through 'static':
//      <BUILD/DIR>/bin/static <input-llvm-file>
//
// License: MIT
//==============================================================================
#include "StaticCallCounter.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;

//------------------------------------------------------------------------------
// StaticCallCounter Implementation
//------------------------------------------------------------------------------
StaticCallCounter::Result StaticCallCounter::runOnModule(Module &M) {
  llvm::MapVector<const llvm::Function *, unsigned> Res;

  for (auto &Func : M) {
    for (auto &BB : Func) {
      for (auto &Ins : BB) {
        // As per the comments in CallSite.h (more specifically, comments for
        // the base class CallSiteBase), ImmutableCallSite constructor creates
        // a valid call-site or NULL for something which is NOT a call site.
        auto ICS = ImmutableCallSite(&Ins);
        if (nullptr == ICS.getInstruction()) {
          continue;
        }

        // Check whether the called function is directly invoked
        auto DirectInvoc =
            dyn_cast<Function>(ICS.getCalledValue()->stripPointerCasts());
        if (nullptr == DirectInvoc) {
          continue;
        }

        // Update the count for the particular call
        auto CallCount = Res.find(DirectInvoc);
        if (Res.end() == CallCount) {
          CallCount = Res.insert(std::make_pair(DirectInvoc, 0)).first;
        }
        ++CallCount->second;
      }
    }
  }

  return Res;
}

StaticCallCounter::Result
StaticCallCounter::run(llvm::Module &M, llvm::ModuleAnalysisManager &) {
  return runOnModule(M);
}

bool LegacyStaticCallCounter::runOnModule(llvm::Module &M) {
  DirectCalls = Impl.runOnModule(M);
  return false;
}

void LegacyStaticCallCounter::print(raw_ostream &OutS, Module const *) const {
  printStaticCCResult(OutS, DirectCalls);
}

//------------------------------------------------------------------------------
// New PM Registration
//------------------------------------------------------------------------------
AnalysisKey StaticCallCounter::Key;

llvm::PassPluginLibraryInfo getStaticCallCounterPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "static-cc", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerAnalysisRegistrationCallback(
                [](ModuleAnalysisManager &MAM) {
                  MAM.registerPass([&] { return StaticCallCounter(); });
                });
          }};
};

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getStaticCallCounterPluginInfo();
}

//------------------------------------------------------------------------------
// Legacy PM Registration
//------------------------------------------------------------------------------
char LegacyStaticCallCounter::ID = 0;

// Register the pass - required for (among others) opt
RegisterPass<LegacyStaticCallCounter>
    X(/*PassArg=*/"legacy-static-cc",
      /*Name=*/"LegacyStaticCallCounter",
      /*CFGOnly=*/true,
      /*is_analysis=*/true);

//------------------------------------------------------------------------------
// Helper functions
//------------------------------------------------------------------------------
void printStaticCCResult(raw_ostream &OutS, const ResultStaticCC &DirectCalls) {
  OutS << "================================================="
       << "\n";
  OutS << "LLVM-TUTOR: static analysis results\n";
  OutS << "=================================================\n";
  const char *str1 = "NAME";
  const char *str2 = "#N DIRECT CALLS";
  OutS << format("%-20s %-10s\n", str1, str2);
  OutS << "-------------------------------------------------"
       << "\n";

  for (auto &CallCount : DirectCalls) {
    OutS << format("%-20s %-10lu\n", CallCount.first->getName().str().c_str(),
                   CallCount.second);
  }

  OutS << "-------------------------------------------------"
       << "\n\n";
}
