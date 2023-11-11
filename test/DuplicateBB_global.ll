; RUN: opt -load-pass-plugin %shlibdir/libRIV%shlibext -load-pass-plugin %shlibdir/libDuplicateBB%shlibext -passes=duplicate-bb -S %s | FileCheck  %s

; No local integer reachable values (only 1 global), hence the only BasicBlock
; in foo is *not* duplicated

@var = global i32 123

define i32 @foo() {
  ret i32 1
}

; CHECK-LABEL: foo
; CHECK-NEXT: ret i32 1
