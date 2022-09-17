; RUN:  opt -load-pass-plugin %shlibdir/libRIV%shlibext -passes="print<riv>" -disable-output %s 2>&1 | FileCheck %s

; Verifies that RIV correctly captures global variables.

@var = global i32 123

define i32 @foo() {
  ret i32 1
}

; CHECK-LABEL: BB %0
; CHECK-NEXT:       i32 123
