; RUN: %clang -S -O1 -emit-llvm %S/../inputs/input_for_mba.c -o - \
; RUN:  | opt -load ../lib/libMBAAdd%shlibext -legacy-mba-add -S \
; RUN:  | FileCheck %s
; RUN: %clang -S -O1 -emit-llvm %S/../inputs/input_for_mba.c -o - \
; RUN:  | opt -load-pass-plugin=../lib/libMBAAdd%shlibext -passes="mba-add" -S \
; RUN:  | FileCheck %s

; The input file contains 3 additions. Verify that these are correctly
; replaced as follows:
;    a + b == (((a ^ b) + 2 * (a & b)) * 39 + 23) * 151 + 111

; CHECK-LABEL: @foo
; 1st substitution
; CHECK-DAG:   {{%[0-9]+}} = xor i8 {{%[0-9]+}}, {{%[0-9]+}}
; CHECK-DAG:   {{%[0-9]+}} = and i8 {{%[0-9]+}}, {{%[0-9]+}}
; CHECK-DAG:   {{%[0-9]+}} = mul i8 2, {{%[0-9]+}}
; CHECK-NOT:   [[REG_2:%[0-9]+]] = mul i8 [[REG_1]], 39
; CHECK-DAG:   [[REG_1:%[0-9]+]] = add i8 {{%[0-9]+}}, {{%[0-9]+}}
; CHECK-NEXT:  [[REG_2:%[0-9]+]] = mul i8 [[REG_1]], 39
; CHECK-NEXT:  [[REG_3:%[0-9]+]] = add i8 [[REG_2]], 23
; CHECK-NEXT:  [[REG_4:%[0-9]+]] = mul i8 [[REG_3]], -105
; CHECK-NEXT:  [[REG_5:%[0-9]+]] = add i8 [[REG_4]], 111
;
; 2nd addition
; CHECK-DAG:   {{%[0-9]+}} = xor i8 {{%[0-9]+}}, {{%[0-9]+}}
; CHECK-DAG:   {{%[0-9]+}} = and i8 {{%[0-9]+}}, {{%[0-9]+}}
; CHECK-DAG:   {{%[0-9]+}} = mul i8 2, {{%[0-9]+}}
; CHECK-NOT:   [[REG_2:%[0-9]+]] = mul i8 [[REG_1]], 39
; CHECK-DAG:   [[REG_1:%[0-9]+]] = add i8 {{%[0-9]+}}, {{%[0-9]+}}
; CHECK-NEXT:  [[REG_2:%[0-9]+]] = mul i8 [[REG_1]], 39
; CHECK-NEXT:  [[REG_3:%[0-9]+]] = add i8 [[REG_2]], 23
; CHECK-NEXT:  [[REG_4:%[0-9]+]] = mul i8 [[REG_3]], -105
; CHECK-NEXT:  [[REG_5:%[0-9]+]] = add i8 [[REG_4]], 111
;
; 3rd addition
; CHECK-DAG:   {{%[0-9]+}} = xor i8 {{%[0-9]+}}, {{%[0-9]+}}
; CHECK-DAG:   {{%[0-9]+}} = and i8 {{%[0-9]+}}, {{%[0-9]+}}
; CHECK-DAG:   {{%[0-9]+}} = mul i8 2, {{%[0-9]+}}
; CHECK-NOT:   [[REG_2:%[0-9]+]] = mul i8 [[REG_1]], 39
; CHECK-DAG:   [[REG_1:%[0-9]+]] = add i8 {{%[0-9]+}}, {{%[0-9]+}}
; CHECK-NEXT:  [[REG_2:%[0-9]+]] = mul i8 [[REG_1]], 39
; CHECK-NEXT:  [[REG_3:%[0-9]+]] = add i8 [[REG_2]], 23
; CHECK-NEXT:  [[REG_4:%[0-9]+]] = mul i8 [[REG_3]], -105
; CHECK-NEXT:  [[REG_5:%[0-9]+]] = add i8 [[REG_4]], 111
;
; Verify that there are no more additions (obfuscated or non-obfuscated)
; CHECK-NOT:   xor
; CHECK-NOT:   and
; CHECK-NOT:   mul
; CHECK-NOT:   add

; CHECK-LABEL: @main
