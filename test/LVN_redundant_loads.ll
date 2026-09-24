; RUN: opt -load-pass-plugin=%shlibdir/libLVN%shlibext -passes="lvn" -S %s \
; RUN:   | FileCheck %s


; Verify that Redundant Load Elimination is taking place
; here %8 and %9 are Redundant Loads since their values are already loaded in %5 and %6
; hence it checks that %8 and %9 are not loads


; CHECK-LABEL: define dso_local i32 @test_redundant_loads(i32 noundef %0, i32 noundef %1)
; CHECK-NOT:   %8 = load
; CHECK-NOT:   %9 = load

; Function Attrs: noinline nounwind optnone sspstrong uwtable
define dso_local i32 @test_redundant_loads(i32 noundef %0, i32 noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 %0, ptr %3, align 4
  store i32 %1, ptr %4, align 4
  %5 = load i32, ptr %3, align 4
  %6 = load i32, ptr %4, align 4
  %7 = add nsw i32 %5, %6
  %8 = load i32, ptr %3, align 4
  %9 = load i32, ptr %4, align 4
  %10 = add nsw i32 %8, %9
  %11 = mul nsw i32 %7, %10
  ret i32 %11
}



