//=============================================================================
// FILE:
//      input_for_fcmp_eq.c
//
// DESCRIPTION:
//      Sample input file for the FindFCmpEq and ConvertFCmpEq passes.
//
// License: MIT
//=============================================================================

// sqrt_impl uses the Newton-Raphson method for approximating square roots.
double sqrt_impl(double x, double hi, double lo) {
  // First direct floating-point equality comparison
  if (hi == lo) {
    return lo;
  }

  double midpoint = (lo + hi + 1.0) / 2.0;
  if (x / midpoint < midpoint) {
    return sqrt_impl(x, lo, midpoint - 1.0);
  }

  return sqrt_impl(x, midpoint, hi);
}

double sqrt(double x) { return sqrt_impl(x, 0, x / 2.0 + 1.0); }

int main() {
  double a = 0.2;
  double b = 1.0 / sqrt(5.0) / sqrt(5.0);
  // Second direct floating-point equality comparison
  if (b == 1.0)
    return a == b ? 1 : 0;
  else
    return a == b ? 0 : 1;
}
