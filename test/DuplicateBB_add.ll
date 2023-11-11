; RUN: opt -load-pass-plugin %shlibdir/libRIV%shlibext -load-pass-plugin %shlibdir/libDuplicateBB%shlibext -passes=duplicate-bb -S %s | FileCheck  %s

; Verify that the output from DuplicateBB is correct, i.e.
;   * every addition was duplicated
;   * the required PHI nodes to merge the values were created.
; This test is a very illustrative example of how DuplicateBB modifies the
; input code in the presence of meaningful instructions (as opposed to empty
; BasicBlocks).

define i32 @foo(i32, i32, i32, i32) {
  %5 = add i32 %1, %0
  %6 = add i32 %5, %2
  %7 = add i32 %6, %3
  ret i32 %7
}

; CHECK-LABEL: @foo
; CHECK-NEXT:  lt-if-then-else-0:
; CHECK-NEXT:  %4 = icmp eq i32 %{{[0-3]}}, 0
; CHECK-NEXT:  br i1 %4, label %lt-clone-1-0, label %lt-clone-2-0

; CHECK-LABEL: lt-clone-1-0:
; CHECK-NEXT:  %5 = add i32 %1, %0
; CHECK-NEXT:  %6 = add i32 %5, %2
; CHECK-NEXT:  %7 = add i32 %6, %3
; CHECK-NEXT:  br label %lt-tail-0

; CHECK-LABEL: lt-clone-2-0:
; CHECK-NEXT:  %8 = add i32 %1, %0
; CHECK-NEXT:  %9 = add i32 %8, %2
; CHECK-NEXT:  %10 = add i32 %9, %3
; CHECK-NEXT:  br label %lt-tail-0

; CHECK-LABEL: lt-tail-0:
; CHECK-NEXT:  %11 = phi i32 [ %5, %lt-clone-1-0 ], [ %8, %lt-clone-2-0 ]
; CHECK-NEXT:  %12 = phi i32 [ %6, %lt-clone-1-0 ], [ %9, %lt-clone-2-0 ]
; CHECK-NEXT:  %13 = phi i32 [ %7, %lt-clone-1-0 ], [ %10, %lt-clone-2-0 ]
; CHECK-NEXT:  ret i32 %13
