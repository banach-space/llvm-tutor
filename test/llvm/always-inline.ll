; RUN: opt < %s -passes=always-inline -S | FileCheck %s --check-prefix=CHECK

;------------------------------------------------------------------------------
; CASE 1: most obvious scenario for inlining
;------------------------------------------------------------------------------
define internal i32 @inner1() alwaysinline {
; CHECK-NOT: @inner1(
  ret i32 1
}

define i32 @outer1() {
; CHECK-LABEL: @outer1(
; CHECK-NOT: call
; CHECK: ret

   %r = call i32 @inner1()
   ret i32 %r
}

;------------------------------------------------------------------------------
; CASE 2: Don't inline functions that would introduce a returns_twice call
;------------------------------------------------------------------------------
declare i32 @a() returns_twice

define internal i32 @inner2() alwaysinline {
; CHECK-LABEL: @inner2(
entry:
  %call = call i32 @a() returns_twice
  %add = add nsw i32 1, %call
  ret i32 %add
}

define i32 @outer2() {
entry:
; CHECK-LABEL: @outer2(
; CHECK-NOT: call i32 @a
; CHECK: ret

  %call = call i32 @inner2()
  %add = add nsw i32 1, %call
  ret i32 %add
}

;------------------------------------------------------------------------------
; CASE 3: Do inline functions that would introduce a returns_twice call in
;         functions already have a returns_twice attribute
;------------------------------------------------------------------------------
declare i32 @b() returns_twice

define internal i32 @inner3() alwaysinline returns_twice {
; CHECK-NOT: @inner3(
entry:
  %call = call i32 @b() returns_twice
  %add = add nsw i32 1, %call
  ret i32 %add
}

define i32 @outer3() {
entry:
; CHECK-LABEL: @outer3(
; CHECK: call i32 @b()
; CHECK: ret

  %call = call i32 @inner3() returns_twice
  %add = add nsw i32 1, %call
  ret i32 %add
}

;------------------------------------------------------------------------------
; CASE 4: Recursive functions are not inlined
;------------------------------------------------------------------------------
define i32 @fibonacci(i32) {
; CHECK-LABEL: @fibonacci(
  %2 = icmp slt i32 %0, 2
  br i1 %2, label %9, label %3

; <label>:3:                                      ; preds = %1
  %4 = add nsw i32 %0, -1
  %5 = tail call i32 @fibonacci(i32 %4)
  %6 = add nsw i32 %0, -2
  %7 = tail call i32 @fibonacci(i32 %6)
  %8 = add nsw i32 %7, %5
  ret i32 %8

; <label>:9:                                      ; preds = %1
  ret i32 %0
}

define void @outer4() {
; CHECK-LABEL: @outer4(
; CHECK: call i32 @fibonacci(i32 42)
; CHECK: ret

entry:
  call i32 @fibonacci(i32 42)
  ret void
}

;------------------------------------------------------------------------------
; CASE 5: `inner6` is inlined, but not deleted. 
;------------------------------------------------------------------------------
; The linkage type (https://llvm.org/docs/LangRef.html#linkage-types) of
; `inner6` is not `internal`. Instead, `external` is assumed (i.e. the default),
; which indicates that this function could be used elsewhere. Hence it cannot be
; deleted.
;------------------------------------------------------------------------------
define void @inner6() alwaysinline {
; CHECK-LABEL: @inner6(
entry:
  ret void
}

define void @outer6() {
; CHECK-LABEL: @outer6(
entry:
  call void @inner6()
; CHECK-NOT: call void @inner6

  ret void
; CHECK: ret void
}
