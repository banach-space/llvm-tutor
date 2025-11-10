##  Derived Induction Variable Passes

###  DerivedInductionVar (Analysis Pass)
This pass analyzes loops to detect **derived induction variables (IVs)** — variables that are linearly dependent on the primary loop IV.  
It traverses each loop, printing:
- The **primary induction variable** (`phi` node),
- Any **derived IVs** computed from it,
- Their **step size** and **relationship**.

Ex:
===== Derived Induction Variable Analysis =====
Analyzing loop in function: matmul
Primary IV: %5 = phi i64 [ 0, %3 ], [ %10, %9 ]
Derived IV: %mul = mul i64 %5, 2

