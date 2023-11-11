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
; CHECK-NEXT:  lt-if-then-else-0:
; CHECK-NEXT:  %1 = icmp eq i32 %0, 0
; CHECK-NEXT:  br i1 %1, label %lt-clone-1-0, label %lt-clone-2-0

; CHECK-LABEL:lt-clone-1-0:
; CHECK-NEXT:  br label %lt-tail-0

; CHECK-LABEL:lt-clone-2-0:
; CHECK-NEXT:  br label %lt-tail-0

; CHECK-LABEL:lt-tail-0:
; CHECK-NEXT:  switch i32 %0, label %lt-if-then-else-2 [
; CHECK-NEXT:    i32 1, label %lt-if-then-else-1
; CHECK-NEXT:  ]

; <label>:2:
; CHECK-LABEL: lt-if-then-else-1:
; CHECK-NEXT:  %2 = icmp eq i32 %0, 0
; CHECK-NEXT:  br i1 %2, label %lt-clone-1-1, label %lt-clone-2-1

; CHECK-LABEL:lt-clone-1-1:
; CHECK-NEXT:  br label %lt-tail-1

; CHECK-LABEL:lt-clone-2-1:
; CHECK-NEXT:  br label %lt-tail-1

; CHECK-LABEL:lt-tail-1:
; CHECK-NEXT:  br label %lt-if-then-else-3

; <label>:3:
; CHECK-LABEL: lt-if-then-else-2:
; CHECK-NEXT:  %3 = icmp eq i32 %0, 0
; CHECK-NEXT:  br i1 %3, label %lt-clone-1-2, label %lt-clone-2-2

; CHECK-LABEL:lt-clone-1-2:
; CHECK-NEXT:  br label %lt-tail-2

; CHECK-LABEL:lt-clone-2-2:
; CHECK-NEXT:  br label %lt-tail-2

; CHECK-LABEL:lt-tail-2:
; CHECK-NEXT:  br label %lt-if-then-else-3

; <label>:4:
; CHECK-LABEL: lt-if-then-else-3:
; CHECK-NEXT:  %4 = phi i32 [ 20, %lt-tail-2 ], [ 10, %lt-tail-1 ]
; CHECK-NEXT:  %5 = icmp eq i32 %0, 0
; CHECK-NEXT:  br i1 %5, label %lt-clone-1-3, label %lt-clone-2-3

; CHECK-LABEL:lt-clone-1-3:
; CHECK-NEXT:  br label %lt-tail-3

; CHECK-LABEL:lt-clone-2-3:
; CHECK-NEXT:  br label %lt-tail-3

; CHECK-LABEL:lt-tail-3:
; CHECK-NEXT:  ret i32 %4
