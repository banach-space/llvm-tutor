//========================================================================
// FILE:
//    StaticCallCounter.cpp
//
// DESCRIPTION:
//    Counts the number of direct function calls (i.e. as seen in the source
//    code) in a file.
//
// USAGE:
//    This pass can be run through opt like this:
//      opt -load <BUILD/DIR>/lib/ --cc-static -analyze <input-llvm-file>
//    You can also run it through 'static':
//      <BUILD/DIR>/bin/static <input-llvm-file>
//
// License: MIT
//========================================================================
#include "StaticCallCounter.h"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/Format.h"

using namespace llvm;
using lt::StaticCallCounter;

namespace lt {

char StaticCallCounter::ID = 0;

// Register the pass - required for (among others) opt
RegisterPass<StaticCallCounter> X("static-cc",
                                  "For each function print the number of direct calls",
                                  true /* Only looks at CFG */,
                                  true /* Analysis Pass */);
} // namespace lt

// For an analysis pass, runOnModule should perform the actual analysis and
// compute the results. The actual output, however, is produced separately.
bool StaticCallCounter::runOnModule(Module &M) {
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
        auto CallCount = DirectCalls.find(DirectInvoc);
        if (DirectCalls.end() == CallCount) {
          CallCount = DirectCalls.insert(std::make_pair(DirectInvoc, 0)).first;
        }
        ++CallCount->second;
      }
    }
  }

  return false;
}

// The print method must be implemented by “analyses” in order to print a human
// readable version of the analysis results. 
// http://llvm.org/docs/WritingAnLLVMPass.html#the-print-method
void StaticCallCounter::print(raw_ostream &OutS, Module const *) const {
  OutS << "=================================================" << "\n";
  OutS << "LLVM-TUTOR: static analysis results\n";
  OutS << "=================================================\n";
  const char *str1 = "NAME";
  const char *str2 = "#N DIRECT CALLS";
  OutS << format("%-20s %-10s\n", str1, str2);
  OutS << "-------------------------------------------------" << "\n";

  // Generate a vector of captured functions, sorted alphabetically by function
  // names. The solution implemented here is a suboptimal - a separate
  // container with functions is created for sorting.
  // TODO Make this more elegant (i.e. avoid creating a separate container)
  std::vector<const Function*> FuncNames;
  FuncNames.reserve(DirectCalls.size());
  for (auto &CallCount : DirectCalls) {
    FuncNames.push_back(CallCount.getFirst());
  }
  std::sort(FuncNames.begin(), FuncNames.end(), [](const Function *x, const Function
        *y){ return (x->getName().str() < y->getName().str()); });

  // Print functions (alphabetically)
  for (auto &Func : FuncNames) {
    unsigned NumDirectCalls = (DirectCalls.find(Func))->getSecond();
    OutS << format("%-20s %-10lu\n", Func->getName().str().c_str(), NumDirectCalls);
  }
}
