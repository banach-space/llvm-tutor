; RUN:  opt -load-pass-plugin %shlibdir/libOpcodeCounter%shlibext -passes="print<opcode-counter>" %S/Inputs/CallCounterInput.ll -disable-output 2>&1\
; RUN:   | FileCheck %s

;------------------------------------------------------------------------------
; EXPECTED OUTPUT
;------------------------------------------------------------------------------

; CHECK-LABEL: foo
; CHECK: ret                  1

; CHECK-LABEL: bar
; CHECK: call                 1
; CHECK-NEXT: ret                  1

; CHECK-LABEL: fez
; CHECK: call                 1
; CHECK-NEXT: ret                  1

; CHECK-LABEL: main
; CHECK: add                  1
; CHECK-NEXT: call                 4
; CHECK-NEXT: ret                  1
; CHECK-NEXT: load                 2
; CHECK-NEXT: br                   4
; CHECK-NEXT: alloca               2
; CHECK-NEXT: store                4
; CHECK-NEXT: icmp                 1
