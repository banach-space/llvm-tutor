; RUN: ../bin/static %S/Inputs/CallCounterInput.ll 2>&1 | FileCheck %s

; Test StaticCallCounter when run via static.

; CHECK: foo                  3
; CHECK: bar                  2
; CHECK: fez                  1
