//=============================================================================
// FILE:
//      input_for_mba.c
//
// DESCRIPTION:
//      Sample input file for the MBA pass.
//
// License: MIT
//=============================================================================
#include <stdlib.h>

int main(int argc, char *argv[]) {
  int a = atoi(argv[1]), b = atoi(argv[2]), c = atoi(argv[3]),
      d = atoi(argv[4]);

  int e = a + b;
  int f = c + d;
  return e + f;
}
