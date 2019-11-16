//=============================================================================
// FILE:
//      input_for_inject.c
//
// DESCRIPTION:
//      Sample input file for InjectFuncCall
//
// License: MIT
//=============================================================================
#include <stdlib.h>

int foo(int a) {
  return a * 2;
}

int bar(int a, int b) {
  return (a + foo(b) * 2);
}

int fez(int a, int b, int c) {
  return (a + bar(a, b) * 2 + c * 3);
}

int main(int argc, char *argv[]) {
  int a = atoi(argv[1]);
  int ret = 0;

  ret += foo(a);
  ret += bar(a, ret);
  ret += fez(a, ret, 123);

  return ret;
}
