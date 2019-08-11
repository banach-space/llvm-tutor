//========================================================================
// FILE:
//      example_1.c
//
// AUTHOR:
//      banach-space@github
//
// DESCRIPTION:
//      A very basic example with a few functions calls and a few function
//      casts (some of which are UB). Note that currently none of the passes
//      handles function casts (i.e. the corresponding calls won't be counted).
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

int foo(int a) {
  printf("  Inside foo ... \n");
}

int bar(int a, int b) {
  printf("  Inside bar ... \n");
}

int main() {
  b();
  a();
  b();

  int (*g)(int a) = foo;
  int (*h)(int a, int b) = foo;

  g(5);
  h(5, 6);

  foo(10);

  printf("Inside main ...\n");
  printf("Inside main ...\n");

  return 0;
}
