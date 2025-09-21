; RUN:  opt -load-pass-plugin=%shlibdir/libInjectFuncCall%shlibext -passes="inject-func-call,verify" -S %s\
; RUN:  | FileCheck %s

; Verify that InjectFuncCall indeed inserts calls to printf and global
; variables that contain functions names (these are needed for the calls to printf).

; The format string
; CHECK: @PrintfFormatStr = global [68 x i8] c"(llvm-tutor) Hello from: %s\0A(llvm-tutor)   number of arguments: %d\0A\00"
; The function names
; CHECK-NEXT: @0 = private unnamed_addr constant [4 x i8] c"foo\00", align 1
; CHECK-NEXT: @1 = private unnamed_addr constant [4 x i8] c"bar\00", align 1
; CHECK-NEXT: @2 = private unnamed_addr constant [4 x i8] c"baz\00", align 1
; CHECK-NEXT: @3 = private unnamed_addr constant [4 x i8] c"bez\00", align 1

; CHECK-LABEL: @foo
; CHECK-NEXT:  %2 = call i32 (ptr, ...) @printf

; CHECK-LABEL: @bar
; CHECK-NEXT:  %3 = call i32 (ptr, ...) @printf

; CHECK-LABEL: @baz
; CHECK-NEXT:  %4 = call i32 (ptr, ...) @printf

; CHECK-LABEL: @bez
; CHECK-NEXT: %2 = call i32 (ptr, ...) @printf

; CHECK: declare i32 @printf(ptr readonly captures(none), ...) #0

; CHECK: attributes #0 = { nounwind }

define i32 @foo(i32) {
  %2 = shl nsw i32 %0, 1
  ret i32 %2
}

define i32 @bar(i32, i32) {
  %3 = tail call i32 @foo(i32 %1)
  %4 = shl i32 %3, 1
  %5 = add nsw i32 %4, %0
  ret i32 %5
}

define i32 @baz(i32, i32, i32) {
  %4 = tail call i32 @bar(i32 %0, i32 %1)
  %5 = shl i32 %4, 1
  %6 = mul nsw i32 %2, 3
  %7 = add i32 %6, %0
  %8 = add i32 %7, %5
  ret i32 %8
}

define i32 @bez(i32) {
  %2 = tail call i32 @foo(i32 %0)
  %3 = tail call i32 @bar(i32 %0, i32 %2)
  %4 = add nsw i32 %3, %2
  %5 = tail call i32 @baz(i32 %0, i32 %4, i32 123)
  %6 = add nsw i32 %4, %5
  ret i32 %6
}
