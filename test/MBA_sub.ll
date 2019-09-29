; RUN: %clang -S -emit-llvm %S/../inputs/input_for_mba_sub.c -o - \
; RUN:  | opt -load ../lib/libMBASub%shlibext -legacy-mba-sub -S \
; RUN:  | FileCheck %s
; RUN: %clang -S -emit-llvm %S/../inputs/input_for_mba_sub.c -o - \
; RUN:  | opt -load-pass-plugin=../lib/libMBASub%shlibext -passes="mba-sub" -S \
; RUN:  | FileCheck %s

; The input file contains 3 subtractions. Verify that these are correctly
; replaced in the output IR as follows:
;    a - b == (a + ~b) + 1

; 1st substitution
; CHECK:       [[REG_1:%[0-9]+]] = load i32, i32* {{%[0-9]+}}, align 4
; CHECK-NEXT:  [[REG_2:%[0-9]+]] = load i32, i32* {{%[0-9]+}}, align 4
; CHECK-NEXT:  [[REG_3:%[0-9]+]] = xor i32 [[REG_2]], -1
; CHECK-NEXT:  [[REG_4:%[0-9]+]] = add i32 [[REG_1]], [[REG_3]]
; CHECK-NEXT:  [[REG_5:%[0-9]+]] = add i32 [[REG_4]], 1
;
; 2nd substitution
; CHECK:       [[REG_6:%[0-9]+]] = load i32, i32* {{%[0-9]+}}, align 4
; CHECK-NEXT:  [[REG_7:%[0-9]+]] = load i32, i32* {{%[0-9]+}}, align 4
; CHECK-NEXT:  [[REG_8:%[0-9]+]] = xor i32 [[REG_7]], -1
; CHECK-NEXT:  [[REG_9:%[0-9]+]] = add i32 [[REG_6]], [[REG_8]]
; CHECK-NEXT:  [[REG_10:%[0-9]+]] = add i32 [[REG_9]], 1
;
; 3rd substitution
; CHECK:       [[REG_11:%[0-9]+]] = load i32, i32* {{%[0-9]+}}, align 4
; CHECK-NEXT:  [[REG_12:%[0-9]+]] = load i32, i32* {{%[0-9]+}}, align 4
; CHECK-NEXT:  [[REG_13:%[0-9]+]] = xor i32 [[REG_12]], -1
; CHECK-NEXT:  [[REG_14:%[0-9]+]] = add i32 [[REG_11]], [[REG_13]]
; CHECK-NEXT:  [[REG_15:%[0-9]+]] = add i32 [[REG_14]], 1
;
; Verify that there are no more substitutions (obfuscated or non-obfuscated)
; CHECK-NOT:   xor
; CHECK-NOT:   sub
; CHECK-NOT:   add
