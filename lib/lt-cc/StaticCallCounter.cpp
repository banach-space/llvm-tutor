//========================================================================
// FILE:
//    StaticCallCounter.cpp
//
// AUTHOR:
//    banach-space@github
//
// DESCRIPTION:
//    Counts the static function calls
//
//    Counts the number of static direct function calls.
//
//    This pass can be run through lt-cc as well ass opt. For the latter:
//      $ opt -load <BUILD_DIR>/lib/liblt-lib.so --lt -analyze <bitcode-file>
//
// License: MIT
//========================================================================
#include <iomanip>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/Format.h"

#include "StaticCallCounter.h"

using namespace llvm;
using lt::StaticCallCounter;

namespace lt {

char StaticCallCounter::ID = 0;

// Register the pass - required for (among others) opt
RegisterPass<StaticCallCounter> X("lt",
                                  "Print the static count of direct calls",
                                  true /* Only looks at CFG */,
                                  true /* Analysis Pass */);
} // namespace lt

// For an analysis pass, runOnModule should perform the actual analysis and
// compute the results. The actual output, however, is produced separately.
bool StaticCallCounter::runOnModule(Module &m) {
  for (auto &f : m) {
    for (auto &bb : f) {
      for (auto &i : bb) {
        // As per the comments in CallSite.h (more specifically, comments for
        // the base class CallSiteBase), ImmutableCallSite constructor creates
        // a valid call-site or NULL for something which is NOT a call site.
        auto ims = ImmutableCallSite(&i);

        // Check whether the instruction is actually a call/invoke
        if (nullptr == ims.getInstruction()) {
          continue;
        }

        // Check whether the called function is directly invoked
        auto called =
            dyn_cast<Function>(ims.getCalledValue()->stripPointerCasts());
        if (nullptr == called) {
          continue;
        }

        // Update the count for the particular call
        auto count = counts.find(called);
        if (counts.end() == count) {
          count = counts.insert(std::make_pair(called, 0)).first;
        }
        ++count->second;
      }
    }
  }

  return false;
}

// The print method must be implemented by “analyses” in order to print a human
// readable version of the analysis results. 
void StaticCallCounter::print(raw_ostream &out, Module const * /*m*/) const {
  out << "=================================================" << "\n";
  out << "LLVM-TUTOR: static analysis results\n";
  out << "=================================================\n";
  const char *str1 = "NAME";
  const char *str2 = "#N DIRECT CALLS";
  out << format("%-20s %-10s\n", str1, str2);
  out << "-------------------------------------------------" << "\n";

  // Generate a vector of captured functions, sorted alphabetically by function
  // names. The solution implemented here is a suboptimal - a separate
  // container with functions is created for sorting.
  // TODO Make this more elegant (i.e. avoid creating a separate container)
  std::vector<const Function*> funcNames;
  funcNames.reserve(counts.size());
  for (auto &kvPair : counts) {
    funcNames.push_back(kvPair.getFirst());
  }
  std::sort(funcNames.begin(), funcNames.end(), [](const Function *x, const Function
        *y){ return (x->getName().str() < y->getName().str()); });

  // Print functions (alphabetically)
  for (auto &func : funcNames) {
    auto count = (counts.find(func))->getSecond();
    out << format("%-20s %-10lu\n", func->getName().str().c_str(), count);
  }
}
