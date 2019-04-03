//========================================================================
// FILE:
//      example_1.c
//
// AUTHOR:
//      banach-space@github
//
// DESCRIPTION:
//      Basic example to test lt-cc with
//
// License: MIT
//========================================================================
#include <stdio.h>

void a() { printf("  Inside a ...\n"); }

void b() {
  printf("  Inside b ...\n");
  int ii = 0;
  for (ii = 0; ii < 10; ii++) {
    a();
  }
}

void c() { printf("  Inside c ...\n"); }

int main() {
  b();
  a();
  b();

  printf("Inside main ...\n");
  printf("Inside main ...\n");

  return 0;
}
