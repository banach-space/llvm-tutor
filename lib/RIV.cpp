//=============================================================================
// FILE:
//    RIV.cpp
//
// DESCRIPTION:
//    The (R)eachable (I)nteger (V)alues pass. For each basic block it creates
//    a list of integer values reachable from that block. It uses the results
//    of the 'dominator trees' pass.
//
// ALGORITHM:
//    ----------------------------------------------------------
//    V = Visible values, D = Defined Values
//    vN = set of integer values defined in basic block N
//    ----------------------------------------------------------
//    STEP 1:
//    V = 0, D = 0, I = {input args, global vals}
//    ----------------------------------------------------------
//    STEP 2:
//    Iterate over all basic blocks and update V and D accordingly. On entry to
//    each basic block, set the list of reachable integer values to V.
//    BB 0:  v0 = ...    V = I, D = {v0}
//            |
//            v
//    BB 1:  v1 = ...    V = {v0}, D = {v1}
//            |
//            v
//    BB 2:  v2 = ...    V = {v0, v1}, D = {v2}
//            |
//            v
//          (...)
//    ----------------------------------------------------------
//
// License: MIT
//=============================================================================
#include "RIV.h"

#include "llvm/IR/Dominators.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"

#include <deque>

#define DEBUG_TYPE "riv"
using namespace llvm;

char RIV::ID = 0;
static RegisterPass<RIV> X("riv", "Compute Reachable Integer values",
                           true, // doesn't modify the CFG => true
                           true  // pure analysis pass => true
);

bool RIV::runOnFunction(Function &F) {
  // For each invocation of the compiler, there's only one instance of this pass
  // being created. As a result, the same instance of RIVMap is re-used for
  // every function that this pass is run on. For instance, if RIV has already
  // been run for some function 'foo', then RIVMap will contain the results for
  // 'foo'.  Hence the map neads to be cleared before analysing afunction.
  RIVMap.clear();

  // STEP 1. Compute sets of integer values defined for each Basic block
  RIVMapTy DefinedValuesMap;
  for (BasicBlock &BB : F) {
    auto &Values = DefinedValuesMap[&BB];
    for (Instruction &Inst : BB)
      if (Inst.getType()->isIntegerTy())
        Values.insert(&Inst);
  }

  // STEP 2. Compute the reachable integer values
  // Arguments and globals are always live
  auto &HeadValues = RIVMap[&F.getEntryBlock()];
  for (Argument &Arg : F.args())
    HeadValues.insert(&Arg);

  // Get the result from the Dominance Tree pass
  auto *Root =
      getAnalysis<DominatorTreeWrapperPass>().getDomTree().getRootNode();
  std::deque<decltype(Root)> NodesToProcess;
  NodesToProcess.push_back(Root);

  LLVM_DEBUG(errs() << "In Function: " << F.getName() << "\n");

  while (!NodesToProcess.empty()) {
    auto *NodeToProcess = NodesToProcess.back();
    NodesToProcess.pop_back();
    LLVM_DEBUG(errs() << "processing BB " << NodeToProcess->getBlock() << "\n");
    for (auto *Child : *NodeToProcess) {
      LLVM_DEBUG(errs() << "updating dominated child " << Child->getBlock()
                        << "\n");
      NodesToProcess.push_back(Child);
      {
        // Add defined values to dominated nodes
        auto &Values = DefinedValuesMap[NodeToProcess->getBlock()];
        RIVMap[Child->getBlock()].insert(Values.begin(), Values.end());
      }
      {
        // Add inherited values from dominating node
        auto &Values = RIVMap[NodeToProcess->getBlock()];
        RIVMap[Child->getBlock()].insert(Values.begin(), Values.end());
      }
    }
  }

  // Analysis pass => doesn't modify F => return false
  return false;
}

// Some guidance for PassManager:
//    * addRequired<DominatorTreeWrapperPass>() - needs the results from
//    Dominator Tree Pass
//    * setPreservesAll() - does not modify the LLVM program
//  More info:
// http://llvm.org/docs/WritingAnLLVMPass.html#specifying-interactions-between-passes
void RIV::getAnalysisUsage(AnalysisUsage &Info) const {
  Info.addRequired<DominatorTreeWrapperPass>();
  Info.setPreservesAll();
}

RIV::RIVMapTy const &RIV::getRIVMap() const { return RIVMap; }

void RIV::print(raw_ostream &out, Module const *) const {
  out << "=================================================\n";
  out << "LLVM-TUTOR: RIV analysis results\n";
  out << "=================================================\n";

  const char *Str1 = "BB id";
  const char *Str2 = "Reachable Ineger Values";
  out << format("%-6s %-30s\n", Str1, Str2);
  out << "-------------------------------------------------\n";

  const char *EmptyStr = "";

  // Generate a map of RIVs, sorted by BB ids.
  // TODO Make this more elegant (i.e. avoid creating a separate container)
  std::map<int, llvm::SmallPtrSet<llvm::Value *, 8>> RIVMapAlt;
  for (auto const &KvPair : RIVMap) {
    std::string DummyStr;
    raw_string_ostream BBIdStream(DummyStr);
    KvPair.first->printAsOperand(BBIdStream, false);
    // Strip the leading '%' and turn into an int
    int BBId = stoi(BBIdStream.str().erase(0, 1));

    RIVMapAlt[BBId].insert(KvPair.second.begin(), KvPair.second.end());
  }

  // Print the results, sorted by BB id
  for (auto const &KV : RIVMapAlt) {
    out << format("BB %-6d %-30s\n", KV.first, EmptyStr);
    for (auto const &IntegerValue : KV.second) {
      std::string DummyStr;
      raw_string_ostream InstrStr(DummyStr);
      IntegerValue->print(InstrStr);
      out << format("%-6s %-30s\n", EmptyStr, InstrStr.str().c_str());
    }
  }
}
