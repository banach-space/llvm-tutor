//========================================================================
// FILE:
//    lt-cc-main.cpp
//
// DESCRIPTION:
//
// License: MIT
//========================================================================
#include "DynamicCallCounter.h"
#include "StaticCallCounter.h"
#include "config.h"

#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/CodeGen/CommandFlags.inc"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"

// Not used here per-se, but required by Debug.h
#define DEBUG_TYPE "cc"

using namespace llvm;
using llvm::legacy::PassManager;
using llvm::sys::ExecuteAndWait;
using llvm::sys::findProgramByName;
using std::string;
using std::unique_ptr;
using std::vector;

enum class AnalysisType {
  STATIC,
  DYNAMIC,
};

//===----------------------------------------------------------------------===//
// Command line (cl) options for lt
//===----------------------------------------------------------------------===//
static cl::OptionCategory CallCounterCategory{"call counter options"};

// This enables the LLVM_DEBUG macro
static cl::opt<bool, true> Debug("debug", cl::desc("Enable debug output"),
                                 cl::Hidden, cl::location(DebugFlag));

static cl::opt<string> InputModule{cl::Positional,
                                   cl::desc{"<Module to analyze>"},
                                   cl::value_desc{"bitcode filename"},
                                   cl::init(""),
                                   cl::Required,
                                   cl::cat{CallCounterCategory}};

static cl::opt<AnalysisType> AnalysisTy{
    cl::desc{"Select analyis type:"},
    cl::values(clEnumValN(AnalysisType::STATIC, "static",
                          "Count static direct calls."),
               clEnumValN(AnalysisType::DYNAMIC, "dynamic",
                          "Count dynamic direct calls.")),
    cl::Required, cl::cat{CallCounterCategory}};

static cl::opt<string> OutputModule{
    "o", cl::desc{"Filename of the instrumented program"},
    cl::value_desc{"filename"}, cl::Required, cl::cat{CallCounterCategory}};

enum OptLevel { O0 = 0, O1, O2, O3 };

cl::opt<OptLevel> optimizationLevel(
    cl::desc("Choose optimization level (default = 'O2'):"),
    cl::values(clEnumVal(O1, "Enable trivial optimizations"),
               clEnumVal(O2, "Enable default optimizations"),
               clEnumVal(O3, "Enable expensive optimizations")),
    cl::init(O2), cl::cat{CallCounterCategory});

static cl::list<string> LibPaths{
    "L", cl::Prefix, cl::desc{"Specify a library search path"},
    cl::value_desc{"directory"}, cl::cat{CallCounterCategory}};

static cl::list<string> Libraries{
    "l", cl::Prefix, cl::desc{"Specify Libraries to link against"},
    cl::value_desc{"library prefix"}, cl::cat{CallCounterCategory}};

//===----------------------------------------------------------------------===//
// The DynamicCountPrinter pass - wrappers
//===----------------------------------------------------------------------===//
static void compile(Module &M, StringRef OutputPath) {
  // 1. Get the target specific information (architecture, OS, etc)
  std::string ErrMsg;
  Triple triple = Triple(M.getTargetTriple());
  Target const *TheTarget = TargetRegistry::lookupTarget(MArch, triple, ErrMsg);

  if (nullptr == TheTarget) {
    report_fatal_error("Unable to find target:\n " + ErrMsg);
  }

  // 2. Generate the optimisation level
  CodeGenOpt::Level OptLevel = CodeGenOpt::Default;
  switch (optimizationLevel) {
  default:
    report_fatal_error("Invalid optimization level\n");
  case O0:
    OptLevel = CodeGenOpt::None;
    break;
  case O1:
    OptLevel = CodeGenOpt::Less;
    break;
  case O2:
    OptLevel = CodeGenOpt::Default;
    break;
  case O3:
    OptLevel = CodeGenOpt::Aggressive;
    break;
  }

  // 3. Generate the output file
  std::error_code ErrCode;
  auto OutputFile =
      std::make_unique<ToolOutputFile>(OutputPath, ErrCode, sys::fs::F_None);
  if (nullptr == OutputFile) {
    report_fatal_error("Unable to create the output file:\n " +
                       ErrCode.message());
  }

  // 4. Get TargetMachine (interface to the complete target machine description)
  std::string FeaturesStr;
  TargetOptions TO = InitTargetOptionsFromCodeGenFlags();
  unique_ptr<TargetMachine> TheMachine(TheTarget->createTargetMachine(
      triple.getTriple(), MCPU, FeaturesStr, TO, getRelocModel(),
      /* CodeModel */ None, OptLevel));
  assert((nullptr != TheMachine) && "Could not allocate target machine!");

  // 5. Set layout properties related to datatype size/offset/alignment
  M.setDataLayout(TheMachine->createDataLayout());

  // 6. Create the output stream for the output file
  raw_pwrite_stream *OutS(&OutputFile->os());
  std::unique_ptr<buffer_ostream> BOS;
  if (!OutputFile->os().supportsSeeking()) {
    BOS = std::make_unique<buffer_ostream>(*OutS);
    OutS = BOS.get();
  }

  // 7. Ask the target to add backend passes as necessary Build up all the
  // passes for the module object file generation
  legacy::PassManager PM;

  TargetLibraryInfoImpl TLI(triple);
  PM.add(new TargetLibraryInfoWrapperPass(TLI));
  FileType = TargetMachine::CGFT_ObjectFile;
  if (TheMachine->addPassesToEmitFile(PM, *OutS, /*DWO file */ nullptr,
                                      FileType,
                                      /*DisableVerify */ true)) {
    report_fatal_error("target does not support generation "
                       "of this file type!\n");
  }

  // 8. Finally, run the passes
  PM.run(M);

  // 9. Keep the output binary if we've been successful to this point.
  OutputFile->keep();
}

static void link(StringRef InObjFile, StringRef OutObjFile) {
  // Arguments for the linker
  SmallVector<const char *, 128> LinkerArgs;

  // Get the clang++ executable path
  auto ClangExecutable = sys::findProgramByName("clang++");
  if (!ClangExecutable) {
    report_fatal_error("Unable to find clang++");
  }
  LinkerArgs.push_back(ClangExecutable->c_str());

  // Optimisation level
  std::string opt("-O");
  opt += std::to_string(optimizationLevel);
  LinkerArgs.push_back(opt.c_str());

  // Output file
  LinkerArgs.push_back("-o");
  LinkerArgs.push_back(OutObjFile.data());

  // Input File
  LinkerArgs.push_back(InObjFile.data());

  // Search paths for libraries
  for (auto &libPath : LibPaths) {
    LinkerArgs.push_back("-L");
    LinkerArgs.push_back(libPath.c_str());
  }

  // Libraries to link
  for (auto &library : Libraries) {
    LinkerArgs.push_back("-l");
    LinkerArgs.push_back(library.c_str());
  }

  // This is needed for toStringRefArray to work
  LinkerArgs.push_back(nullptr);

  // ExecuteAndWait requires the arguments as a vector of StringRef
  auto LinkerArgsStrRef = llvm::toStringRefArray(LinkerArgs.data());

  LLVM_DEBUG(
      errs() << "LLVM-TUTOR - about to run:\n\t"; for (auto &arg
                                                       : LinkerArgsStrRef) {
        errs() << arg << " ";
      } errs() << "\n";);

  // Run the linker (via clang++)
  if (0 !=
      ExecuteAndWait(ClangExecutable.get(), LinkerArgsStrRef, None, {}, 0, 0)) {
    report_fatal_error("Unable to link output file");
  }
}

static void generateBinary(Module &M, StringRef ExecutableOut) {
  // Compile into an object
  string ObjectFile = ExecutableOut.str() + ".o";
  compile(M, ObjectFile);

  // Generate the executable
  LibPaths.push_back(LT_RT_LIBRARY_PATH);
  Libraries.push_back(LT_RUNTIME_LIB);
  link(ObjectFile, ExecutableOut);
}

static void saveModule(const Module &M, StringRef OutputFile) {
  std::error_code ErrC;
  raw_fd_ostream OutS(OutputFile.data(), ErrC, sys::fs::F_None);

  if (ErrC) {
    report_fatal_error("error saving llvm module to '" + OutputFile + "': \n" +
                       ErrC.message());
  }
  WriteBitcodeToFile(M, OutS);
}

static void countDynamicCalls(Module &M) {
  // 1. Initialize the native target for emitting object code.
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();

  // 2. Build up and run all of the passes that we want for the input module.
  legacy::PassManager PM;
  PM.add(new lt::DynamicCallCounter());
  PM.add(createVerifierPass());

  PM.run(M);

  // 3. Save the instrumented module and generate a binary from it
  saveModule(M, OutputModule + ".lt.bc");
  generateBinary(M, OutputModule);
}

//===----------------------------------------------------------------------===//
// The StaticCountPrinter pass - wrappers
//===----------------------------------------------------------------------===//
struct StaticCountPrinter : public ModulePass {
  static char ID;
  raw_ostream &OutS;

  explicit StaticCountPrinter(raw_ostream &OS) : ModulePass(ID), OutS(OS) {}

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

char StaticCountPrinter::ID = 0;

static void countStaticCalls(Module &M) {
  // Build up all of the passes that we want to run on the module.
  legacy::PassManager PM;

  PM.add(new lt::StaticCallCounter());
  PM.add(new StaticCountPrinter(outs()));

  PM.run(M);
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//
int main(int Argc, char **Argv) {
  cl::HideUnrelatedOptions(CallCounterCategory);
  cl::ParseCommandLineOptions(Argc, Argv,
                              "Basic command line static analysis tool\n\n"
                              "This program will count the number of function "
                              "calls in the input IR file\n");
  // Makes shure llvm_shutdown() is called:
  //  http://llvm.org/docs/ProgrammersManual.html#ending-execution-with-llvm-shutdown
  llvm_shutdown_obj SDO;

  // Parse the IR file passed on the command line.
  SMDiagnostic Err;
  LLVMContext Ctx;
  unique_ptr<Module> M = parseIRFile(InputModule.getValue(), Err, Ctx);

  if (!M) {
    errs() << "Error reading bitcode file: " << InputModule << "\n";
    Err.print(Argv[0], errs());
    return -1;
  }

  switch (AnalysisTy) {
  case AnalysisType::DYNAMIC:
    countDynamicCalls(*M);
    break;
  case AnalysisType::STATIC:
    countStaticCalls(*M);
    break;
  default:
    report_fatal_error("Invalid analysis type.\n");
    return -1;
  }

  return 0;
}
