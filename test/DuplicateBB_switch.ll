; RUN: opt -load %shlibdir/libRIV%shlibext -load %shlibdir/libDuplicateBB%shlibext -legacy-duplicate-bb -S %s | FileCheck  %s
; RUN: opt -load-pass-plugin %shlibdir/libRIV%shlibext -load-pass-plugin %shlibdir/libDuplicateBB%shlibext -passes=duplicate-bb -S %s | FileCheck  %s

; Below is a rather simple function with one switch statement. This test
; verifies that the output from DuplicateBB is correct. As expected, for every
; BasicBlock 4 new BasicBlocks are created. The switch instruction is not
; duplicated - it's a terminator instruction, so that's expected.

define i32 @foo(i32) {
  switch i32 %0, label %3 [
    i32 1, label %2
  ]

; <label>:2:
  br label %4

; <label>:3:
  br label %4

; <label>:4:
  %5 = phi i32 [ 20, %3 ], [ 10, %2 ]
  ret i32 %5
}

; CHECK-LABEL: foo
; Entry block
; CHECK-NEXT:  %2 = icmp eq i32 %0, 0
; CHECK-NEXT:  br i1 %2, label %lt-if-then-0, label %lt-else-0

; CHECK-LABEL:lt-if-then-0:
; CHECK-NEXT:  br label %3

; CHECK-LABEL:lt-else-0:
; CHECK-NEXT:  br label %3

; CHECK-LABEL:3:
; CHECK-NEXT:  switch i32 %0, label %7 [
; CHECK-NEXT:    i32 1, label %4
; CHECK-NEXT:  ]

; <label>:2:
; CHECK-LABEL:4:
; CHECK-NEXT:  %5 = icmp eq i32 %0, 0
; CHECK-NEXT:  br i1 %5, label %lt-if-then-1, label %lt-else-1

; CHECK-LABEL:lt-if-then-1:
; CHECK-NEXT:  br label %6

; CHECK-LABEL:lt-else-1:
; CHECK-NEXT:  br label %6

; CHECK-LABEL:6:
; CHECK-NEXT:  br label %10

; <label>:3:
; CHECK-LABEL:7:
; CHECK-NEXT:  %8 = icmp eq i32 %0, 0
; CHECK-NEXT:  br i1 %8, label %lt-if-then-2, label %lt-else-2

; CHECK-LABEL:lt-if-then-2:
; CHECK-NEXT:  br label %9

; CHECK-LABEL:lt-else-2:
; CHECK-NEXT:  br label %9

; CHECK-LABEL:9:
; CHECK-NEXT:  br label %10

; <label>:4:
; CHECK-LABEL:10:
; CHECK-NEXT:  %11 = phi i32 [ 20, %9 ], [ 10, %6 ]
; CHECK-NEXT:  %12 = icmp eq i32 %0, 0
; CHECK-NEXT:  br i1 %12, label %lt-if-then-3, label %lt-else-3

; CHECK-LABEL:lt-if-then-3:
; CHECK-NEXT:  br label %13

; CHECK-LABEL:lt-else-3:
; CHECK-NEXT:  br label %13

; CHECK-LABEL:13:
; CHECK-NEXT:  ret i32 %11
