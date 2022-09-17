; RUN:  opt -load-pass-plugin %shlibdir/libStaticCallCounter%shlibext -passes="print<static-cc>" -disable-output %s 2>&1 | FileCheck %s

; Makes sure that calls via function pointers are not analysed

; CHECK-NOT: foo
; CHECK-NOT: bar

define i32 @foo(i32) {
  ret i32 %0
}

define i32 @bar(i32) {
  %2 = alloca i32 (i32)*, align 8
  store i32 (i32)* @foo, i32 (i32)** %2, align 8
  %3 = load i32 (i32)*, i32 (i32)** %2, align 8
  %4 = call i32 %3(i32 %0)
  ret i32 %4
}
