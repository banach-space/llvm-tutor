; Legacy PM - OpcodeCounter is run as part of existing pass pipeline
; RUN:  opt --enable-new-pm=0 -load %shlibdir/libOpcodeCounter%shlibext -O0 -disable-verify -debug-pass=Executions %s -disable-output 2>&1\
; RUN:   | FileCheck --check-prefix=LEGACY_PM %s
; RUN:  opt --enable-new-pm=0 -load %shlibdir/libOpcodeCounter%shlibext -O3 -disable-verify -debug-pass=Executions %s -disable-output 2>&1\
; RUN:   | FileCheck --check-prefix=LEGACY_PM %s
; RUN:  opt --enable-new-pm=0 -load %shlibdir/libOpcodeCounter%shlibext -Os -disable-verify -debug-pass=Executions %s -disable-output 2>&1\
; RUN:   | FileCheck --check-prefix=LEGACY_PM %s
; RUN:  opt --enable-new-pm=0 -load %shlibdir/libOpcodeCounter%shlibext -Oz -disable-verify -debug-pass=Executions %s -disable-output 2>&1\
; RUN:   | FileCheck --check-prefix=LEGACY_PM %s

; New PM - OpcodeCounter is run as part of existing pass pipeline
; With the Legacy PM, OpcodeCounter is also run at `-O0`. However, that's not
; the case with the New PM. I assume that that's because the corresponding
; optimisation pipelines are different.
; TODO: Verify this ^^^.
; RUN:  opt -disable-verify -debug-pass-manager -load-pass-plugin %shlibdir/libOpcodeCounter%shlibext -passes='default<O1>' %s -disable-output 2>&1\
; RUN:   | FileCheck --check-prefix=NEW_PM %s
; RUN:  opt -disable-verify -debug-pass-manager -load-pass-plugin %shlibdir/libOpcodeCounter%shlibext -passes='default<O3>' %s -disable-output 2>&1\
; RUN:   | FileCheck --check-prefix=NEW_PM %s
; RUN:  opt -disable-verify -debug-pass-manager -load-pass-plugin %shlibdir/libOpcodeCounter%shlibext -passes='default<Oz>' %s -disable-output 2>&1\
; RUN:   | FileCheck --check-prefix=NEW_PM %s
; RUN:  opt -disable-verify -debug-pass-manager -load-pass-plugin %shlibdir/libOpcodeCounter%shlibext -passes='default<Os>' %s -disable-output 2>&1\
; RUN:   | FileCheck --check-prefix=NEW_PM %s

;------------------------------------------------------------------------------
; EXPECTED OUTPUT
;------------------------------------------------------------------------------
; LEGACY_PM: Executing Pass 'Legacy OpcodeCounter Pass' on Function 'foo'...
; NEW_PM: Running pass: OpcodeCounterPrinter on foo

define void @foo() {
  ret void
}
