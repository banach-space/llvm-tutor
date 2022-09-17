; RUN:  opt -load-pass-plugin %shlibdir/libStaticCallCounter%shlibext -passes="print<static-cc>" -disable-output %s 2>&1 | FileCheck %s

; Makes sure that nested function calls are analysed correctly. Note that only
; compile-time calls are reported. At run-time foo is called 3 times, bar 2
; times and fez 1 time.

; CHECK:-------------------------------------------------
; CHECK-NEXT: foo                  2
; CHECK-NEXT: bar                  2
; CHECK-NEXT: fez                  1
; CHECK-NEXT: -------------------------------------------------

define i32 @bar(i32) {
  %2 = tail call i32 @foo(i32 %0)
  ret i32 %2
}

declare i32 @foo(i32)

define i32 @fez(i32) {
  %2 = tail call i32 @bar(i32 %0)
  ret i32 %2
}

define i32 @far(i32) {
  %2 = tail call i32 @foo(i32 %0)
  %3 = tail call i32 @bar(i32 %0)
  %4 = add nsw i32 %3, %2
  %5 = tail call i32 @fez(i32 %0)
  %6 = add nsw i32 %4, %5
  ret i32 %6
}
