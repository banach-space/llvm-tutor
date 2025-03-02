//=============================================================================
// FILE:
//    RIV.cpp
//
// DESCRIPTION:
//    For every basic block  in the input function, this pass creates a list of
//    integer values reachable from that block. It uses the results of the
//    DominatorTree pass.
//
// ALGORITHM:
//    -------------------------------------------------------------------------
//    v_N = set of integer values defined in basic block N (BB_N)
//    RIV_N = set of reachable integer values for basic block N (BB_N)
//    -------------------------------------------------------------------------
//    STEP 1:
//    For every BB_N in F:
//      compute v_N and store it in DefinedValuesMap
//    -------------------------------------------------------------------------
//    STEP 2:
//    Compute the RIVs for the entry block (BB_0):
//      RIV_0 = {input args, global vars}
//    -------------------------------------------------------------------------
//    STEP 3: Traverse the CFG and for every BB_M that BB_N dominates,
//    calculate RIV_M as follows:
//      RIV_M = {RIV_N, v_N}
//    -------------------------------------------------------------------------
//
// REFERENCES:
//    Based on examples from:
//    "Building, Testing and Debugging a Simple out-of-tree LLVM Pass", Serge
//    Guelton and Adrien Guinet, LLVM Dev Meeting 2015
//
// License: MIT
//=============================================================================
#include "RIV.h"

#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Format.h"

#include <deque>

using namespace llvm;

// DominatorTree node types used in RIV. One could use auto instead, but IMO
// being verbose makes it easier to follow.
using NodeTy = DomTreeNodeBase<llvm::BasicBlock> *;
// A map that a basic block BB holds a set of pointers to values defined in BB.
using DefValMapTy = RIV::Result;

// Pretty-prints the result of this analysis
static void printRIVResult(llvm::raw_ostream &OutS, const RIV::Result &RIVMap);

//-----------------------------------------------------------------------------
// RIV Implementation
//-----------------------------------------------------------------------------
RIV::Result RIV::buildRIV(Function &F, NodeTy CFGRoot) {
  Result ResultMap;

  // Initialise a double-ended queue that will be used to traverse all BBs in F
  std::deque<NodeTy> BBsToProcess;
  BBsToProcess.push_back(CFGRoot);

  // STEP 1: For every basic block BB compute the set of integer values defined
  // in BB
  DefValMapTy DefinedValuesMap;
  for (BasicBlock &BB : F) {
    auto &Values = DefinedValuesMap[&BB];
    for (Instruction &Inst : BB)
      if (Inst.getType()->isIntegerTy())
        Values.insert(&Inst);
  }

  // STEP 2: Compute the RIVs for the entry BB. This will include global
  // variables and input arguments.
  auto &EntryBBValues = ResultMap[&F.getEntryBlock()];

  for (auto &Global : F.getParent()->globals())
    if (Global.getValueType()->isIntegerTy())
      EntryBBValues.insert(&Global);

  for (Argument &Arg : F.args())
    if (Arg.getType()->isIntegerTy())
      EntryBBValues.insert(&Arg);

  // STEP 3: Traverse the CFG for every BB in F calculate its RIVs
  while (!BBsToProcess.empty()) {
    auto *Parent = BBsToProcess.back();
    BBsToProcess.pop_back();

    // Get the values defined in Parent
    auto &ParentDefs = DefinedValuesMap[Parent->getBlock()];
    // Get the RIV set of for Parent
    // (Since RIVMap is updated on every iteration, its contents are likely to
    // be moved around when resizing. This means that we need a copy of it
    // (i.e. a reference is not sufficient).
    llvm::SmallPtrSet<llvm::Value *, 8> ParentRIVs =
        ResultMap[Parent->getBlock()];

    // Loop over all BBs that Parent dominates and update their RIV sets
    for (NodeTy Child : *Parent) {
      BBsToProcess.push_back(Child);
      auto ChildBB = Child->getBlock();

      // Add values defined in Parent to the current child's set of RIV
      ResultMap[ChildBB].insert(ParentDefs.begin(), ParentDefs.end());

      // Add Parent's set of RIVs to the current child's RIV
      ResultMap[ChildBB].insert(ParentRIVs.begin(), ParentRIVs.end());
    }
  }

  return ResultMap;
}

RIV::Result RIV::run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM) {
  DominatorTree *DT = &FAM.getResult<DominatorTreeAnalysis>(F);
  Result Res = buildRIV(F, DT->getRootNode());

  return Res;
}

PreservedAnalyses RIVPrinter::run(Function &Func,
                                  FunctionAnalysisManager &FAM) {

  auto RIVMap = FAM.getResult<RIV>(Func);

  printRIVResult(OS, RIVMap);
  return PreservedAnalyses::all();
}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
AnalysisKey RIV::Key;

llvm::PassPluginLibraryInfo getRIVPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "riv", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            // #1 REGISTRATION FOR "opt -passes=print<riv>"
            PB.registerPipelineParsingCallback(
                [&](StringRef Name, FunctionPassManager &FPM,
                    ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "print<riv>") {
                    FPM.addPass(RIVPrinter(llvm::errs()));
                    return true;
                  }
                  return false;
                });
            // #2 REGISTRATION FOR "FAM.getResult<RIV>(Function)"
            PB.registerAnalysisRegistrationCallback(
                [](FunctionAnalysisManager &FAM) {
                  FAM.registerPass([&] { return RIV(); });
                });
          }};
};

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getRIVPluginInfo();
}

//------------------------------------------------------------------------------
// Helper functions
//------------------------------------------------------------------------------
static void printRIVResult(raw_ostream &OutS, const RIV::Result &RIVMap) {
  OutS << "=================================================\n";
  OutS << "LLVM-TUTOR: RIV analysis results\n";
  OutS << "=================================================\n";

  const char *Str1 = "BB id";
  const char *Str2 = "Reachable Integer Values";
  OutS << format("%-10s %-30s\n", Str1, Str2);
  OutS << "-------------------------------------------------\n";

  const char *EmptyStr = "";

  for (auto const &KV : RIVMap) {
    std::string DummyStr;
    raw_string_ostream BBIdStream(DummyStr);
    KV.first->printAsOperand(BBIdStream, false);
    OutS << format("BB %-12s %-30s\n", BBIdStream.str().c_str(), EmptyStr);
    for (auto const *IntegerValue : KV.second) {
      std::string DummyStr;
      raw_string_ostream InstrStr(DummyStr);
      IntegerValue->print(InstrStr);
      OutS << format("%-12s %-30s\n", EmptyStr, InstrStr.str().c_str());
    }
  }

  OutS << "\n\n";
}
