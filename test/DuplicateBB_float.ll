; RUN: opt -load-pass-plugin %shlibdir/libRIV%shlibext -load-pass-plugin %shlibdir/libDuplicateBB%shlibext -passes=duplicate-bb -S %s | FileCheck  %s

; No integer reachable values (only floats), hence the only BasicBlock in foo
; is *not* duplicated

@var = global float 1.25

define i32 @foo(float %in) {
  ret i32 1
}

; CHECK-LABEL: foo
; CHECK-NEXT:  ret i32 1
