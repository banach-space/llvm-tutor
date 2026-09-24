; RUN: opt -load-pass-plugin=%shlibdir/libLVN%shlibext -passes="lvn" -S %s \
; RUN:   | FileCheck %s
; CHECK-LABEL: define dso_local i32 @test_constant(i32 noundef %0)
; CHECK-NOT:   %10 = mul
; CHECK-NOT:   %11 = mul
; Function Attrs: noinline nounwind optnone sspstrong uwtable
define dso_local i32 @test_constant(i32 noundef %0) #0 {
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
  %6 = load i32, ptr %2, align 4
  %7 = load i32, ptr %2, align 4
  %8 = mul nsw i32 %6, %7
  store i32 %8, ptr %3, align 4
  %9 = load i32, ptr %2, align 4
  %10 = load i32, ptr %2, align 4
  %11 = mul nsw i32 %9, %10
  store i32 %11, ptr %4, align 4
  %12 = load i32, ptr %4, align 4
  %13 = load i32, ptr %3, align 4
  %14 = add nsw i32 %12, %13
  store i32 %14, ptr %5, align 4
  %15 = load i32, ptr %5, align 4
  ret i32 %15
}

