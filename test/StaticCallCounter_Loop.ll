; RUN:  opt -load-pass-plugin %shlibdir/libStaticCallCounter%shlibext -passes="print<static-cc>" -disable-output %s 2>&1 | FileCheck %s

; Check that a call within a loop is counted only once. Note that the loop will
; iterate 10 times (i.e. `bar` will be called 10 times at run-time).

; CHECK: -------------------------------------------------
; CHECK-NEXT: bar                  1
; CHECK-NEXT: -------------------------------------------------

define i32 @foo(i32) {
  br label %2

; <label>:2:
  %3 = phi i32 [ 0, %1 ], [ %7, %2 ]
  %4 = phi i32 [ %0, %1 ], [ %6, %2 ]
  %5 = tail call i32 @bar(i32 %4)
  %6 = add nsw i32 %5, %4
  %7 = add nuw nsw i32 %3, 1
  %8 = icmp eq i32 %7, 10
  br i1 %8, label %9, label %2

; <label>:9:
  ret i32 %6
}

declare i32 @bar(i32)
