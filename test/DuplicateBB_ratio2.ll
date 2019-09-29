; RUN: not opt -load ../lib/libRIV%shlibext -load ../lib/libDuplicateBB%shlibext -duplicate-bb -duplicate-bb-ratio=100 -S %s 2>&1 | FileCheck %s

;  Verify that wrong DuplicateBB ratio triggers an adequate error

; CHECK: opt: for the --duplicate-bb-ratio option: '100' is not in [0., 1.] 

define i32 @foo(i32) {
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  %4 = load i32, i32* %2, align 4
  %5 = mul nsw i32 %4, 4
  store i32 %5, i32* %3, align 4
  %6 = load i32, i32* %3, align 4
  ret i32 %6
}
