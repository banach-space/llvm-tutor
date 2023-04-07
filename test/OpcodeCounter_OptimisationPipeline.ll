; RUN:  opt -disable-verify -debug-pass-manager -load-pass-plugin %shlibdir/libOpcodeCounter%shlibext -passes='default<O1>' %s -disable-output 2>&1\
; RUN:   | FileCheck %s
; RUN:  opt -disable-verify -debug-pass-manager -load-pass-plugin %shlibdir/libOpcodeCounter%shlibext -passes='default<O3>' %s -disable-output 2>&1\
; RUN:   | FileCheck %s
; RUN:  opt -disable-verify -debug-pass-manager -load-pass-plugin %shlibdir/libOpcodeCounter%shlibext -passes='default<Oz>' %s -disable-output 2>&1\
; RUN:   | FileCheck %s
; RUN:  opt -disable-verify -debug-pass-manager -load-pass-plugin %shlibdir/libOpcodeCounter%shlibext -passes='default<Os>' %s -disable-output 2>&1\
; RUN:   | FileCheck %s

; CHECK: Running pass: OpcodeCounterPrinter on foo

define void @foo() {
  ret void
}
