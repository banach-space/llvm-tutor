; RUN: opt -load-pass-plugin %shlibdir/libOpcodeCounter%shlibext --passes='default<O1>' -debug-pass-manager  %s -o /dev/null 2>&1 | FileCheck --check-prefixes=CHECK-O1 %s
; RUN: opt -load-pass-plugin %shlibdir/libOpcodeCounter%shlibext --passes='default<O2>' -debug-pass-manager  %s -o /dev/null 2>&1 | FileCheck --check-prefixes=CHECK-O2 %s
; RUN: opt -load-pass-plugin %shlibdir/libOpcodeCounter%shlibext --passes='default<O3>' -debug-pass-manager  %s -o /dev/null 2>&1 | FileCheck --check-prefixes=CHECK-O3 %s
; RUN: opt -load-pass-plugin %shlibdir/libOpcodeCounter%shlibext --passes='default<Os>' -debug-pass-manager  %s -o /dev/null 2>&1 | FileCheck --check-prefixes=CHECK-Os %s

; Verify that OpcodeCounter is run when an optimisation level is specified
; (e.g. -O1 or -O3). Note that contrary to other tests, there's no explicit
; request to run OpcodeCounter (through --legacy-opcode-counter or
; -passes=opcode-counter).

; CHECK-O1: Running pass: OpcodeCounterPrinter on main
; CHECK-O2: Running pass: OpcodeCounterPrinter on main
; CHECK-O3: Running pass: OpcodeCounterPrinter on main
; CHECK-Os: Running pass: OpcodeCounterPrinter on main

define i32 @main(i32, i8** nocapture readnone) {
  ret i32 1
}
