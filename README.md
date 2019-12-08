llvm-tutor
=========
[![Build Status](https://travis-ci.org/banach-space/llvm-tutor.svg?branch=master)](https://travis-ci.org/banach-space/llvm-tutor)
[![Build Status](https://github.com/banach-space/llvm-tutor/workflows/x86-Ubuntu/badge.svg?branch=master)](https://github.com/banach-space/llvm-tutor/actions?query=workflow%3Ax86-Ubuntu+branch%3Amaster)
[![Build Status](https://github.com/banach-space/llvm-tutor/workflows/x86-Darwin/badge.svg?branch=master)](https://github.com/banach-space/llvm-tutor/actions?query=workflow%3Ax86-Darwin+branch%3Amaster)


Example LLVM passes - based on **LLVM 9**

**llvm-tutor** is a collection of self-contained reference LLVM passes. It's a
tutorial that targets novice and aspiring LLVM developers. Key features:
  * **Complete** - includes `CMake` build scripts, LIT tests and CI set-up
  * **Out of source** - builds against a binary LLVM installation (no need to
    build LLVM from sources)
  * **Modern** - based on the latest version of LLVM (and updated with every release)

The source files contain comments that will guide you through the
implementation and the LIT tests verify that each pass works as expected. This
document explains how to get started.

### Table of Contents
* [HelloWorld](#helloworld)
* [Development Environment](#development-environment)
* [Building & Testing](#building--testing)
* [Overview Of The Passes](#overview-of-the-passes)
* [Debugging](#debugging)
* [About Pass Managers in LLVM](#about-pass-managers-in-llvm)
* [Credits & References](#credits)
* [License](#license)


HelloWorld
==========
The **HelloWorld** pass from
[HelloWorld.cpp](https://github.com/banach-space/llvm-tutor/blob/master/HelloWorld/HelloWorld.cpp)
is a self-contained *reference example*. The corresponding
[CMakeLists.txt](https://github.com/banach-space/llvm-tutor/blob/master/HelloWorld/CMakeLists.txt)
implements the minimum set-up for an out-of-source pass.

For every function defined in the input module, **HelloWord** prints its name
and the number of arguments that it takes. You can build it like this:
```bash
export LLVM_DIR=<installation/dir/of/llvm/9>
mkdir build
cd build
cmake -DLT_LLVM_INSTALL_DIR=$LLVM_DIR <source/dir/llvm/tutor>/HelloWorld/
make
```
Before you can test it, you need to prepare an input file:
```bash
# Generate an llvm test file
$LLVM_DIR/bin/clang -S -emit-llvm <source/dir/llvm/tutor/>inputs/input_for_hello.c -o input_for_hello.ll
```
Finally, run **HelloWorld** with [**opt**](http://llvm.org/docs/CommandGuide/opt.html):
```bash
# Run the pass on the llvm file
$LLVM_DIR/bin/opt -load-pass-plugin libHelloWorld.dylib -passes=hello-world -disable-output input_for_hello.ll
# The expected output
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
This project has been tested on **Linux 18.04** and **Mac OS X 10.14.4**. In
order to build **llvm-tutor** you will need:
  * LLVM 9
  * C++ compiler that supports C++14
  * CMake 3.4.3 or higher

In order to run the passes, you will need:
  * **clang-9** (to generate input LLVM files)
  * **opt** (to run the passes)

There are additional requirements for tests (these will be satisfied by
installing LLVM-9):
  * [**lit**](https://llvm.org/docs/CommandGuide/lit.html) (aka **llvm-lit**,
    LLVM tool for executing the tests)
  * [**FileCheck**](https://llvm.org/docs/CommandGuide/lit.html) (LIT
    requirement, it's used to check whether tests generate the expected output)

## Installing LLVM-9 on Mac OS X
On Darwin you can install LLVM 9 with [Homebrew](https://brew.sh/):
```bash
brew install llvm@9
```
This will install all the required header files, libraries and tools in
`/usr/local/opt/llvm/`.

## Installing LLVM-9 on Ubuntu
On Ubuntu Bionic, you can [install modern
LLVM](https://blog.kowalczyk.info/article/k/how-to-install-latest-clang-6.0-on-ubuntu-16.04-xenial-wsl.html)
from the official [repository](http://apt.llvm.org/):
```bash
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt-add-repository "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-9 main"
sudo apt-get update
sudo apt-get install -y llvm-9 llvm-9-dev clang-9 llvm-9-tools
```
This will install all the required header files, libraries and tools in
`/usr/lib/llvm-9/`.

## Building LLVM-9 From Sources
Building from sources can be slow and tricky to debug. It is not necessary, but
might be your preferred way of obtaining LLVM-9. The following steps will work
on Linux and Mac OS X:
```bash
git clone https://github.com/llvm/llvm-project.git
cd llvm-project
git checkout release/9.x
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=X86 <llvm-project/root/dir>/llvm/
cmake --build .
```
For more details read the [official
documentation](https://llvm.org/docs/CMake.html).

Building & Testing
===================
You can build **llvm-tutor** (and all the provided passes) as follows:
```bash
cd <build/dir>
cmake -DLT_LLVM_INSTALL_DIR=<installation/dir/of/llvm/9> <source/dir/llvm/tutor>
make
```

The `LT_LLVM_INSTALL_DIR` variable should be set to the root of either the
installation or build directory of LLVM 9. It is used to locate the
corresponding `LLVMConfig.cmake` script that is used to set the include and
library paths.

In order to run the tests, you need to install **llvm-lit** (aka **lit**). It's
not bundled with LLVM 9 packages, but you can install it with **pip**:
```bash
# Install lit - note that this installs lit globally
pip install lit
```
Running the tests is as simple as:
```bash
$ lit <build_dir>/test
```
Voilà! You should see all tests passing.

Overview of The Passes
======================
   * [**HelloWorld**](#helloworld) - prints the functions in
     the input module and prints the number of arguments for each
   * [**InjectFuncCall**](#inject-calls-to-printf-injectfunccall) - instruments
     the input module by inserting calls to `printf`
   * [**StaticCallCounter**](#count-compile-time-function-calls-staticcallcounter) - counts
     direct function calls at compile time
   * [**DynamicCallCounter**](#count-run-time-function-calls-dynamiccallcounter) - counts
     direct function calls at run-time
   * [**MBASub**](#mbasub) - code transformation for integer `sub`
     instructions
   * [**MBAAdd**](#mbaadd) - code transformation for 8-bit integer `add`
     instructions
   * [**RIV**](#reachable-integer-values-riv) - finds reachable integer values
     for each basic block
   * [**DuplicateBB**](#duplicate-basic-blocks-duplicatebb) - duplicates basic
     blocks, requires **RIV** analysis results

Once you've [built](#build-instructions) this project, you can experiment with
every pass separately. It is assumed that you have `clang` and `opt` available
in your `PATH`. All passes work with LLVM files. You can generate one like
this:
```bash
export LLVM_DIR=<installation/dir/of/llvm/9>
# Textual form
$LLVM_DIR/bin/clang  -emit-llvm input.c -S -o out.ll
# Binary/bit-code form
$LLVM_DIR/bin/clang  -emit-llvm input.c -o out.bc
```
It doesn't matter whether you choose the binary (without `-S`) or textual
form (with `-S`), but obviously the latter is more human-friendly. All passes,
except for [**HelloWorld**](#helloworld), are described below.

## Inject Calls To Printf (**InjectFuncCall**)
This pass is a _HelloWorld_ example for _code instrumentation_. For every function
defined in the input module, `InjectFuncCall` will add (_inject_) the following
call to [`printf`](https://en.cppreference.com/w/cpp/io/c/fprintf):
```C
printf("(llvm-tutor) Hello from: %s\n(llvm-tutor)   number of arguments: %d\n", FuncName, FuncNumArgs)
```
This call is added at the beginning of each function (i.e. before any other
instruction). `FuncName` is the name of the function and `FuncNumArgs` is the
number of arguments that the function takes.

You might have noticed that **InjectFuncCall** is somewhat similar to
[**HelloWorld**](#helloworld) - in both cases the pass visits all functions,
prints their names and the number of arguments. To understand the
difference between the two, lets use `input_for_hello.c` to test
**InjectFuncCall**:

```bash
export LLVM_DIR=<installation/dir/of/llvm/9>
# Generate an LLVM file to analyze
$LLVM_DIR/bin/clang  -emit-llvm -c <source_dir>/inputs/input_for_hello.c -o input_for_hello.bc
# Run the pass through opt
$LLVM_DIR/bin/opt -load <build_dir>/lib/libInjectFuncCall.dylib -legacy-inject-func-call input_for_hello.bc -o instrumented.bin
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
The output is indeed similar to what [**HelloWorld**](#helloworld) would
generate. However, the number of times `Hello from` is printed is different.
Indeed, with **InjectFuncCall** it is printed whenever a function is called,
whereas for **HelloWorld** it is only printed once per function definition. This
makes perfect sense and hints how different the two passes are. _Whether to
print `Hello`_ is determined at:
* compile time (when the pass is run) for **HelloWorld**, and
* run-time (when the instrumented module is run) for **InjectFuncCall**.

Last but not least, note that in the case of **InjectFuncCall** we had to run
the pass with **opt** and then execute the instrumented IR module in order to see the
output. For **HelloWorld** running the pass with **opt** was sufficient.

## Count Compile Time Function Calls (**StaticCallCounter**)
`StaticCallCounter` will count the number of function calls in the input
LLVM file that are visible during the compilation (i.e. if a function is
called within a loop, that counts as one call). Only direct function calls are
considered (TODO: Expand).
```bash
export LLVM_DIR=<installation/dir/of/llvm/9>
# Generate an LLVM file to analyze
$LLVM_DIR/bin/clang  -emit-llvm -c <source_dir>/inputs/input_for_cc.c -o input_for_cc.bc
# Run the pass through opt
$LLVM_DIR/bin/opt -load <build_dir>/lib/libStaticCallCounter.dylib -legacy-static-cc -analyze input_for_cc.bc
```
The `static` executable is a command line wrapper that allows you to run
`StaticCallCounter` without the need for `opt`:
```bash
<build_dir>/bin/static input_for_cc.bc
```

## Count Run-Time Function Calls (**DynamicCallCounter**)
`DynamicCallCounter` will count the number of run-time function calls. It does
so by instrumenting the input LLVM file - it inserts call-counting instructions
that are executed every time a function is called. This pass will only count
calls to functions that are _defined_ in the input module are counted.

Although the primary goal of this pass is to _analyse_ function calls, it also
modifies the input file. Therefore it is a transformation/instrumentation pass.
You can test	it with one of the provided examples, e.g.:

```bash
export LLVM_DIR=<installation/dir/of/llvm/9>
# Generate an LLVM file to analyze
$LLVM_DIR/bin/clang  -emit-llvm -c <source_dir>/inputs/input_for_cc.c -o input_for_cc.bc
# Instrument the input file
$LLVM_DIR/bin/opt -load <build_dir>/lib/libDynamicCallCounter.dylib -legacy-dynamic-cc input_for_cc.bc -o instrumented_bin
# Run the instrumented binary
./instrumented_bin
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

If you are interested in a more introductory example for code instrumentation,
you may want experiment with [**InjectFuncCall**](#injectfunccall) first.

## Mixed Boolean Arithmetic Transformations
These passes implement [mixed
boolean arithmetic](https://tel.archives-ouvertes.fr/tel-01623849/document)
transformations. Similar transformation are often used in code obfuscation (you
may also know them from [Hacker's
Delight](https://www.amazon.co.uk/Hackers-Delight-Henry-S-Warren/dp/0201914654))
and are a great illustration of what and how LLVM passes can be used for.

### **MBASub**
The **MBASub** pass implements this rather basic expression:
```
a - b == (a + ~b) + 1
```
Basically, it replaces all instances of integer `sub` according to the above
formula. The corresponding LIT tests verify that both the formula  and that the
implementation are correct. You can run this pass as follows:
```bash
export LLVM_DIR=<installation/dir/of/llvm/9>
$LLVM_DIR/bin/clang -emit-llvm -S inputs/input_for_mba_sub.c -o input_for_sub.ll
$LLVM_DIR/bin/opt -load <build_dir>/lib/libMBASub.so -legacy-mba-sub input_for_sub.ll -o out.ll
```

### **MBAAdd**
The **MBAAdd** pass implements a slightly more involved formula that is only
valid for 8 bit integers:
```
a + b == (((a ^ b) + 2 * (a & b)) * 39 + 23) * 151 + 111
```
Similarly to `MBASub`, it replaces all instances of integer `add` according to
the above identity, but only for 8-bit integers. The LIT tests verify that both
the formula and the implementation are correct. You can run **MBAAdd** like this:
```bash
export LLVM_DIR=<installation/dir/of/llvm/9>
$LLVM_DIR/bin/clang -O1 -emit-llvm -S inputs/input_for_mba.c -o input_for_mba.ll
$LLVM_DIR/bin/opt -load <build_dir>/lib/libMBAAdd.so -legacy-mba-add input_for_mba.ll -o out.ll
```
You can also specify the level of _obfuscation_ on a scale of `0.0` to `1.0`, with
`0` corresponding to no obfuscation and `1` meaning that all `add` instructions
are to be replaced with `(((a ^ b) + 2 * (a & b)) * 39 + 23) * 151 + 111`, e.g.:
```bash
$LLVM_DIR/bin/opt -load <build_dir>/lib/libMBAAdd.so -legacy-mba-add -mba-ratio=0.3 inputs/input_for_mba.c -o out.ll
```

## Reachable Integer Values (**RIV**)
For each basic block in a module, **RIV** calculates the reachable integer
values (i.e.  values that can be used in the particular basic block).  There
are a few LIT tests that verify that indeed this is correct. You can run this
pass as follows:
```bash
export LLVM_DIR=<installation/dir/of/llvm/9>
$LLVM_DIR/bin/opt -load <build_dir>/lib/libRIV.so -riv inputs/input_for_riv.c
```

Note that this pass, unlike previous passes, will produce information
only about the IR representation of the original module. It won't be very
useful if trying to understand the original C or C++ input file.

## Duplicate Basic Blocks (**DuplicateBB**)
This pass will duplicate all basic blocks in a module, with the exception of
basic blocks for which there are no reachable integer values (identified
through the **RIV** pass). An example of such a basic block is the entry block
in a function that:
* takes no arguments and
* is embedded in a module that defines no global values.

This pass depends on the **RIV** pass, hence you need to load it too in order
for **DuplicateBB** to work:
```bash
export LLVM_DIR=<installation/dir/of/llvm/9>
$LLVM_DIR/bin/opt -load <build_dir>/lib/libRIV.so -load <build_dir>/lib/libDuplicateBB.so -riv inputs/input_for_duplicate_bb.c
```
Basic blocks are duplicated by inserting an `if-then-else` construct and
cloning all the instructions (with the exception of [PHI
nodes](https://en.wikipedia.org/wiki/Static_single_assignment_form)) into the
new blocks.

Debugging
==========
Before running a debugger, you may want to analyze the output from
[LLVM_DEBUG](http://llvm.org/docs/ProgrammersManual.html#the-llvm-debug-macro-and-debug-option)
and
[STATISTIC](http://llvm.org/docs/ProgrammersManual.html#the-statistic-class-stats-option)
macros. For example, for **MBAAdd**:
```bash
export LLVM_DIR=<installation/dir/of/llvm/9>
$LLVM_DIR/bin/clang -emit-llvm -S -O1 inputs/input_for_mba.c -o input_for_mba.ll
$LLVM_DIR/bin/opt -load-pass-plugin <build_dir>/lib/libMBAAdd.dylib -passes=mba-add input_for_mba.ll -debug-only=mba-add -stats -o out.ll
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
be sufficient to understand what might be going wrong.

For tricker issues just use a debugger. Below I demonstrate how to debug
[**MBAAdd**](#mbaadd). More specifically, how to set up a breakpoint on entry
to `MBAAdd::run`. Hopefully that will be sufficient for you to start.

## Mac OS X
The default debugger on OS X is [LLDB](http://lldb.llvm.org). You will
normally use it like this:
```bash
export LLVM_DIR=<installation/dir/of/llvm/9>
$LLVM_DIR/bin/clang -emit-llvm -S -O1 inputs/input_for_mba.c -o input_for_mba.ll
lldb -- $LLVM_DIR/bin/opt -load-pass-plugin <build_dir>/lib/libMBAAdd.dylib -passes=mba-add input_for_mba.ll -o out.ll
(lldb) breakpoint set --name MBAAdd::run
(lldb) process launch
```
or, equivalently, by using LLDBs aliases:
```bash
export LLVM_DIR=<installation/dir/of/llvm/9>
$LLVM_DIR/bin/clang -emit-llvm -S -O1 inputs/input_for_mba.c -o input_for_mba.ll
lldb -- $LLVM_DIR/bin/opt -load-pass-plugin <build_dir>/lib/libMBAAdd.dylib -passes=mba-add input_for_mba.ll -o out.ll
(lldb) b MBAAdd::run
(lldb) r
```
At this point, LLDB should break at the entry to `MBAAdd::run`.

## Ubuntu
On most Linux systems, [GDB](https://www.gnu.org/software/gdb/) is the most
popular debugger. A typical session will look like this:
```bash
export LLVM_DIR=<installation/dir/of/llvm/9>
$LLVM_DIR/bin/clang -emit-llvm -S -O1 inputs/input_for_mba.c -o input_for_mba.ll
gdb --args $LLVM_DIR/bin/opt -load-pass-plugin <build_dir>/lib/libMBAAdd.so -passes=mba-add input_for_mba.ll -o out.ll
(gdb) b MBAAdd.cpp:MBAAdd::run
(gdb) r
```
At this point, GDB should break at the entry to `MBAAdd::run`.

About Pass Managers in LLVM
===========================
LLVM is a quite complex project (to put it mildly) and passes lay at its
center - this is true for any [multi-pass
compiler](https://en.wikipedia.org/wiki/Multi-pass_compiler<Paste>). In order
to manage the passes, a compiler needs a pass manager. LLVM currently enjoys
not one, but two pass manager. This is important, because depending on which
pass manager you decide to use, the implementation (and in particular pass
registration) will look slightly differently. I have tried my best to make the
distinction in the source code very clear.

## Overview of Pass Managers in LLVM
As mentioned earlier, there are two pass managers in LLVM:
* _Legacy Pass Manager_ which currently is the default pass manager
	* It is implemented in the _legacy_ namespace
	* It is very well [documented](http://llvm.org/docs/WritingAnLLVMPass.html)
		(more specifically, writing and registering a pass withing the Legacy PM is
		very well documented)
* _New Pass Manager_ aka [_Pass Manager_](https://github.com/llvm-mirror/llvm/blob/ff8c1be17aa3ba7bacb1ef7dcdbecf05d5ab4eb7/include/llvm/IR/PassManager.h#L458) (that's how it's referred to in the code base)
	* I understand that it is [soon to become](http://lists.llvm.org/pipermail/llvm-dev/2019-August/134326.html) the default pass manager in LLVM
	* The source code is very throughly commented, but otherwise I am only aware of
		this great [blog series](https://medium.com/@mshockwave/writing-llvm-pass-in-2018-preface-6b90fa67ae82) by Min-Yih Hsu.

The best approach is to implement your passes for both pass managers.
Fortunately, once you have an implementation that works for one of them, it's
relatively straightforward to extend it so that it works for the other one as
well. All passes in LLVM provide an interface for both and that's what I've
been trying to achieve here as well.

## New vs Legacy PM When Running Opt
**MBAAdd** implements interface for both pass managers. This is how you will
use it via the legacy pass manager:
```bash
$LLVM_DIR/bin/opt -load <build_dir>/lib/libMBAAdd.so -legacy-mba-add input_for_mba.ll -o out.ll
```

And this is how you run it with the new pass manager:
```bash
$LLVM_DIR/bin/opt -load-pass-plugin <build_dir>/lib/libMBAAdd.so -passes=mba-add input_for_mba.ll -o out.ll
```
There are two differences:
* the way you load your plugins: `-load` vs `-load-pass-plugin`
* the way you specify which pass/plugin to run: `-legacy-mba-add` vs
  `-passes=mba-add`

The command line option is different because with the legacy pass manager you
_register_ a new command line option with **opt** and with the new pass manager
you just define the pass pipeline (via `-passes=`).

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
this project. The implementations available here reflect what **I** (aka
banach-space) found most challenging while studying them.

I also want to thank Min-Yih Hsu for his [blog
series](https://medium.com/@mshockwave/writing-llvm-pass-in-2018-preface-6b90fa67ae82)
_"Writing LLVM Pass in 2018"_. It was invaluable in understanding how the new
pass manager works and how to use it. Last, but not least I am very grateful to
[Nick Sunmer](https://www.cs.sfu.ca/~wsumner/index.html) (e.g.
[llvm-demo](https://github.com/nsumner/llvm-demo)) and [Mike
Shah](http://www.mshah.io) (see Mike's Fosdem 2018
[talk](http://www.mshah.io/fosdem18.html)) for sharing their knowledge online.
I have learnt a great deal from it, thank you! I always look-up to those of us
brave and bright enough to work in academia - thank you for driving the
education and research forward!

## References
Below is a list of LLVM resources available outside the official online
documentation that I have found very helpful. Where possible, the items are sorted by
date.

* **LLVM IR**
  *  _”LLVM IR Tutorial-Phis,GEPs and other things, ohmy!”_, V.Bridgers, F.
Piovezan, EuroLLVM, ([slides](https://llvm.org/devmtg/2019-04/slides/Tutorial-Bridgers-LLVM_IR_tutorial.pdf),
  [video](https://www.youtube.com/watch?v=m8G_S5LwlTo&feature=youtu.be))
  * _"Mapping High Level Constructs to LLVM IR"_, M. Rodler ([link](https://mapping-high-level-constructs-to-llvm-ir.readthedocs.io/en/latest/))
* **Legacy vs New Pass Manager**
  * _"New PM: taming a custom pipeline of Falcon JIT"_, F. Sergeev, EuroLLVM 2018
    ([slides](http://llvm.org/devmtg/2018-04/slides/Sergeev-Taming%20a%20custom%20pipeline%20of%20Falcon%20JIT.pdf),
     [video](https://www.youtube.com/watch?v=6X12D46sRFw))
  * _"The LLVM Pass Manager Part 2"_, Ch. Carruth, LLVM Dev Meeting 2014
    ([slides](https://llvm.org/devmtg/2014-10/Slides/Carruth-TheLLVMPassManagerPart2.pdf),
     [video](http://web.archive.org/web/20160718071630/http://llvm.org/devmtg/2014-10/Videos/The%20LLVM%20Pass%20Manager%20Part%202-720.mov))a
  * _”Passes in LLVM, Part 1”_, Ch. Carruth, EuroLLVM 2014 ([slides](https://llvm.org/devmtg/2014-04/PDFs/Talks/Passes.pdf), [video](https://www.youtube.com/watch?v=rY02LT08-J8))
* **Examples in LLVM**
  * Examples in LLVM source tree in
    [llvm/examples/IRTransforms/](https://github.com/llvm/llvm-project/tree/bf142fc43347d8a35a71f46f7dda7e2a0a992e0d/llvm/examples/IRTransforms).
    This was recently added in the following commit:

```
commit 7d0b1d77b3d4d47df477519fd1bf099b3df6f899
Author: Florian Hahn <flo@fhahn.com>
Date:   Tue Nov 12 14:06:12 2019 +0000

[Examples] Add IRTransformations directory to examples.
```
* **LLVM Pass Development**
  * _"Getting Started With LLVM: Basics "_, J. Paquette, F. Hahn, LLVM Dev Meeting 2019 (not yet uploaded)
  * _"Writing an LLVM Pass: 101"_, A. Warzyński, LLVM Dev Meeting 2019 (not yet uploaded)
  * _"Writing LLVM Pass in 2018"_, Min-Yih Hsu, [blog series](https://medium.com/@mshockwave/writing-llvm-pass-in-2018-preface-6b90fa67ae82)
  * _"Building, Testing and Debugging a Simple out-of-tree LLVM Pass"_ Serge Guelton, Adrien Guinet, LLVM Dev Meeting 2015 ([slides](https://llvm.org/devmtg/2015-10/slides/GueltonGuinet-BuildingTestingDebuggingASimpleOutOfTreePass.pdf), [video](https://www.youtube.com/watch?v=BnlG-owSVTk&index=8&list=PL_R5A0lGi1AA4Lv2bBFSwhgDaHvvpVU21))
* **LLVM Based Tools Development**
  * _"Introduction to LLVM"_, M. Shah, Fosdem 2018, [link](http://www.mshah.io/fosdem18.html)
  *  [llvm-demo](https://github.com/nsumner/llvm-demo), by N Sumner
  * _"Building an LLVM-based tool. Lessons learned"_, A. Denisov, [blog post](https://lowlevelbits.org/building-an-llvm-based-tool.-lessons-learned/), [video](https://www.youtube.com/watch?reload=9&v=Yvj4G9B6pcU)

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
