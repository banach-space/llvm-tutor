; RUN: opt -load-pass-plugin=%shlibdir/libMBAAdd%shlibext -passes="mba-add" -S %s \
; RUN:  | FileCheck %s

define signext i32 @foo(i32 signext, i32 signext, i32 signext, i32 signext) {
  %5 = add i32 %1, %0
  %6 = add i32 %5, %2
  %7 = add i32 %6, %3
  ret i32 %7
}

; Verify that the additions in foo are _not_ substituted (because MBAAdd
; is only meant for 8-bit additions)

; CHECK-LABEL: @foo
; CHECK-NEXT:  %5 = add i32 %1, %0
; CHECK-NEXT:  %6 = add i32 %5, %2
; CHECK-NEXT:  %7 = add i32 %6, %3
; CHECK-NEXT:  ret i32 %7
