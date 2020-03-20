; RUN:  opt -load %shlibdir/libOpcodeCounter%shlibext --legacy-opcode-counter %S/Inputs/CallCounterInput.ll -disable-output 2>&1\
; RUN:   | FileCheck %s

; Test 'opcode-counter' when run through opt (both new PM and legacy PM)

; CHECK-LABEL: foo
; CHECK: ret                  1

; CHECK-LABEL: bar
; CHECK: ret                  1
; CHECK-NEXT: call                 1

; CHECK-LABEL: fez
; CHECK: ret                  1
; CHECK-NEXT: call                 1

; CHECK-LABEL: main
; CHECK: load                 2
; CHECK-NEXT: br                   4
; CHECK-NEXT: icmp                 1
; CHECK-NEXT: add                  1
; CHECK-NEXT: ret                  1
; CHECK-NEXT: alloca               2
; CHECK-NEXT: store                4
; CHECK-NEXT: call                 4
