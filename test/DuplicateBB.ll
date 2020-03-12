; RUN: opt -load %shlibdir/libRIV%shlibext -load %shlibdir/libDuplicateBB%shlibext -legacy-duplicate-bb -S %s | FileCheck  %s
; RUN: opt -load-pass-plugin %shlibdir/libRIV%shlibext -load-pass-plugin %shlibdir/libDuplicateBB%shlibext -passes=duplicate-bb -S %s | FileCheck  %s

; Verify that indeed the only BasicBlock in foo is duplcated. It's a trivial
; BasicBlock with only one terminator instruction (terminator instructions are
; not duplicated). In total, 1 BasicBlock is replaced with 4.

define i32 @foo(i32) {
  ret i32 1
}

; CHECK-LABEL: foo
; CHECK-NEXT:   %2 = icmp eq i32 %0, 0
; CHECK: br i1 %2, label %lt-if-then-0, label %lt-else-0

; CHECK: lt-if-then-0:
; CHECK-NEXT:   br label %3

; CHECK: lt-else-0:
; CHECK-NEXT:   br label %3

; CHECK: 3:
; CHECK-NEXT:   ret i32 1
