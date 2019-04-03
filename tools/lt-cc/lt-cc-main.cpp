//========================================================================
// FILE:
//    lt-cc-main.cpp
//
// AUTHOR:
//    banach-space@github
//
// DESCRIPTION:
//
//
// License: MIT
//========================================================================
#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/CodeGen/CommandFlags.inc"
#include "llvm/CodeGen/LinkAllAsmWriterComponents.h"
#include "llvm/CodeGen/LinkAllCodegenComponents.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Linker/Linker.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileUtilities.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Scalar.h"

#include <memory>
#include <string>

#include "DynamicCallCounter.h"
#include "StaticCallCounter.h"

#include "config.h"

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
static cl::OptionCategory callCounterCategory{"call counter options"};

static cl::opt<string> inPath{cl::Positional,
                              cl::desc{"<Module to analyze>"},
                              cl::value_desc{"bitcode filename"},
                              cl::init(""),
                              cl::Required,
                              cl::cat{callCounterCategory}};

static cl::opt<AnalysisType> analysisType{
    cl::desc{"Select analyis type:"},
    cl::values(clEnumValN(AnalysisType::STATIC, "static",
                          "Count static direct calls."),
               clEnumValN(AnalysisType::DYNAMIC, "dynamic",
                          "Count dynamic direct calls.")),
    cl::Required, cl::cat{callCounterCategory}};

static cl::opt<string> outFile{
    "o", cl::desc{"Filename of the instrumented program"},
    cl::value_desc{"filename"}, cl::init(""), cl::cat{callCounterCategory}};

enum OptLevel {
  O0 = 0,
  O1,
  O2,
  O3
};

cl::opt<OptLevel> optimizationLevel(cl::desc("Choose optimization level (default = 'O2'):"),
  cl::values(
    clEnumVal(O1        , "Enable trivial optimizations"),
    clEnumVal(O2        , "Enable default optimizations"),
    clEnumVal(O3        , "Enable expensive optimizations")),
  cl::init(O2), cl::cat{callCounterCategory});

static cl::list<string> libPaths{
    "L", cl::Prefix, cl::desc{"Specify a library search path"},
    cl::value_desc{"directory"}, cl::cat{callCounterCategory}};

static cl::list<string> libraries{
    "l", cl::Prefix, cl::desc{"Specify libraries to link against"},
    cl::value_desc{"library prefix"}, cl::cat{callCounterCategory}};

//===----------------------------------------------------------------------===//
// The DynamicCountPrinter pass - wrappers
//===----------------------------------------------------------------------===//
static void compile(Module &m, StringRef outputPath) {
  string err;

  // 1. Get the target
  Triple triple = Triple(m.getTargetTriple());
  Target const *target = TargetRegistry::lookupTarget(MArch, triple, err);
  if (nullptr == target) {
    report_fatal_error("Unable to find target:\n " + err);
  }

  // 2. Generate the options
  CodeGenOpt::Level level = CodeGenOpt::Default;
  switch (optimizationLevel) {
  default:
    report_fatal_error("Invalid optimization level.\n");
  // No fall through
  case O0:
    level = CodeGenOpt::None;
    break;
  case O1:
    level = CodeGenOpt::Less;
    break;
  case O2:
    level = CodeGenOpt::Default;
    break;
  case O3:
    level = CodeGenOpt::Aggressive;
    break;
  }

  string FeaturesStr;
  TargetOptions options = InitTargetOptionsFromCodeGenFlags();
  unique_ptr<TargetMachine> machine(target->createTargetMachine(
      triple.getTriple(), MCPU, FeaturesStr, options, getRelocModel(),
      /* CodeModel */ None, level));
  assert((nullptr != machine) && "Could not allocate target machine!");

  if (FloatABIForCalls != FloatABI::Default) {
    options.FloatABIType = FloatABIForCalls;
  }

  // 3. Generate the output file
  std::error_code errc;
  auto out =
      std::make_unique<ToolOutputFile>(outputPath, errc, sys::fs::F_None);
  if (nullptr == out) {
    report_fatal_error("Unable to create file:\n " + errc.message());
  }

  // 4. Build up and run all of the passes for the module - object file
  // generation
  legacy::PassManager pm;

  // Add target specific info and transforms
  TargetLibraryInfoImpl tlii(triple);
  pm.add(new TargetLibraryInfoWrapperPass(tlii));

  m.setDataLayout(machine->createDataLayout());

  { // Bound this scope
    raw_pwrite_stream *os(&out->os());

    FileType = TargetMachine::CGFT_ObjectFile;
    std::unique_ptr<buffer_ostream> bos;
    if (!out->os().supportsSeeking()) {
      bos = std::make_unique<buffer_ostream>(*os);
      os = bos.get();
    }

    // Ask the target to add backend passes as necessary.
    if (machine->addPassesToEmitFile(pm, *os, /*DWO file */ nullptr, FileType,
                                     /*DisableVerify */ true)) {
      report_fatal_error("target does not support generation "
                         "of this file type!\n");
    }
    pm.run(m);
  }

  // Keep the output binary if we've been successful to this point.
  out->keep();
}

static void link(StringRef objectFile, StringRef outputFile) {
  auto clang = findProgramByName("clang++");
  string opt("-O");
  opt += std::to_string(optimizationLevel);

  if (!clang) {
    report_fatal_error("Unable to find clang.");
  }
  vector<string> args{clang.get(), opt, "-o", outputFile, objectFile};

  for (auto &libPath : libPaths) {
    args.push_back("-L" + libPath);
  }

  for (auto &library : libraries) {
    args.push_back("-l" + library);
  }

  vector<StringRef> args_stref;
  args_stref.reserve(args.size() + 1);
  for (auto &arg : args) {
    args_stref.push_back(arg.c_str());
  }

  for (auto &arg : args_stref) {
    outs() << arg << " ";
  }
  outs() << "\n";

  string err;
  if (-1 == ExecuteAndWait(clang.get(), args_stref, None, {}, 0, 0, &err)) {
    report_fatal_error("Unable to link output file.");
  }
}

static void generateBinary(Module &m, StringRef outputFilename) {
  // 1. Compile
  // Compiling to native should allow things to keep working even when the
  // version of clang on the system and the version of LLVM used to compile
  // the tool don't quite match up.
  string objectFile = outputFilename.str() + ".o";
  compile(m, objectFile);


  // 2. Link
  libPaths.push_back(LT_RT_LIBRARY_PATH);
  libraries.push_back(LT_RUNTIME_LIB);
  link(objectFile, outputFilename);
}

static void saveModule(const Module &m, StringRef filename) {
  std::error_code errc;
  raw_fd_ostream out(filename.data(), errc, sys::fs::F_None);

  if (errc) {
    report_fatal_error("error saving llvm module to '" + filename + "': \n" +
                       errc.message());
  }
  WriteBitcodeToFile(m, out);
}

static void countDynamicCalls(Module &m) {
  // 1. Quickly verify the input
  if (outFile.getValue().empty()) {
    errs() << "-o command line option must be specified.\n";
    exit(-1);
  }

  // 2. Initialize all the targets for emitting object code.
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmPrinters();
  InitializeAllAsmParsers();
  // cl::AddExtraVersionPrinter(TargetRegistry::printRegisteredTargetsForVersion);

  // 3. Build up and run all of the passes that we want for the input module.
  legacy::PassManager pm;
  pm.add(new lt::DynamicCallCounter());
  pm.add(createVerifierPass());

  // Before executing passes, print the final values of the LLVM options.
  cl::PrintOptionValues();

  pm.run(m);

  // 4. Save the instrumented module and generate a binary from it
  saveModule(m, outFile + ".lt.bc");
  generateBinary(m, outFile);
 }

//===----------------------------------------------------------------------===//
// The StaticCountPrinter pass - wrappers
//===----------------------------------------------------------------------===//
struct StaticCountPrinter : public ModulePass {
  static char ID;
  raw_ostream &out;

  explicit StaticCountPrinter(raw_ostream &out) : ModulePass(ID), out(out) {}

  bool runOnModule(Module &m) override {
    // Prints the result of StaticCallCounter
    getAnalysis<lt::StaticCallCounter>().print(out, &m);
    return false;
  }

  // Set dependencies
  void getAnalysisUsage(AnalysisUsage &au) const override {
    au.addRequired<lt::StaticCallCounter>();
    au.setPreservesAll();
  }
};

char StaticCountPrinter::ID = 0;

static void countStaticCalls(Module &m) {
  // Build up all of the passes that we want to run on the module.
  legacy::PassManager pm;

  pm.add(new lt::StaticCallCounter());
  pm.add(new StaticCountPrinter(outs()));

  pm.run(m);
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//
int main(int argc, char **argv) {
  cl::HideUnrelatedOptions(callCounterCategory);
  cl::ParseCommandLineOptions(argc, argv,
                              "Basic command line static analysis tool\n\n"
                              "This program will count the number of function "
                              "calls in the input IR file\n");

  // This boilerplate provides convenient stack traces and clean LLVM exit
  // handling.
  sys::PrintStackTraceOnErrorSignal(argv[0]);
  llvm::PrettyStackTraceProgram X(argc, argv);
  llvm_shutdown_obj shutdown;

  // Parse the IR file passed on the command line.
  SMDiagnostic err;
  LLVMContext context;
  unique_ptr<Module> module = parseIRFile(inPath.getValue(), err, context);

  if (!module) {
    errs() << "Error reading bitcode file: " << inPath << "\n";
    err.print(argv[0], errs());
    return -1;
  }

  // Run either the static or dynamic analysis
  switch (analysisType) {
    case AnalysisType::DYNAMIC:
      countDynamicCalls(*module);
      break;
    case AnalysisType::STATIC:
      countStaticCalls(*module);
      break;
    default:
      report_fatal_error("Invalid analysis type.\n");
      return -1;
  }

  return 0;
}
