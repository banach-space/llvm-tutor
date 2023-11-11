; RUN: opt -load-pass-plugin %shlibdir/libMergeBB%shlibext -passes=merge-bb -S %s | FileCheck  %s

; %3 and %5 shouldn't be merged:
;   * %0 != %4, and
;   * The value for phi when coming from %5 is not defined in %5.

define i32 @foo(i32) {
  %2 = icmp eq i32 %0, 19
  br i1 %2, label %3, label %5

; <label>:3:
  %4 = add i32 %0,  13
  br label %7

; <label>:5:
  %6 = add i32 %0,  13
  br label %7

; <label>:7:
  %8 = phi i32 [ %4, %3 ], [ %0, %5 ]
  ret i32 %8
}

; CHECK-LABEL: foo
; CHECK-NEXT:   %2 = icmp eq i32 %0, 19
; CHECK-NEXT:   br i1 %2, label %3, label %5

; CHECK-LABEL: 3:
; CHECK-NEXT:   %4 = add i32 %0, 13
; CHECK-NEXT:   br label %7

; CHECK-LABEL: 5:
; CHECK-NEXT:   %6 = add i32 %0, 13
; CHECK-NEXT:   br label %7

; CHECK-LABEL: 7:
; CHECK-NEXT:   %8 = phi i32 [ %4, %3 ], [ %0, %5 ]
; CHECK-NEXT:   ret i32 %8
