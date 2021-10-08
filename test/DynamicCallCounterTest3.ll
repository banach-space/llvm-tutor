; RUN:  opt --enable-new-pm=0 -load %shlibdir/libDynamicCallCounter%shlibext -legacy-dynamic-cc -verify  -S %s | FileCheck %s

declare void @foo()

; CHECK-NOT: @CounterFor_foo
; CHECK-NOT: @ResultFormatStrIR = global [14 x i8]
; CHECK-NOT: @ResultHeaderStrIR = global [225 x i8]

; CHECK: declare void @foo()

; CHECK-NOT: @printf_wrapper
; CHECK-NOT: @printf
