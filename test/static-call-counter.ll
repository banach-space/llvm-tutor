; RUN: clang -c -emit-llvm %S/example_1.c -o %t
; RUN: opt -load ../lib/liblt-cc-shared.so --lt -analyze %t | FileCheck %s

; CHECK: a                    2         
; CHECK: b                    2
; CHECK: printf               5         

; =====> THE SOURCE FILE FOR TESTING <======
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
