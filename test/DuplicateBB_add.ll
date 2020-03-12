; RUN: opt -load %shlibdir/libRIV%shlibext -load %shlibdir/libDuplicateBB%shlibext -legacy-duplicate-bb -S %s | FileCheck  %s
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
; CHECK-NEXT:  %5 = icmp eq i32 %{{[0-3]}}, 0
; CHECK-NEXT:  br i1 %5, label %lt-if-then-0, label %lt-else-0

; CHECK-LABEL: lt-if-then-0:
; CHECK-NEXT:  %6 = add i32 %1, %0
; CHECK-NEXT:  %7 = add i32 %6, %2
; CHECK-NEXT:  %8 = add i32 %7, %3
; CHECK-NEXT:  br label %12

; CHECK-LABEL: lt-else-0:
; CHECK-NEXT:  %9 = add i32 %1, %0
; CHECK-NEXT:  %10 = add i32 %9, %2
; CHECK-NEXT:  %11 = add i32 %10, %3
; CHECK-NEXT:  br label %12

; CHECK-LABEL: 12:
; CHECK-NEXT:  %13 = phi i32 [ %6, %lt-if-then-0 ], [ %9, %lt-else-0 ]
; CHECK-NEXT:  %14 = phi i32 [ %7, %lt-if-then-0 ], [ %10, %lt-else-0 ]
; CHECK-NEXT:  %15 = phi i32 [ %8, %lt-if-then-0 ], [ %11, %lt-else-0 ]
; CHECK-NEXT:  ret i32 %15
