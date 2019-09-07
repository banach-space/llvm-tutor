//=============================================================================
// FILE:
//      input_for_rvi.c
//
// DESCRIPTION:
//      Sample input file for RIV analysis.
//
// License: MIT
//=============================================================================
int fez() {
  int foo_var1 = 1;
  int foo_var2 = 2;
  int foo_var3 = foo_var1 + foo_var2;

  if (foo_var1)
    foo_var2 = 3;

  return foo_var3;
}

int foo(int a) {
  int foo_var1 = 1;
  int foo_var2 = 2;
  int foo_var3 = foo_var1 + foo_var2;

  return foo_var3;
}

int bar() {
  int bar_var1 = foo(1);

  int ii = 0;
  for (ii = 0; ii < 10; ii++) {
    int bar_var2 = ii;
    bar_var1 += bar_var2;
  }

  return bar_var1;
}

int main(int argc, char *argv[]) { return bar(); }
