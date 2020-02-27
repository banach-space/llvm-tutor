//=============================================================================
// FILE:
//      input_for_riv.c
//
// DESCRIPTION:
//      Sample input file for RIV analysis.
//
// License: MIT
//=============================================================================
int foo(int a, int b, int c) {
  int result = 123 + a;

  if (a > 0) {
    int d = a * b;
    int e = b / c;
    if (d == e) {
      int f = d * e;
      result = result - 2*f;
    } else {
      int g = 987;
      result = g * c * e;
    }
  } else {
    result = 321;
  }

  return result;
}
