; RUN: opt -load-pass-plugin=%shlibdir/libLVN%shlibext -passes="lvn" -S %s \
; RUN:   | FileCheck %s
; CHECK-LABEL: define dso_local i32 @test_constant()
; CHECK:         ret i32 900

; Function Attrs: noinline nounwind optnone sspstrong uwtable
define dso_local i32 @test_constant() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store i32 10, ptr %1, align 4
  store i32 20, ptr %2, align 4
  %4 = load i32, ptr %1, align 4
  %5 = load i32, ptr %2, align 4
  %6 = add nsw i32 %4, %5
  store i32 %6, ptr %3, align 4
  %7 = load i32, ptr %3, align 4
  %8 = load i32, ptr %3, align 4
  %9 = mul nsw i32 %7, %8
  ret i32 %9
}


