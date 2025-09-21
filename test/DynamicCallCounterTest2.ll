; RUN:  opt -load-pass-plugin %shlibdir/libDynamicCallCounter%shlibext -passes="dynamic-cc,verify"  -S %s | FileCheck %s

; Instrument this file with DynamicCallCounter and verify that the inserted code
; is correct.

; The global variables inserted by the pass
; CHECK: @CounterFor_foo = common global i32 0, align 4
; CHECK-NEXT: @0 = private unnamed_addr constant [4 x i8] c"foo\00", align 1
; CHECK-NEXT: @ResultFormatStrIR = global [14 x i8]
; CHECK-NEXT: @ResultHeaderStrIR = global [225 x i8]
; CHECK-NEXT: @llvm.global_dtors = appending global
; CHECK-SAME: @printf_wrapper

define void @foo() {
; CHECK-LABEL: @foo(
; Call-counting instructions inserted by the pass
; CHECK-NEXT:    [[TMP1:%.*]] = load i32, ptr @CounterFor_foo
; CHECK-NEXT:    [[TMP2:%.*]] = add i32 1, [[TMP1]]
; CHECK-NEXT:    store i32 [[TMP2]], ptr @CounterFor_foo
; CHECK-NEXT:    ret void
;
  ret void
}

; Declaration of `printf` inserted by the pass
; CHECK: declare i32 @printf(ptr readonly captures(none), ...) #0

; Definition of `printf_wrapper` inserted by the pass
; CHECK: define void @printf_wrapper() {
; CHECK-NEXT: enter:
; CHECK-NEXT:  %0 = call i32 (ptr, ...) @printf
; CHECK-SAME: @ResultHeaderStrIR
; CHECK-NEXT:  %1 = load i32, ptr @CounterFor_foo
; CHECK-NEXT:  %2 = call i32 (ptr, ...) @printf
; CHECK-SAME: @ResultFormatStrIR
; CHECK-NEXT:  ret void
; CHECK-NEXT: }
