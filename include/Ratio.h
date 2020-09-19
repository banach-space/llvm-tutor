//========================================================================
// FILE:
//    Ratio.h
//
// DESCRIPTION:
//   Declares:
//    * class Ratio to hold a [0., 1.] ratio
//    * llvm::cl::parser specialisation for the Ratio type. See:
//      http://llvm.org/docs/CommandLine.html#extending-the-library
//    The two items are added to support custom command line argumets for
//    code obfuscation passes.
//
// License: MIT
//========================================================================
#ifndef LLVM_TUTOR_UTILS_H
#define LLVM_TUTOR_UTILS_H

#include "llvm/Support/CommandLine.h"

// A ratio in [0., 1.] interval
class Ratio {
  double Value = 0.0;

public:
  Ratio() = default;
  Ratio(double Value) : Value(Value) {}
  double getRatio() const { return Value; }
  void setRatio(double InValue) { Value = InValue; }
};

// CL parser specialisation to parse Ratio
namespace llvm {
namespace cl {

template <> class parser<Ratio> : public basic_parser<Ratio> {
public:
  parser(Option &Opt) : basic_parser<Ratio>(Opt) {}
  virtual ~parser() = default;

  bool parse(Option &O, StringRef ArgName, const StringRef &Arg, Ratio &Val);

  void printOptionDiff(const Option &O, Ratio const &V, OptionValue<Ratio> D,
                       size_t GlobalWidth) const;
};
} // namespace cl
} // namespace llvm
#endif
