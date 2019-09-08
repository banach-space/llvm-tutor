//=============================================================================
// FILE:
//      input_for_hello.c
//
// DESCRIPTION:
//      Sample input file for the HelloWorld pass.
//
// License: MIT
//=============================================================================
int foo(int a) { return a * 2; }

int bar(int a, int b) { return (a + b * 2); }
int fez(int a, int b, int c) { return (a + b * 2 + c * 3); }

int main(int argc, const char **argv) {
  int a = 1;
  int b = 11;
  int c = 111;

  return (foo(a) + bar(a, b) + fez(a, b, c));
}
