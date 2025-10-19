llvm-tutor
=========
[![Apple Silicon](https://github.com/banach-space/llvm-tutor/actions/workflows/apple-silicon.yml/badge.svg?branch=main)](https://github.com/banach-space/llvm-tutor/actions/workflows/apple-silicon.yml)
[![x86-Ubuntu](https://github.com/banach-space/llvm-tutor/actions/workflows/x86-ubuntu.yml/badge.svg?branch=main)](https://github.com/banach-space/llvm-tutor/actions/workflows/x86-ubuntu.yml)


Example LLVM passes - based on **LLVM 21**

**llvm-tutor** is a collection of self-contained reference LLVM passes. It's a
tutorial that targets novice and aspiring LLVM developers. Key features:

* **Out-of-tree** - builds against a binary LLVM installation (no need to build LLVM from sources)
* **Complete** - includes `CMake` build scripts, LIT tests, CI set-up and documentation
* **Modern** - based on the latest version of LLVM (and updated with every release)

### Overview
LLVM implements a very rich, powerful and popular API. However, like many
complex technologies, it can be quite daunting and overwhelming to learn and
master. The goal of this LLVM tutorial is to showcase that LLVM can in fact be
easy and fun to work with. This is demonstrated through a range self-contained,
testable LLVM passes, which are implemented using idiomatic LLVM.

This document explains how to set-up your environment, build and run the
examples, and go about debugging. It contains a high-level overview of the
implemented examples and contains some background information on writing LLVM
passes. The source files, apart from the code itself, contain comments that
will guide you through the implementation. All examples are complemented with
[LIT](https://llvm.org/docs/TestingGuide.html) tests and reference [input
files](https://github.com/banach-space/llvm-tutor/blob/main/inputs).

Visit [**clang-tutor**](https://github.com/banach-space/clang-tutor/) if you
are interested in similar tutorial for Clang.

### Table of Contents
* [HelloWorld: Your First Pass](#helloworld-your-first-pass)
* Part 1: **llvm-tutor** in more detail
  * [Development Environment](#development-environment)
  * [Building & Testing](#building--testing)
  * [Overview of the Passes](#overview-of-the-passes)
  * [Debugging](#debugging)
* Part 2: Passes In LLVM
  * [Analysis vs Transformation Pass](#analysis-vs-transformation-pass)
  * [Dynamic vs Static Plugins](#dynamic-vs-static-plugins)
  * [Optimisation Passes Inside LLVM](#optimisation-passes-inside-llvm)
* [References](#references)


HelloWorld: Your First Pass
===========================
The **HelloWorld** pass from
[HelloWorld.cpp](https://github.com/banach-space/llvm-tutor/blob/main/HelloWorld/HelloWorld.cpp)
is a self-contained *reference example*. The corresponding
[CMakeLists.txt](https://github.com/banach-space/llvm-tutor/blob/main/HelloWorld/CMakeLists.txt)
implements the minimum set-up for an out-of-source pass.

For every function defined in the input module, **HelloWorld** prints its name
and the number of arguments that it takes. You can build it like this:

```bash
export LLVM_DIR=<installation/dir/of/llvm/21>
mkdir build
cd build
cmake -DLT_LLVM_INSTALL_DIR=$LLVM_DIR <source/dir/llvm/tutor>/HelloWorld/
make
```

Before you can test it, you need to prepare an input file:

```bash
# Generate an LLVM test file
$LLVM_DIR/bin/clang -O1 -S -emit-llvm <source/dir/llvm/tutor>/inputs/input_for_hello.c -o input_for_hello.ll
```

Finally, run **HelloWorld** with
[**opt**](http://llvm.org/docs/CommandGuide/opt.html) (use `libHelloWorld.so`
on Linux and `libHelloWorld.dylib` on Mac OS):

```bash
# Run the pass
$LLVM_DIR/bin/opt -load-pass-plugin ./libHelloWorld.{so|dylib} -passes=hello-world -disable-output input_for_hello.ll
# Expected output
(llvm-tutor) Hello from: foo
(llvm-tutor)   number of arguments: 1
(llvm-tutor) Hello from: bar
(llvm-tutor)   number of arguments: 2
(llvm-tutor) Hello from: fez
(llvm-tutor)   number of arguments: 3
(llvm-tutor) Hello from: main
(llvm-tutor)   number of arguments: 2
```

The **HelloWorld** pass doesn't modify the input module. The `-disable-output`
flag is used to prevent **opt** from printing the output bitcode file.

Development Environment
=======================
## Platform Support And Requirements
This project has been tested on **Ubuntu 22.04** and **Mac OS X 11.7**. In
order to build **llvm-tutor** you will need:
  * LLVM 21
  * C++ compiler that supports C++17
  * CMake 3.20 or higher

In order to run the passes, you will need:
  * **clang-21** (to generate input LLVM files)
  * [**opt**](http://llvm.org/docs/CommandGuide/opt.html) (to run the passes)

There are additional requirements for tests (these will be satisfied by
installing LLVM 21):
  * [**lit**](https://llvm.org/docs/CommandGuide/lit.html) (aka **llvm-lit**,
    LLVM tool for executing the tests)
  * [**FileCheck**](https://llvm.org/docs/CommandGuide/FileCheck.html) (LIT
    requirement, it's used to check whether tests generate the expected output)

## Installing LLVM 21 on Mac OS X
On Darwin you can install LLVM 21 with [Homebrew](https://brew.sh/):

```bash
brew install llvm@21
```

If you already have an older version of LLVM installed, you can upgrade it to
LLVM 21 like this:

```bash
brew upgrade llvm
```

Once the installation (or upgrade) is complete, all the required header files,
libraries and tools will be located in `/opt/homebrew/opt/llvm/`.

## Installing LLVM 21 on Ubuntu
On Ubuntu Jammy Jellyfish, you can install modern LLVM from the official
[repository](http://apt.llvm.org/):

```bash
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt-add-repository "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-21 main"
sudo apt-get update
sudo apt-get install -y llvm-21 llvm-21-dev llvm-21-tools clang-21
```
This will install all the required header files, libraries and tools in
`/usr/lib/llvm-21/`.

## Building LLVM 21 From Sources
Building from sources can be slow and tricky to debug. It is not necessary, but
might be your preferred way of obtaining LLVM 21. The following steps will work
on Linux and Mac OS X:

```bash
git clone https://github.com/llvm/llvm-project.git
cd llvm-project
git checkout release/21.x
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=host -DLLVM_ENABLE_PROJECTS=clang <llvm-project/root/dir>/llvm/
cmake --build .
```
For more details read the [official
documentation](https://llvm.org/docs/CMake.html).

Building & Testing
===================
## Building
You can build **llvm-tutor** (and all the provided pass plugins) as follows:

```bash
cd <build/dir>
cmake -DLT_LLVM_INSTALL_DIR=<installation/dir/of/llvm/21> <source/dir/llvm/tutor>
make
```

The `LT_LLVM_INSTALL_DIR` variable should be set to the root of either the
installation or build directory of LLVM 21. It is used to locate the
corresponding `LLVMConfig.cmake` script that is used to set the include and
library paths.

## Testing
In order to run **llvm-tutor** tests, you need to install **llvm-lit** (aka
**lit**). It's not bundled with LLVM 21 packages, but you can install it with
**pip**:

```bash
# Install lit - note that this installs lit globally
pip install lit
```
Running the tests is as simple as:

```bash
$ lit <build_dir>/test
```
Voilà! You should see all tests passing.

## LLVM Plugins as shared objects
In **llvm-tutor** every LLVM pass is implemented in a separate shared object
(you can learn more about shared objects
[here](http://www.yolinux.com/TUTORIALS/LibraryArchives-StaticAndDynamic.html)).
These shared objects are essentially dynamically loadable plugins for **opt**.
All plugins are built in the `<build/dir>/lib` directory.

Note that the extension of dynamically loaded shared objects differs between
Linux and Mac OS. For example, for the **HelloWorld** pass you will get:

* `libHelloWorld.so` on Linux
* `libHelloWorld.dylib` on MacOS.

For the sake of consistency, in this README.md file all examples use the `*.so`
extension. When working on Mac OS, use `*.dylib` instead.

Overview of The Passes
======================
The available passes are categorised as either Analysis, Transformation or CFG.
The difference between Analysis and Transformation passes is rather
self-explanatory ([here](#analysis-vs-transformation-pass) is a more technical
breakdown). A CFG pass is simply a Transformation pass that modifies the Control
Flow Graph. This is frequently a bit more complex and requires some extra bookkeeping,
hence a dedicated category.

In the following table the passes are grouped thematically and ordered by the
level of complexity.

| Name      | Description     | Category |
|-----------|-----------------|------|
|[**HelloWorld**](#helloworld-your-first-pass) | visits all functions and prints their names | Analysis |
|[**OpcodeCounter**](#opcodecounter) | prints a summary of LLVM IR opcodes in the input module | Analysis |
|[**InjectFuncCall**](#injectfunccall) | instruments the input module by inserting calls to `printf` | Transformation |
|[**StaticCallCounter**](#staticcallcounter) | counts direct function calls at compile-time (static analysis) | Analysis |
|[**DynamicCallCounter**](#dynamiccallcounter) | counts direct function calls at run-time (dynamic analysis) | Transformation |
|[**MBASub**](#mbasub) | obfuscate integer `sub` instructions | Transformation |
|[**MBAAdd**](#mbaadd) | obfuscate 8-bit integer `add` instructions | Transformation |
|[**FindFCmpEq**](#findfcmpeq) | finds floating-point equality comparisons | Analysis |
|[**ConvertFCmpEq**](#convertfcmpeq) | converts direct floating-point equality comparisons to difference comparisons | Transformation |
|[**RIV**](#riv) | finds reachable integer values for each basic block | Analysis |
|[**DuplicateBB**](#duplicatebb) | duplicates basic blocks, requires **RIV** analysis results | CFG |
|[**MergeBB**](#mergebb) | merges duplicated basic blocks | CFG |

Once you've [built](#building--testing) this project, you can experiment with
every pass separately. All passes, except for
[**HelloWorld**](#helloworld-your-first-pass), are described in more details
below.

LLVM passes work with LLVM IR files. You can generate one like this:

```bash
export LLVM_DIR=<installation/dir/of/llvm/21>
# Textual form
$LLVM_DIR/bin/clang -O1 -emit-llvm input.c -S -o out.ll
# Binary/bit-code form
$LLVM_DIR/bin/clang -O1 -emit-llvm input.c -c -o out.bc
```
It doesn't matter whether you choose the binary, `*.bc` (default), or
textual/LLVM assembly form (`.ll`, requires the `-S` flag). Obviously, the
latter is more human-readable. Similar logic applies to **opt** - by default it
generates `*.bc` files. You can use `-S` to have the output written as `*.ll`
files instead.

Note that `clang` adds the `optnone` [function
attribute](https://llvm.org/docs/LangRef.html#function-attributes) if either

* no optimization level is specified, or
* `-O0` is specified.

If you want to compile at `-O0`, you need to specify `-O0 -Xclang
-disable-O0-optnone` or define a static
[isRequired](https://llvm.org/docs/WritingAnLLVMNewPMPass.html#required-passes)
method in your pass.  Alternatively, you can specify `-O1` or higher.
Otherwise the new pass manager will register the pass but your pass will not be
executed.

As noted [earlier](#llvm-plugins-as-shared-objecs), all examples in this file
use the `*.so` extension for pass plugins. When working on Mac OS, use
`*.dylib` instead.

## OpcodeCounter
**OpcodeCounter** is an Analysis pass that prints a summary of the [LLVM IR
opcodes](https://github.com/llvm/llvm-project/blob/release/21.x/llvm/lib/IR/Instruction.cpp#L397-L480)
encountered in every function in the input module. This pass can be [run
automatically](#auto-registration-with-optimisation-pipelines) with one of the
pre-defined optimisation pipelines. However, let's use our tried and tested method
first.

### Run the pass
We will use
[input_for_cc.c](https://github.com/banach-space/llvm-tutor/blob/main/inputs/input_for_cc.c)
to test **OpcodeCounter**. Since **OpcodeCounter** is an Analysis pass, we want
**opt** to _print_ its results. To this end, we will use a [printing
pass](#printing-passes-for-the-new-pass-manager) that corresponds to
**OpcodeCounter**. This pass is called `print<opcode-counter>`. No extra
arguments are needed, but it's a good idea to add `-disable-output` to prevent
**opt** from printing the output LLVM IR module - we are only interested in the
results of the analysis rather than the module itself. In fact, as this pass
does not modify the input IR, the output module would be identical to the
input anyway.

```bash
export LLVM_DIR=<installation/dir/of/llvm/21>
# Generate an LLVM file to analyze
$LLVM_DIR/bin/clang -emit-llvm -c <source_dir>/inputs/input_for_cc.c -o input_for_cc.bc
# Run the pass through opt
$LLVM_DIR/bin/opt -load-pass-plugin <build_dir>/lib/libOpcodeCounter.so --passes="print<opcode-counter>" -disable-output input_for_cc.bc
```

For `main`, **OpcodeCounter** prints the following summary (note that when running the pass,
a summary for other functions defined in `input_for_cc.bc` is also printed):

```
=================================================
LLVM-TUTOR: OpcodeCounter results for `main`
=================================================
OPCODE               #N TIMES USED
-------------------------------------------------
load                 2
br                   4
icmp                 1
add                  1
ret                  1
alloca               2
store                4
call                 4
-------------------------------------------------
```

### Auto-registration with optimisation pipelines
You can run **OpcodeCounter** by simply specifying an optimisation level (e.g.
`-O{1|2|3|s}`). This is achieved through auto-registration with the existing
optimisation pass pipelines. Note that you still have to specify the plugin
file to be loaded:

```bash
$LLVM_DIR/bin/opt -load-pass-plugin <build_dir>/lib/libOpcodeCounter.so --passes='default<O1>' input_for_cc.bc
```

This is implemented in
[OpcodeCounter.cpp](https://github.com/banach-space/llvm-tutor/blob/main/lib/OpcodeCounter.cpp),
on
[line 106](https://github.com/banach-space/llvm-tutor/blob/main/lib/OpcodeCounter.cpp#L106-L110).

## InjectFuncCall
This pass is a _HelloWorld_ example for _code instrumentation_. For every function
defined in the input module, **InjectFuncCall** will add (_inject_) the following
call to [`printf`](https://en.cppreference.com/w/cpp/io/c/fprintf):

```C
printf("(llvm-tutor) Hello from: %s\n(llvm-tutor)   number of arguments: %d\n", FuncName, FuncNumArgs)
```
This call is added at the beginning of each function (i.e. before any other
instruction). `FuncName` is the name of the function and `FuncNumArgs` is the
number of arguments that the function takes.

### Run the pass
We will use
[input_for_hello.c](https://github.com/banach-space/llvm-tutor/blob/main/inputs/input_for_hello.c)
to test **InjectFuncCall**:

```bash
export LLVM_DIR=<installation/dir/of/llvm/21>
# Generate an LLVM file to analyze
$LLVM_DIR/bin/clang -O0 -emit-llvm -c <source_dir>/inputs/input_for_hello.c -o input_for_hello.bc
# Run the pass through opt
$LLVM_DIR/bin/opt -load-pass-plugin <build_dir>/lib/libInjectFuncCall.so --passes="inject-func-call" input_for_hello.bc -o instrumented.bin
```
This generates `instrumented.bin`, which is the instrumented version of
`input_for_hello.bc`. In order to verify that **InjectFuncCall** worked as
expected, you can either check the output file (and verify that it contains
extra calls to `printf`) or run it:

```
$LLVM_DIR/bin/lli instrumented.bin
(llvm-tutor) Hello from: main
(llvm-tutor)   number of arguments: 2
(llvm-tutor) Hello from: foo
(llvm-tutor)   number of arguments: 1
(llvm-tutor) Hello from: bar
(llvm-tutor)   number of arguments: 2
(llvm-tutor) Hello from: foo
(llvm-tutor)   number of arguments: 1
(llvm-tutor) Hello from: fez
(llvm-tutor)   number of arguments: 3
(llvm-tutor) Hello from: bar
(llvm-tutor)   number of arguments: 2
(llvm-tutor) Hello from: foo
(llvm-tutor)   number of arguments: 1
```

### InjectFuncCall vs HelloWorld
You might have noticed that **InjectFuncCall** is somewhat similar to
[**HelloWorld**](#helloworld-your-first-pass). In both cases the pass visits
all functions, prints their names and the number of arguments. The difference
between the two passes becomes quite apparent when you compare the output
generated for the same input file, e.g. `input_for_hello.c`. The number of
times `Hello from` is printed is either:
* once per every function call in the case of **InjectFuncCall**, or
* once per function definition in the case of **HelloWorld**.

This makes perfect sense and hints how different the two passes are. Whether to
print `Hello from` is determined at either:
* run-time for **InjectFuncCall**, or
* compile-time for **HelloWorld**.

Also, note that in the case of **InjectFuncCall** we had to first run the pass
with **opt** and then execute the instrumented IR module in order to see the
output.  For **HelloWorld** it was sufficient to run the pass with **opt**.

## StaticCallCounter
The **StaticCallCounter** pass counts the number of _static_ function calls in
the input LLVM module. _Static_ refers to the fact that these function calls
are compile-time calls (i.e. visible during the compilation). This is in
contrast to _dynamic_ function calls, i.e. function calls encountered at
run-time (when the compiled module is run). The distinction becomes apparent
when analysing functions calls within loops, e.g.:
```c
  for (i = 0; i < 10; i++)
    foo();
```
Although at run-time `foo` will be executed 10 times, **StaticCallCounter**
will report only 1 function call.

This pass will only consider direct functions calls. Functions calls via
function pointers are not taken into account.

### Run the pass through **opt**
We will use
[input_for_cc.c](https://github.com/banach-space/llvm-tutor/blob/main/inputs/input_for_cc.c)
to test **StaticCallCounter**:

```bash
export LLVM_DIR=<installation/dir/of/llvm/21>
# Generate an LLVM file to analyze
$LLVM_DIR/bin/clang -emit-llvm -c <source_dir>/inputs/input_for_cc.c -o input_for_cc.bc
# Run the pass through opt
$LLVM_DIR/bin/opt -load-pass-plugin <build_dir>/lib/libStaticCallCounter.so -passes="print<static-cc>" -disable-output input_for_cc.bc
```
You should see the following output:

```
=================================================
LLVM-TUTOR: static analysis results
=================================================
NAME                 #N DIRECT CALLS
-------------------------------------------------
foo                  3
bar                  2
fez                  1
-------------------------------------------------
```

Note that in order to print the output, you will have to use the printing pass
that corresponds to **StaticCallCounter** (by passing
`-passes="print<static-cc>"` to **opt**). We discussed printing passes in more
detail [here](#run-the-pass).

### Run the pass through `static`
You can run **StaticCallCounter** through a standalone tool called `static`.
`static` is an LLVM based tool implemented in
[StaticMain.cpp](https://github.com/banach-space/llvm-tutor/blob/main/tools/StaticMain.cpp).
It is a command line wrapper that allows you to run **StaticCallCounter**
without the need for **opt**:

```bash
<build_dir>/bin/static input_for_cc.bc
```
It is an example of a relatively basic static analysis tool. Its implementation
demonstrates how basic pass management in LLVM works (i.e. it handles that for
itself instead of relying on **opt**).

## DynamicCallCounter
The **DynamicCallCounter** pass counts the number of _run-time_ (i.e.
encountered during the execution) function calls. It does so by inserting
call-counting instructions that are executed every time a function is called.
Only calls to functions that are _defined_ in the input module are counted.
This pass builds on top of ideas presented in
[**InjectFuncCall**](#injectfunccall). You may want to experiment with that
example first.

### Run the pass
We will use
[input_for_cc.c](https://github.com/banach-space/llvm-tutor/blob/main/inputs/input_for_cc.c)
to test **DynamicCallCounter**:

```bash
export LLVM_DIR=<installation/dir/of/llvm/21>
# Generate an LLVM file to analyze
$LLVM_DIR/bin/clang -emit-llvm -c <source_dir>/inputs/input_for_cc.c -o input_for_cc.bc
# Instrument the input file
$LLVM_DIR/bin/opt -load-pass-plugin=<build_dir>/lib/libDynamicCallCounter.so -passes="dynamic-cc" input_for_cc.bc -o instrumented_bin
```
This generates `instrumented.bin`, which is the instrumented version of
`input_for_cc.bc`. In order to verify that **DynamicCallCounter** worked as
expected, you can either check the output file (and verify that it contains
new call-counting instructions) or run it:

```bash
# Run the instrumented binary
$LLVM_DIR/bin/lli  ./instrumented_bin
```
You will see the following output:

```
=================================================
LLVM-TUTOR: dynamic analysis results
=================================================
NAME                 #N DIRECT CALLS
-------------------------------------------------
foo                  13
bar                  2
fez                  1
main                 1
```

### DynamicCallCounter vs StaticCallCounter
The number of function calls reported by **DynamicCallCounter** and
**StaticCallCounter** are different, but both results are correct. They
correspond to _run-time_ and _compile-time_ function calls respectively. Note
also that for **StaticCallCounter** it was sufficient to run the pass through
**opt** to have the summary printed. For **DynamicCallCounter** we had to _run
the instrumented binary_ to see the output. This is similar to what we observed
when comparing [HelloWorld and InjectFuncCall](#injectfunccall-vs-helloworld).

## Mixed Boolean Arithmetic Transformations
These passes implement [mixed
boolean arithmetic](https://tel.archives-ouvertes.fr/tel-01623849/document)
transformations. Similar transformation are often used in code obfuscation (you
may also know them from [Hacker's
Delight](https://www.amazon.co.uk/Hackers-Delight-Henry-S-Warren/dp/0201914654))
and are a great illustration of what and how LLVM passes can be used for.

Similar transformations are possible at the source-code level. The relevant
Clang plugins are available in
[**clang-tutor**](https://github.com/banach-space/clang-tutor#obfuscator).

### MBASub
The **MBASub** pass implements this rather basic expression:

```
a - b == (a + ~b) + 1
```
Basically, it replaces all instances of integer `sub` according to the above
formula. The corresponding LIT tests verify that both the formula  and that the
implementation are correct.

#### Run the pass
We will use
[input_for_mba_sub.c](https://github.com/banach-space/llvm-tutor/blob/main/inputs/input_for_mba_sub.c)
to test **MBASub**:

```bash
export LLVM_DIR=<installation/dir/of/llvm/21>
$LLVM_DIR/bin/clang -emit-llvm -S <source_dir>/inputs/input_for_mba_sub.c -o input_for_sub.ll
$LLVM_DIR/bin/opt -load-pass-plugin=<build_dir>/lib/libMBASub.so -passes="mba-sub" -S input_for_sub.ll -o out.ll
```

### MBAAdd
The **MBAAdd** pass implements a slightly more involved formula that is only
valid for 8 bit integers:

```
a + b == (((a ^ b) + 2 * (a & b)) * 39 + 23) * 151 + 111
```
Similarly to `MBASub`, it replaces all instances of integer `add` according to
the above identity, but only for 8-bit integers. The LIT tests verify that both
the formula and the implementation are correct.

#### Run the pass
We will use
[input_for_add.c](https://github.com/banach-space/llvm-tutor/blob/main/inputs/input_for_mba.c)
to test **MBAAdd**:

```bash
export LLVM_DIR=<installation/dir/of/llvm/21>
$LLVM_DIR/bin/clang -O1 -emit-llvm -S <source_dir>/inputs/input_for_mba.c -o input_for_mba.ll
$LLVM_DIR/bin/opt -load-pass-plugin=<build_dir>/lib/libMBAAdd.so -passes="mba-add" -S input_for_mba.ll -o out.ll
```

## RIV
**RIV** is an analysis pass that for each [basic
block](http://llvm.org/docs/ProgrammersManual.html#the-basicblock-class) BB in
the input function computes the set reachable integer values, i.e. the integer
values that are visible (i.e. can be used) in BB. Since the pass operates on
the LLVM IR representation of the input file, it takes into account all values
that have [integer type](https://llvm.org/docs/LangRef.html#integer-type) in
the [LLVM IR](https://llvm.org/docs/LangRef.html) sense. In particular, since
at the LLVM IR level booleans are represented as 1-bit wide integers (i.e.
`i1`), you will notice that booleans are also included in the result.

This pass demonstrates how to request results from other analysis passes in
LLVM. In particular, it relies on the [Dominator
Tree](https://en.wikipedia.org/wiki/Dominator_(graph_theory)) analysis pass
from LLVM, which is used to obtain the dominance tree for the basic blocks
in the input function.

### Run the pass
We will use
[input_for_riv.c](https://github.com/banach-space/llvm-tutor/blob/main/inputs/input_for_riv.c)
to test **RIV**:

```bash
export LLVM_DIR=<installation/dir/of/llvm/21>
# Generate an LLVM file to analyze
$LLVM_DIR/bin/clang -emit-llvm -S -O1 <source_dir>/inputs/input_for_riv.c -o input_for_riv.ll
# Run the pass through opt
$LLVM_DIR/bin/opt -load-pass-plugin <build_dir>/lib/libRIV.so -passes="print<riv>" -disable-output input_for_riv.ll
```
You will see the following output:

```
=================================================
LLVM-TUTOR: RIV analysis results
=================================================
BB id      Reachable Integer Values
-------------------------------------------------
BB %entry
             i32 %a
             i32 %b
             i32 %c
BB %if.then
               %add = add nsw i32 %a, 123
               %cmp = icmp sgt i32 %a, 0
             i32 %a
             i32 %b
             i32 %c
BB %if.end8
               %add = add nsw i32 %a, 123
               %cmp = icmp sgt i32 %a, 0
             i32 %a
             i32 %b
             i32 %c
BB %if.then2
               %mul = mul nsw i32 %b, %a
               %div = sdiv i32 %b, %c
               %cmp1 = icmp eq i32 %mul, %div
               %add = add nsw i32 %a, 123
               %cmp = icmp sgt i32 %a, 0
             i32 %a
             i32 %b
             i32 %c
BB %if.else
               %mul = mul nsw i32 %b, %a
               %div = sdiv i32 %b, %c
               %cmp1 = icmp eq i32 %mul, %div
               %add = add nsw i32 %a, 123
               %cmp = icmp sgt i32 %a, 0
             i32 %a
             i32 %b
             i32 %c
```

Note that in order to print the output, you will have to use the printing pass
that corresponds to **RIV** (by passing `-passes="print<riv>"` to **opt**). We
discussed printing passes in more detail [here](#run-the-pass).

## DuplicateBB
This pass will duplicate all basic blocks in a module, with the exception of
basic blocks for which there are no reachable integer values (identified through
the **RIV** pass). An example of such a basic block is the entry block in a
function that:
* takes no arguments and
* is embedded in a module that defines no global values.

Basic blocks are duplicated by first inserting an `if-then-else` construct and
then cloning all the instructions from the original basic block (with the
exception of [PHI
nodes](https://en.wikipedia.org/wiki/Static_single_assignment_form)) into two
new basic blocks (clones of the original basic block). The `if-then-else`
construct is introduced as a non-trivial mechanism that decides which of the
cloned basic blocks to branch to. This condition is equivalent to:

```cpp
if (var == 0)
  goto clone 1
else
  goto clone 2
```
in which:
* `var` is a randomly picked variable from the `RIV` set for the current basic
  block
* `clone 1` and `clone 2` are labels for the cloned basic blocks.

The complete transformation looks like this:

```c
BEFORE:                     AFTER:
-------                     ------
                              [ if-then-else ]
             DuplicateBB           /  \
[ BB ]      ------------>   [clone 1] [clone 2]
                                   \  /
                                 [ tail ]

LEGEND:
-------
[BB]           - the original basic block
[if-then-else] - a new basic block that contains the if-then-else statement (inserted by DuplicateBB)
[clone 1|2]    - two new basic blocks that are clones of BB (inserted by DuplicateBB)
[tail]         - the new basic block that merges [clone 1] and [clone 2] (inserted by DuplicateBB)
```
As depicted above, **DuplicateBB** replaces qualifying basic blocks with 4 new
basic blocks. This is implemented through LLVM's
[SplitBlockAndInsertIfThenElse](https://github.com/llvm/llvm-project/blob/release/21.x/llvm/include/llvm/Transforms/Utils/BasicBlockUtils.h#L471).
**DuplicateBB** does all the necessary preparation and clean-up. In other
words, it's an elaborate wrapper for LLVM's `SplitBlockAndInsertIfThenElse`.

### Run the pass
This pass depends on the **RIV** pass, which also needs be loaded in order for
**DuplicateBB** to work. Let's use
[input_for_duplicate_bb.c](https://github.com/banach-space/llvm-tutor/blob/main/inputs/input_for_duplicate_bb.c)
as our sample input. First, generate the LLVM file:

```bash
export LLVM_DIR=<installation/dir/of/llvm/21>
$LLVM_DIR/bin/clang -emit-llvm -S -O1 <source_dir>/inputs/input_for_duplicate_bb.c -o input_for_duplicate_bb.ll
```

Function `foo` in `input_for_duplicate_bb.ll` should look like this (all metadata has been stripped):

```llvm
define i32 @foo(i32) {
  ret i32 1
}
```
Note that there's only one basic block (the _entry_ block) and that `foo` takes
one argument (this means that the result from **RIV** will be a non-empty set).
We will now apply **DuplicateBB** to `foo`:

```bash
$LLVM_DIR/bin/opt -load-pass-plugin <build_dir>/lib/libRIV.so -load-pass-plugin <build_dir>/lib/libDuplicateBB.so -passes=duplicate-bb -S input_for_duplicate_bb.ll -o duplicate.ll
```
After the instrumentation `foo` will look like this (all metadata has been stripped):

```llvm
define i32 @foo(i32) {
lt-if-then-else-0:
  %2 = icmp eq i32 %0, 0
  br i1 %2, label %lt-if-then-0, label %lt-else-0

clone-1-0:
  br label %lt-tail-0

clone-2-0:
  br label %lt-tail-0

lt-tail-0:
  ret i32 1
}
```
There are four basic blocks instead of one. All new basic blocks end with a
numeric id of the original basic block (`0` in this case). `lt-if-then-else-0`
contains the new `if-then-else` condition. `clone-1-0` and `clone-2-0` are
clones of the original basic block in `foo`. `lt-tail-0` is the extra basic
block that's required to merge `clone-1-0` and `clone-2-0`.

## MergeBB
**MergeBB** will merge qualifying basic blocks that are identical. To some
extent, this pass reverts the transformations introduced by **DuplicateBB**.
This is illustrated below:

```c
BEFORE:                     AFTER DuplicateBB:                 AFTER MergeBB:
-------                     ------------------                 --------------
                              [ if-then-else ]                 [ if-then-else* ]
             DuplicateBB           /  \               MergeBB         |
[ BB ]      ------------>   [clone 1] [clone 2]      -------->    [ clone ]
                                   \  /                               |
                                 [ tail ]                         [ tail* ]

LEGEND:
-------
[BB]           - the original basic block
[if-then-else] - a new basic block that contains the if-then-else statement (**DuplicateBB**)
[clone 1|2]    - two new basic blocks that are clones of BB (**DuplicateBB**)
[tail]         - the new basic block that merges [clone 1] and [clone 2] (**DuplicateBB**)
[clone]        - [clone 1] and [clone 2] after merging, this block should be very similar to [BB] (**MergeBB**)
[label*]       - [label] after being updated by **MergeBB**
```
Recall that **DuplicateBB** replaces all qualifying basic block with four new
basic blocks, two of which are clones of the original block.  **MergeBB** will
merge those two clones back together, but it will not remove the remaining two
blocks added by **DuplicateBB** (it will update them though).

### Run the pass
Let's use the following IR implementation of `foo` as input. Note that basic
blocks 3 and 5 are identical and can safely be merged:

```llvm
define i32 @foo(i32) {
  %2 = icmp eq i32 %0, 19
  br i1 %2, label %3, label %5

; <label>:3:
  %4 = add i32 %0,  13
  br label %7

; <label>:5:
  %6 = add i32 %0,  13
  br label %7

; <label>:7:
  %8 = phi i32 [ %4, %3 ], [ %6, %5 ]
  ret i32 %8
}
```
We will now apply **MergeBB** to `foo`:

```bash
$LLVM_DIR/bin/opt -load <build_dir>/lib/libMergeBB.so -legacy-merge-bb -S foo.ll -o merge.ll
```
After the instrumentation `foo` will look like this (all metadata has been stripped):
```llvm
define i32 @foo(i32) {
  %2 = icmp eq i32 %0, 19
  br i1 %2, label %3, label %3

3:
  %4 = add i32 %0, 13
  br label %5

5:
  ret i32 %4
}
```
As you can see, basic blocks 3 and 5 from the input module have been merged
into one basic block.


### Run MergeBB on the output from DuplicateBB
It is really interesting to see the effect of **MergeBB** on the output from
**DuplicateBB**. Let's start with the same input as we used for **DuplicateBB**:

```bash
export LLVM_DIR=<installation/dir/of/llvm/21>
$LLVM_DIR/bin/clang -emit-llvm -S -O1 <source_dir>/inputs/input_for_duplicate_bb.c -o input_for_duplicate_bb.ll
```

Now we will apply **DuplicateBB** _and_ **MergeBB** (in this order) to `foo`.
Recall that **DuplicateBB** requires **RIV**, which means that in total we have
to load three plugins:

```bash
$LLVM_DIR/bin/opt -load-pass-plugin <build_dir>/lib/libRIV.so -load-pass-plugin <build_dir>/lib/libMergeBB.so -load-pass-plugin <build-dir>/lib/libDuplicateBB.so -passes=duplicate-bb,merge-bb -S input_for_duplicate_bb.ll -o merge_after_duplicate.ll
```
And here's the output:

```llvm
define i32 @foo(i32) {
lt-if-then-else-0:
  %1 = icmp eq i32 %0, 0
  br i1 %1, label %lt-clone-2-0, label %lt-clone-2-0

lt-clone-2-0:
  br label %lt-tail-0

lt-tail-0:
  ret i32 1
}
```
Compare this with the [output generated by **DuplicateBB**](#run-the-pass-7).
Only one of the clones, `lt-clone-2-0`, has been  preserved, and
`lt-if-then-else-0` has been updated accordingly. Regardless of the value of of
the `if` condition (more precisely, variable `%1`), the control flow jumps to
`lt-clone-2-0`.

## FindFCmpEq
The **FindFCmpEq** pass finds all floating-point comparison operations that 
directly check for equality between two values. This is important because these
sorts of comparisons can sometimes be indicators of logical issues due to 
[rounding errors](https://en.wikipedia.org/wiki/Machine_epsilon) inherent in 
floating-point arithmetic.

**FindFCmpEq** is implemented as two passes: an analysis pass (`FindFCmpEq`) and a 
printing pass (`FindFCmpEqPrinter`). The legacy implementation (`FindFCmpEqWrapper`) 
makes use of both of these passes.

### Run the pass
We will use [input_for_fcmp_eq.ll](https://github.com/banach-space/llvm-tutor/blob/main/inputs/input_for_fcmp_eq.c)
to test **FindFCmpEq**:

```bash
export LLVM_DIR=<installation/dir/of/llvm/21>
# Generate the input file
$LLVM_DIR/bin/clang -emit-llvm -S -Xclang -disable-O0-optnone -c <source_dir>/inputs/input_for_fcmp_eq.c -o input_for_fcmp_eq.ll
# Run the pass
$LLVM_DIR/bin/opt --load-pass-plugin <build_dir>/lib/libFindFCmpEq.so --passes="print<find-fcmp-eq>" -disable-output input_for_fcmp_eq.ll
```

You should see the following output which lists the direct floating-point equality comparison instructions found:

```llvm
Floating-point equality comparisons in "sqrt_impl":
  %11 = fcmp oeq double %9, %10
Floating-point equality comparisons in "main":
  %9 = fcmp oeq double %8, 1.000000e+00
  %13 = fcmp oeq double %11, %12
  %19 = fcmp oeq double %17, %18
```

## ConvertFCmpEq
The **ConvertFCmpEq** pass is a transformation that uses the analysis results
of [**FindFCmpEq**](#FindFCmpEq) to convert direct floating-point equality
comparison instructions into logically equivalent ones that use a
pre-calculated rounding threshold.

### Run the pass
As with [**FindFCmpEq**](#FindFCmpEq), we will use
[input_for_fcmp_eq.ll](https://github.com/banach-space/llvm-tutor/blob/main/inputs/input_for_fcmp_eq.c)
to test **ConvertFCmpEq**:

```bash
export LLVM_DIR=<installation/dir/of/llvm/21>
$LLVM_DIR/bin/clang -emit-llvm -S -Xclang -disable-O0-optnone \
  -c <source_dir>/inputs/input_for_fcmp_eq.c -o input_for_fcmp_eq.ll
$LLVM_DIR/bin/opt --load-pass-plugin <build_dir>/lib/libFindFCmpEq.so \
  --load-pass-plugin <build_dir>/lib/libConvertFCmpEq.so \
  --passes=convert-fcmp-eq -S input_for_fcmp_eq.ll -o fcmp_eq_after_conversion.ll
```

For the legacy implementation, the `opt` command would be changed to the following:

```bash
$LLVM_DIR/bin/opt -load <build_dir>/lib/libFindFCmpEq.so \
  <build_dir>/lib/libConvertFCmpEq.so -convert-fcmp-eq \
  -S input_for_fcmp_eq.ll -o fcmp_eq_after_conversion.ll
```

Notice that both `libFindFCmpEq.so` _and_ `libConvertFCmpEq.so` must be loaded
-- and the load order matters. Since **ConvertFCmpEq** requires
[**FindFCmpEq**](#FindFCmpEq), its library must be loaded before
**ConvertFCmpEq**. If both passes were built as part of the same library, this
would not be required.

After transformation, both `fcmp oeq` instructions will have been converted to
difference based `fcmp olt` instructions using the IEEE 754 double-precision
machine epsilon constant as the round-off threshold:

```llvm
  %cmp = fcmp oeq double %0, %1
```

... has now become

```llvm
  %3 = fsub double %0, %1
  %4 = bitcast double %3 to i64
  %5 = and i64 %4, 9223372036854775807
  %6 = bitcast i64 %5 to double
  %cmp = fcmp olt double %6, 0x3CB0000000000000
```

The values are subtracted from each other and the absolute value of their
difference is calculated. If this absolute difference is less than the value of
the machine epsilon, the original two floating-point values are considered to
be equal.

Debugging
==========
Before running a debugger, you may want to analyze the output from
[LLVM_DEBUG](http://llvm.org/docs/ProgrammersManual.html#the-llvm-debug-macro-and-debug-option)
and
[STATISTIC](http://llvm.org/docs/ProgrammersManual.html#the-statistic-class-stats-option)
macros. For example, for **MBAAdd**:

```bash
export LLVM_DIR=<installation/dir/of/llvm/21>
$LLVM_DIR/bin/clang -emit-llvm -S -O1 <source_dir>/inputs/input_for_mba.c -o input_for_mba.ll
$LLVM_DIR/bin/opt -S -load-pass-plugin <build_dir>/lib/libMBAAdd.so -passes=mba-add input_for_mba.ll -debug-only=mba-add -stats -o out.ll
```
Note the `-debug-only=mba-add` and `-stats` flags in the command line - that's
what enables the following output:

```bash
  %12 = add i8 %1, %0 ->   <badref> = add i8 111, %11
  %20 = add i8 %12, %2 ->   <badref> = add i8 111, %19
  %28 = add i8 %20, %3 ->   <badref> = add i8 111, %27
===-------------------------------------------------------------------------===
                          ... Statistics Collected ...
===-------------------------------------------------------------------------===

3 mba-add - The # of substituted instructions
```
As you can see, you get a nice summary from **MBAAdd**. In many cases this will
be sufficient to understand what might be going wrong. Note that for these
macros to work you need a debug build of LLVM (i.e. **opt**) and **llvm-tutor**
(i.e. use `-DCMAKE_BUILD_TYPE=Debug` instead of `-DCMAKE_BUILD_TYPE=Release`).

For tricker issues just use a debugger. Below I demonstrate how to debug
[**MBAAdd**](#mbaadd). More specifically, how to set up a breakpoint on entry
to `MBAAdd::run`. Hopefully that will be sufficient for you to start.

## Mac OS X
The default debugger on OS X is [LLDB](http://lldb.llvm.org). You will
normally use it like this:

```bash
export LLVM_DIR=<installation/dir/of/llvm/21>
$LLVM_DIR/bin/clang -emit-llvm -S -O1 <source_dir>/inputs/input_for_mba.c -o input_for_mba.ll
lldb -- $LLVM_DIR/bin/opt -S -load-pass-plugin <build_dir>/lib/libMBAAdd.dylib -passes=mba-add input_for_mba.ll -o out.ll
(lldb) breakpoint set --name MBAAdd::run
(lldb) process launch
```
or, equivalently, by using LLDBs aliases:

```bash
export LLVM_DIR=<installation/dir/of/llvm/21>
$LLVM_DIR/bin/clang -emit-llvm -S -O1 <source_dir>/inputs/input_for_mba.c -o input_for_mba.ll
lldb -- $LLVM_DIR/bin/opt -S -load-pass-plugin <build_dir>/lib/libMBAAdd.dylib -passes=mba-add input_for_mba.ll -o out.ll
(lldb) b MBAAdd::run
(lldb) r
```
At this point, LLDB should break at the entry to `MBAAdd::run`.

## Ubuntu
On most Linux systems, [GDB](https://www.gnu.org/software/gdb/) is the most
popular debugger. A typical session will look like this:

```bash
export LLVM_DIR=<installation/dir/of/llvm/21>
$LLVM_DIR/bin/clang -emit-llvm -S -O1 <source_dir>/inputs/input_for_mba.c -o input_for_mba.ll
gdb --args $LLVM_DIR/bin/opt -S -load-pass-plugin <build_dir>/lib/libMBAAdd.so -passes=mba-add input_for_mba.ll -o out.ll
(gdb) b MBAAdd.cpp:MBAAdd::run
(gdb) r
```
At this point, GDB should break at the entry to `MBAAdd::run`.

Analysis vs Transformation Pass
===============================
The implementation of a pass depends on whether it is an Analysis or a
Transformation pass:

* a transformation pass will normally inherit from [PassInfoMixin](https://github.com/llvm/llvm-project/blob/release/21.x/llvm/include/llvm/IR/PassManager.h#L371),
* an analysis pass will inherit from [AnalysisInfoMixin](https://github.com/llvm/llvm-project/blob/release/21.x/llvm/include/llvm/IR/PassManager.h#L394).

This is one of the key characteristics of the New Pass Managers - it makes the
split into Analysis and Transformation passes very explicit. An Analysis pass
requires a bit more bookkeeping and hence a bit more code.  For example, you
need to add an instance of
[AnalysisKey](https://github.com/llvm/llvm-project/blob/release/21.x/llvm/include/llvm/IR/PassManager.h#L410)
so that it can be identified by the New Pass Manager.

Note that for small standalone examples, the difference between Analysis and
Transformation passes becomes less relevant.
[**HelloWorld**](#helloworld-your-first-pass) is a good example. It does not
transform the input module, so in practice it is an Analysis pass. However, in
order to keep the implementation as simple as possible, I used the API for
Transformation passes.

Within **llvm-tutor** the following passes can be used as reference Analysis
and Transformation examples:

* [**OpcodeCounter**](#opcodecounter) - analysis pass
* [**MBASub**](#mbasub) - transformation pass

Other examples also adhere to LLVM's convention, but may contain other
complexities. However, only in the case of
[**HelloWorld**](#helloworld-your-first-pass) simplicity was favoured over
strictness (i.e. it is neither a transformation nor analysis pass).

### Printing passes for the new pass manager
A printing pass for an Analysis pass is basically a Transformation pass that:

* requests the results of the analysis from the original pass, and
* prints these results.

In other words, it's just a wrapper pass. There's a convention to register such
passes under the `print<analysis-pass-name>` command line option.

Dynamic vs Static Plugins
=========================
By default, all examples in **llvm-tutor** are built as
[dynamic plugins](#llvm-plugins-as-shared-objecs). However, LLVM provides
infrastructure for both _dynamic_ and _static_ plugins
([documentation](https://llvm.org/docs/WritingAnLLVMPass.html#building-pass-plugins)).
Static plugins are simply libraries linked into your executable (e.g. **opt**)
statically. This way, unlike dynamic plugins, they don't require to be loaded at
runtime with `-load-pass-plugin`.

Static plugins are normally developed in-tree, i.e. within `llvm-project/llvm`,
and all examples in **llvm-tutor** can be adapted to work this way. You can use
[static_registation.sh](https://github.com/banach-space/llvm-tutor/blob/main/utils/static_registration.sh)
to see it can be done for [**MBASub**](#mbasub). This script will:

* copy the required source and test files into `llvm-project/llvm`
* adapt in-tree CMake scripts so that the in-tree version of **MBASub** is actually built
* remove `-load` and `-load-pass-plugin` from the in-tree tests for **MBASub**

Note that this script will modify `llvm-project/llvm`, but leave **llvm-tutor**
intact. After running the script you will have to re-build **opt**. Two
additional CMake flags have to be set: `LLVM_BUILD_EXAMPLES` and
`LLVM_MBASUB_LINK_INTO_TOOLS`:

```bash
# LLVM_TUTOR_DIR: directory in which you cloned llvm-tutor
cd $LLVM_TUTOR_DIR
# LLVM_PROJECT_DIR: directory in which you cloned llvm-project
bash utils/static_registration.sh --llvm_project_dir $LLVM_PROJECT_DIR
# LLVM_BUILD_DIR: directory in which you previously built opt
cd $LLVM_BUILD_DIR
cmake -DLLVM_BUILD_EXAMPLES=On -DLLVM_MBASUB_LINK_INTO_TOOLS=On .
cmake --build . --target opt
```

Once **opt** is re-built, **MBASub** will be statically linked
into **opt**. Now you can run it like this:

```bash
$LLVM_BUILD_DIR/bin/opt --passes=mba-sub -S $LLVM_TUTOR_DIR/test/MBA_sub.ll
```

Note that this time we didn't have to use `-load-pass-plugin` to load
**MBASub**. If you want to dive deeper into the required steps for static
registration, you can scan `static_registation.sh` or run:

```bash
cd $LLVM_PROJECT_DIR
git diff
git status
```

This will print all the changes within `llvm-project/llvm` introduced by the
script.

Optimisation Passes Inside LLVM
=================================
Apart from writing your own transformations an analyses, you may want to
familiarize yourself with [the passes available within
LLVM](https://llvm.org/docs/Passes.html). It is a great resource for learning
how LLVM works and what makes it so powerful and successful. It is also a great
resource for discovering how compilers work in general. Indeed, many of the
passes implement general concepts known from the theory of compiler development.

The list of the available passes in LLVM can be a bit daunting. Below is a list
of the selected few that are a good starting point. Each entry contains a link
to the implementation in LLVM, a short description and a link to test files
available within **llvm-tutor**. These test files contain a collection of
annotated test cases for the corresponding pass. The goal of these tests is to
demonstrate the functionality of the tested pass through relatively simple
examples.

| Name      | Description     | Test files in llvm-tutor |
|-----------|-----------------|--------------------------|
|[**dce**](https://github.com/llvm/llvm-project/blob/release/21.x/llvm/lib/Transforms/Scalar/DCE.cpp) | Dead Code Elimination | [dce.ll](https://github.com/banach-space/llvm-tutor/blob/main/test/llvm/dce.ll) |
|[**memcpyopt**](https://github.com/llvm/llvm-project/blob/release/21.x/llvm/lib/Transforms/Scalar/MemCpyOptimizer.cpp) | Optimise calls to `memcpy` (e.g. replace them with `memset`) | [memcpyopt.ll](https://github.com/banach-space/llvm-tutor/blob/main/test/llvm/memcpyopt.ll) |
|[**reassociate**](https://github.com/llvm/llvm-project/blob/release/21.x/llvm/lib/Transforms/Scalar/Reassociate.cpp) | Reassociate (e.g. 4 + (x + 5) -> x + (4 + 5)). This enables further optimisations, e.g. LICM. | [reassociate.ll](https://github.com/banach-space/llvm-tutor/blob/main/test/llvm/reassociate.ll) |
|[**always-inline**](https://github.com/llvm/llvm-project/blob/release/21.x/llvm/lib/Transforms/IPO/AlwaysInliner.cpp) | Always inlines functions decorated with [`alwaysinline`](https://llvm.org/docs/LangRef.html#function-attributes) | [always-inline.ll](https://github.com/banach-space/llvm-tutor/blob/main/test/llvm/always-inline.ll) |
|[**loop-deletion**](https://github.com/llvm/llvm-project/blob/release/21.x/llvm/lib/Transforms/Scalar/LoopDeletion.cpp) | Delete unused loops | [loop-deletion.ll](https://github.com/banach-space/llvm-tutor/blob/main/test/llvm/loop-deletion.ll) |
|[**licm**](https://github.com/llvm/llvm-project/blob/release/21.x/llvm/lib/Transforms/Scalar/LICM.cpp) | [Loop-Invariant Code Motion](https://en.wikipedia.org/wiki/Loop-invariant_code_motion) (a.k.a. LICM) | [licm.ll](https://github.com/banach-space/llvm-tutor/blob/main/test/llvm/licm.ll) |
|[**slp**](https://github.com/llvm/llvm-project/blob/release/21.x/llvm/lib/Transforms/Vectorize/SLPVectorizer.cpp) | [Superword-level parallelism vectorisation](https://llvm.org/docs/Vectorizers.html#the-slp-vectorizer) | [slp\_x86.ll](https://github.com/banach-space/llvm-tutor/blob/main/test/llvm/slp_x86.ll), [slp\_aarch64.ll](https://github.com/banach-space/llvm-tutor/blob/main/test/llvm/slp_aarch64.ll)  |

This list focuses on [LLVM's transform
passes](https://llvm.org/docs/Passes.html#transform-passes) that are relatively
easy to demonstrate through small, standalone examples. You can ran an
individual test like this:

```bash
lit <source/dir/llvm/tutor>/test/llvm/always-inline.ll
```

To run an individual pass, extract one [RUN line](https://github.com/banach-space/llvm-tutor/blob/main/test/llvm/always-inline.ll#L2)
from the test file and run it:

```bash
$LLVM_DIR/bin/opt -inline-threshold=0 -passes=always-inline -S <source/dir/llvm/tutor>/test/llvm/always-inline.ll
```

References
===========
Below is a list of LLVM resources available outside the official online
documentation that I have found very helpful. Where possible, the items are sorted by
date.

* **LLVM IR**
  *  _”LLVM IR Tutorial-Phis,GEPs and other things, ohmy!”_, V.Bridgers, F.
Piovezan, EuroLLVM, ([slides](https://llvm.org/devmtg/2019-04/slides/Tutorial-Bridgers-LLVM_IR_tutorial.pdf),
  [video](https://www.youtube.com/watch?v=m8G_S5LwlTo&feature=youtu.be))
  * _"Mapping High Level Constructs to LLVM IR"_, M. Rodler ([link](https://mapping-high-level-constructs-to-llvm-ir.readthedocs.io/en/latest/))
* **Examples in LLVM**
  * Control Flow Graph simplifications:
    [llvm/examples/IRTransforms/](https://github.com/llvm/llvm-project/tree/release/21.x/llvm/examples/IRTransforms)
  * Hello World Pass:
    [llvm/lib/Transforms/Hello/](https://github.com/llvm/llvm-project/blob/release/21.x/llvm/lib/Transforms/Hello)
  * Good Bye World Pass:
    [llvm/examples/Bye/](https://github.com/llvm/llvm-project/tree/release/21.x/llvm/examples/Bye)
* **LLVM Pass Development**
  * _"Writing an LLVM Optimization"_,  Jonathan Smith [video](https://www.youtube.com/watch?v=MagR2KY8MQI&t)
  * _"Getting Started With LLVM: Basics "_, J. Paquette, F. Hahn, LLVM Dev Meeting 2019 [video](https://www.youtube.com/watch?v=3QQuhL-dSys&t=826s)
  * _"Writing an LLVM Pass: 101"_, A. Warzyński, LLVM Dev Meeting 2019 [video](https://www.youtube.com/watch?v=ar7cJl2aBuU)
  * _"Writing LLVM Pass in 2018"_, Min-Yih Hsu [blog](https://medium.com/@mshockwave/writing-llvm-pass-in-2018-preface-6b90fa67ae82)
  * _"Building, Testing and Debugging a Simple out-of-tree LLVM Pass"_ Serge Guelton, Adrien Guinet, LLVM Dev Meeting 2015 ([slides](https://llvm.org/devmtg/2015-10/slides/GueltonGuinet-BuildingTestingDebuggingASimpleOutOfTreePass.pdf), [video](https://www.youtube.com/watch?v=BnlG-owSVTk&index=8&list=PL_R5A0lGi1AA4Lv2bBFSwhgDaHvvpVU21))
* **LLVM Based Tools Development**
  * _"Introduction to LLVM"_, M. Shah, Fosdem 2018, [link](http://www.mshah.io/fosdem18.html)
  * _"Building an LLVM-based tool. Lessons learned"_, A. Denisov, [blog](https://lowlevelbits.org/building-an-llvm-based-tool.-lessons-learned/), [video](https://www.youtube.com/watch?reload=9&v=Yvj4G9B6pcU)

Credits
========
This is first and foremost a community effort. This project wouldn't be
possible without the amazing LLVM [online
documentation](http://llvm.org/docs/), the plethora of great comments in the
source code, and the llvm-dev mailing list. Thank you!

It goes without saying that there's plenty of great presentations on YouTube,
blog posts and GitHub projects that cover similar subjects. I've learnt a great
deal from them - thank you all for sharing! There's one presentation/tutorial
that has been particularly important in my journey as an aspiring LLVM
developer and that helped to _democratise_ out-of-source pass development:

* "Building, Testing and Debugging a Simple out-of-tree LLVM Pass" Serge
  Guelton, Adrien Guinet
  ([slides](https://llvm.org/devmtg/2015-10/slides/GueltonGuinet-BuildingTestingDebuggingASimpleOutOfTreePass.pdf),
  [video](https://www.youtube.com/watch?v=BnlG-owSVTk&index=8&list=PL_R5A0lGi1AA4Lv2bBFSwhgDaHvvpVU21))

Adrien and Serge came up with some great, illustrative and self-contained
examples that are great for learning and tutoring LLVM pass development. You'll
notice that there are similar transformation and analysis passes available in
this project. The implementations available here reflect what **I** found most
challenging while studying them.

License
========
The MIT License (MIT)

Copyright (c) 2019 Andrzej Warzyński

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
