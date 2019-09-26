//========================================================================
// FILE:
//    StaticCallCounter.cpp
//
// DESCRIPTION:
//    Counts the number of direct function calls (i.e. as seen in the source
//    code) in a file.
//
// USAGE:
//    1. Run through opt - legacy pass manager
//      opt -load <BUILD/DIR>/lib/libStaticCallCounter.so --legacy-static-cc
//      -analyze <input-llvm-file>
//    2. You can also run it through 'static':
//      <BUILD/DIR>/bin/static <input-llvm-file>
//
// License: MIT
//========================================================================
#include "StaticCallCounter.h"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Format.h"

using namespace llvm;

//-----------------------------------------------------------------------------
// StaticCallCounter Implementation
//-----------------------------------------------------------------------------
AnalysisKey StaticCallCounter::Key;

StaticCallCounter::Result StaticCallCounter::runOnModule(Module &M) {
  llvm::DenseMap<const llvm::Function *, unsigned> Res;

  for (auto &Func : M) {
    for (auto &BB : Func) {
      for (auto &Ins : BB) {
        // As per the comments in CallSite.h (more specifically, comments for
        // the base class CallSiteBase), ImmutableCallSite constructor creates
        // a valid call-site or NULL for something which is NOT a call site.
        auto ICS = ImmutableCallSite(&Ins);

        // Check whether the instruction is actually a call/invoke
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

void LegacyStaticCallCounter::print(raw_ostream &OutS, Module const *) const {
  printStaticCCResult(OutS, DirectCalls);
}

bool LegacyStaticCallCounter::runOnModule(llvm::Module &M) {
  DirectCalls = Impl.runOnModule(M);
  return false;
}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
// Legacy PM Registration
//-----------------------------------------------------------------------------
char LegacyStaticCallCounter::ID = 0;

// Register the pass - required for (among others) opt
RegisterPass<LegacyStaticCallCounter>
    X("legacy-static-cc", "For each function print the number of direct calls",
      true, // Doesn't modify the CFG => true
      true  // It's a pure analysis pass => true
    );

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

  // Generate a vector of captured functions, sorted alphabetically by function
  // names. The solution implemented here is a suboptimal - a separate
  // container with functions is created for sorting.
  // TODO Make this more elegant (i.e. avoid creating a separate container)
  std::vector<const Function *> FuncNames;
  FuncNames.reserve(DirectCalls.size());
  for (auto &CallCount : DirectCalls) {
    FuncNames.push_back(CallCount.getFirst());
  }
  std::sort(FuncNames.begin(), FuncNames.end(),
            [](const Function *x, const Function *y) {
              return (x->getName().str() < y->getName().str());
            });

  // Print functions (alphabetically)
  for (auto &Func : FuncNames) {
    unsigned NumDirectCalls = (DirectCalls.find(Func))->getSecond();
    OutS << format("%-20s %-10lu\n", Func->getName().str().c_str(),
                   NumDirectCalls);
  }
}
