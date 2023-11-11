; RUN: opt -load-pass-plugin %shlibdir/libRIV%shlibext -load-pass-plugin %shlibdir/libDuplicateBB%shlibext -passes=duplicate-bb -S %s | FileCheck  %s

; Verify that indeed the only BasicBlock in foo is duplcated. It's a trivial
; BasicBlock with only one terminator instruction (terminator instructions are
; not duplicated). In total, 1 BasicBlock is replaced with 4.

define i32 @foo(i32) {
  ret i32 1
}

; CHECK-LABEL: foo
; CHECK-NEXT: lt-if-then-else-0:
; CHECK-NEXT: %1 = icmp eq i32 %0, 0
; CHECK: br i1 %1, label %lt-clone-1-0, label %lt-clone-2-0

; CHECK: lt-clone-1-0:
; CHECK-NEXT:   br label %lt-tail-0

; CHECK: lt-clone-2-0:
; CHECK-NEXT:   br label %lt-tail-0

; CHECK: lt-tail-0:
; CHECK-NEXT:   ret i32 1
