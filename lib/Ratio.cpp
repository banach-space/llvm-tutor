//==============================================================================
// FILE:
//    Ratio.cpp
//
// DESCRIPTION:
//    Implementation of llvm::cl::parser specialisation for the Ratio type. See:
//    http://llvm.org/docs/CommandLine.html#extending-the-library
//
// License: MIT
//==============================================================================
#include "Ratio.h"

namespace llvm {
namespace cl {

// Check if the user input is a real in [0., 1.]
// Returns false on success.
bool parser<Ratio>::parse(Option &Opt, StringRef ArgName,
                          const std::string &Arg, Ratio &Val) {
  char const *ArgCStr = Arg.c_str();
  char *EndPtr = nullptr;
  double TheRatio = std::strtod(
      ArgCStr, &EndPtr); // cannot use std::stod: no exception support in LLVM

  if (EndPtr == ArgCStr) {
    return Opt.error(ArgName + " value `" + Arg +
                     "' is not a floating point value");
  } else if (0. > TheRatio or 1. < TheRatio) {
    return Opt.error("'" + Arg + "' is not in [0., 1.]");
  } else {
    Val.setRatio(TheRatio);
    return false;
  }
}

void parser<Ratio>::printOptionDiff(const Option &Opt, Ratio const &,
                                    OptionValue<Ratio>,
                                    size_t GlobalWidth) const {
  printOptionName(Opt, GlobalWidth);
}
} // namespace cl
} // namespace llvm
