; RUN: opt -load ../lib/libRIV%shlibext -load ../lib/libDuplicateBB%shlibext -duplicate-bb -duplicate-bb-ratio=1 -S %s  | FileCheck -check-prefix=DUPL %s
; RUN: opt -load ../lib/libRIV%shlibext -load ../lib/libDuplicateBB%shlibext -duplicate-bb -duplicate-bb-ratio=0 -S %s  | FileCheck -check-prefix=NO_DUPL %s

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

; Verify that with duplicate-bb-ratio set to 0, there's no BB duplication
; NO_DUPL: define i32 @foo(i32) {
; NO_DUPL:   %2 = alloca i32, align 4
; NO_DUPL:   %3 = alloca i32, align 4
; NO_DUPL:   store i32 %0, i32* %2, align 4
; NO_DUPL:   %4 = load i32, i32* %2, align 4
; NO_DUPL:   %5 = mul nsw i32 %4, 4
; NO_DUPL:   store i32 %5, i32* %3, align 4
; NO_DUPL:   %6 = load i32, i32* %3, align 4
; NO_DUPL:   ret i32 %6
; NO_DUPL: }

; Verify that with duplicate-bb-ratio set to 1, all BBs are duplicated
; DUPL: define i32 @foo(i32) {
; DUPL:   %2 = icmp eq i32 %0, 0
; DUPL:   br i1 %2, label %3, label %9
; DUPL: 3:                                      ; preds = %1
; DUPL:   %4 = alloca i32, align 4
; DUPL:   %5 = alloca i32, align 4
; DUPL:   store i32 %0, i32* %4, align 4
; DUPL:   %6 = load i32, i32* %4, align 4
; DUPL:   %7 = mul nsw i32 %6, 4
; DUPL:   store i32 %7, i32* %5, align 4
; DUPL:   %8 = load i32, i32* %5, align 4
; DUPL:   br label %15
; DUPL: 9:                                      ; preds = %1
; DUPL:   %10 = alloca i32, align 4
; DUPL:   %11 = alloca i32, align 4
; DUPL:   store i32 %0, i32* %10, align 4
; DUPL:   %12 = load i32, i32* %10, align 4
; DUPL:   %13 = mul nsw i32 %12, 4
; DUPL:   store i32 %13, i32* %11, align 4
; DUPL:   %14 = load i32, i32* %11, align 4
; DUPL:   br label %15
; DUPL: 15:                                     ; preds = %9, %3
; DUPL:   %16 = phi i32* [ %4, %3 ], [ %10, %9 ]
; DUPL:   %17 = phi i32* [ %5, %3 ], [ %11, %9 ]
; DUPL:   %18 = phi i32 [ %6, %3 ], [ %12, %9 ]
; DUPL:   %19 = phi i32 [ %7, %3 ], [ %13, %9 ]
; DUPL:   %20 = phi i32 [ %8, %3 ], [ %14, %9 ]
; DUPL:   ret i32 %20
; DUPL: }
