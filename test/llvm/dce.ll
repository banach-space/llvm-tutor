; RUN: opt -S --passes=dce %s | FileCheck %s

;------------------------------------------------------------------------------
; CASE 1: A loop without functions calls - hoisting successful
;------------------------------------------------------------------------------
; Verify that all instructions except for the return statement are eliminated
; (they are `dead`)
define signext i8 @foo(i8 signext, i8 signext, i8 signext, i8 signext) {
; CHECK-LABEL: foo
; CHECK-NEXT: ret i8 123
  %5 = add i8 %1, %0
  %6 = add i8 %5, %2
  %7 = add i8 %6, %3

  ret i8 123
}

;------------------------------------------------------------------------------
; CASE 2: A loop without functions calls - hoisting successful
;------------------------------------------------------------------------------
; Verify that all instructions except for the return statement are eliminated
; (they are `dead`)
; Generated from dce_basic_add.ll with:
;     opt --load-pass-plugin lib/libMBAAdd.dylib --passes=mba-add -S dce_basic_add.ll
define signext i8 @foo_v2(i8 signext %0, i8 signext %1, i8 signext %2, i8 signext %3) {
; CHECK-LABEL: foo_v2
; CHECK-NEXT: ret i8 123
  %5 = xor i8 %1, %0
  %6 = and i8 %1, %0
  %7 = mul i8 2, %6
  %8 = add i8 %5, %7
  %9 = mul i8 39, %8
  %10 = add i8 23, %9
  %11 = mul i8 -105, %10
  %12 = add i8 111, %11
  %13 = xor i8 %12, %2
  %14 = and i8 %12, %2
  %15 = mul i8 2, %14
  %16 = add i8 %13, %15
  %17 = mul i8 39, %16
  %18 = add i8 23, %17
  %19 = mul i8 -105, %18
  %20 = add i8 111, %19
  %21 = xor i8 %20, %3
  %22 = and i8 %20, %3
  %23 = mul i8 2, %22
  %24 = add i8 %21, %23
  %25 = mul i8 39, %24
  %26 = add i8 23, %25
  %27 = mul i8 -105, %26
  %28 = add i8 111, %27
  ret i8 123
}

;------------------------------------------------------------------------------
; CASE 3: A loop without functions calls - hoisting successful
;------------------------------------------------------------------------------
; Verify that all instructions except for the return statement are eliminated
; (they are `dead`)
declare i8* @strcat(i8*, i8*) readonly nounwind willreturn

define void @foo_v3() {
; CHECK-LABEL: foo_v3
; CHECK-NEXT: ret void
  call i8* @strcat(i8* null,  i8* null)
  ret void
}

;------------------------------------------------------------------------------
; CASE 4: A loop without functions calls - hoisting successful
;------------------------------------------------------------------------------
; Verify that a call to @llvm.sideeffect is _not_ elimnated. See here for an
; explanation: https://llvm.org/docs/LangRef.html#llvm-sideeffect-intrinsic
declare void @llvm.sideeffect()

define void @foo_v4() {
; CHECK-LABEL: foo_v4
; CHECK: call void @llvm.sideeffect()
    call void @llvm.sideeffect()
    ret void
}
