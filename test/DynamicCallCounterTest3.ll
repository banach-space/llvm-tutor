; RUN:  opt -load-pass-plugin %shlibdir/libDynamicCallCounter%shlibext -passes="dynamic-cc,verify"  -S %s | FileCheck %s

declare void @foo()

; CHECK-NOT: @CounterFor_foo
; CHECK-NOT: @ResultFormatStrIR = global [14 x i8]
; CHECK-NOT: @ResultHeaderStrIR = global [225 x i8]

; CHECK: declare void @foo()

; CHECK-NOT: @printf_wrapper
; CHECK-NOT: @printf
