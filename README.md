llvm-tutor
=========
[![Build Status](https://travis-ci.org/banach-space/llvm-tutor.svg?branch=master)](https://travis-ci.org/banach-space/llvm-tutor)

Example LLVM passes - based on **LLVM 9**

**llvm-tutor** (aka **lt**) is a collection of self-contained reference LLVM
passes. It's a tutorial that targets novice and aspiring LLVM developers.
Key features:
  * **Complete** - includes `CMake` build scripts, LIT tests and CI set-up
  * **Out of source** - builds against a binary LLVM installation (no need to
    build LLVM from sources)
  * **Modern** - based on the latest version of LLVM (and updated with every release)

The source files contain comments that will guide you through the
implementation and the LIT tests verify that each pass works as expected. This
document explains how to get started.

### Table of Contents
[TL;DR](#tldr-the-reference-example) **::**
[Development Environment](#development-environment) **::**
[Build Instructions](#build-instructions) **::**
[Passes](#passes) **::**
[Testing](#testing) **::**
[Credits & References](#credits) **::**
[License](#license)


TL;DR (the reference example)
-----------------------------
**HelloWorld** implemented in
[HelloWorld.cpp](https://github.com/banach-space/llvm-tutor/blob/master/HelloWorld/HelloWorld.cpp)
is a basic, self-contained example. It's a good starting point if all you want is a
reference example. The corresponding
[CMakeLists.txt](https://github.com/banach-space/llvm-tutor/blob/master/HelloWorld/CMakeLists.txt)
implements the minimum set-up for an out-of-source pass.

You can build **HelloWorld** like this:
```bash
mkdir build
cd build
cmake -DLT_LLVM_INSTALL_DIR=<installation/dir/of/llvm/9> <source/dir/llvm/tutor>/HelloWorld/
make
```
Before you can test it, you need to prepare an input file:
```bash
# Generate an llvm test file
clang -S -emit-llvm <source/dir/llvm/tutor/>inputs/input_for_hello.c> -o
input_for_hello.ll
```
Finally, run it with `opt`:
```bash
# Run the pass on the llvm file
opt -load libHelloWorld.dylib -hello-world -disable-output input_for_hello.ll
# The expected output
Visiting: foo (takes 1 args)
Visiting: bar (takes 2 args)
Visiting: fez (takes 3 args)
Visiting: main (takes 2 args)
```
The `disable-output` flag will prevent `opt` from printing the output binary.
The `HelloWorld` pass doesn't modify the input module, so that would just
unnecessarily clutter the output.

Development Environment
-----------------------
### Platform Support And Requirements
This project is regularly tested on **Linux 16.04** and **Mac OS X 10.14.4**
(see
[\.travis.yml](https://github.com/banach-space/llvm-tutor/blob/master/.travis.yml) for more details).
Support for Windows is *work-in-progress*. In order to build **llvm-tutor** you will need:
  * the development version of LLVM 9
  * a C++ compiler that supports C++14
  * CMake 3.4.3 or higher

You will also need [opt](http://llvm.org/docs/CommandGuide/opt.html) to load
and run the passes. It's normally bundled with LLVM development packages.

Installing `clang-9` (and other LLVM-based tools) won't hurt, but is neither
required nor sufficient - you need the headers, libraries and `CMake` scripts
that come with the development packages. There are additional requirements for
tests documented [here](#test_requirements).

### Obtaining LLVM-9
There are two options:
* **build from sources** (works regardless of the operating system, but can
  be slow and tricky to debug if something doesn't work)
* **download pre-compilled packages** (very quick and easy, but works only on
  systems for which there are LLVM package maintainers).

The packages available for Ubuntu and Mac OS X are sufficient. Windows users
will have to build from sources.

#### Installing on Ubuntu
If you're using `Ubuntu` then install `llvm-9-dev` (other dependencies will be
_pulled_ automatically). Note that this very recent version of LLVM is not yet
available in the official repositories. On `Ubuntu Bionic`, you can [install
LLVM](https://blog.kowalczyk.info/article/k/how-to-install-latest-clang-6.0-on-ubuntu-16.04-xenial-wsl.html)
from the official [repository](http://apt.llvm.org/):
```bash
$ wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
$ sudo apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-9.0 main"
$ sudo apt-get update
$ sudo apt-get install -y llvm-9-dev
```
Note that this won't install the dependencies required for [testing](#testing).
Ubuntu maintainers have stopped those them starting with LLVM-3.8 (more info
[here](https://bugs.launchpad.net/ubuntu/+source/llvm-toolchain-3.8/+bug/1700630)).
In order to run LIT tests you will have to build the dependencies from sources.
However, you don't need them to be able to build and run the passes developed
here.

#### Installing on Mac OS X
On Darwin you can install LLVM 9 with [Homebrew](https://brew.sh/):
```bash
$ brew install llvm@9
```
This will install all the required header files, libraries and binaries in
`/usr/local/opt/llvm/`. Currently this will also install the binaries required
for testing.

#### Building From Sources
You can also choose to [build LLVM](https://llvm.org/docs/CMake.html) from
sources. It might be required if there are no precompiled packages for your
OS. This will work on Linux, OS X and Windows:
```bash
git clone https://github.com/llvm/llvm-project.git
cd llvm-project
git checkout release/9.x
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=X86 <llvm-project/root/dir>/llvm/
cmake --build .
```
Although sufficient, these steps are not optimal. Please refer to the official
documentation for more hints.

Build Instructions
------------------
You can build all the examples as follows:
```bash
cd <build/dir>
cmake -DLT_LLVM_INSTALL_DIR=<installation/dir/of/llvm/9> <source/dir/llvm/tutor>
make
```

The `LT_LLVM_INSTALL_DIR` variable should be set to the root of either the
installation or build directory of LLVM 9. It is used to locate the
corresponding `LLVMConfig.cmake` script that sets the include and library
paths.

Passes
------
   * [**HelloWorld**](#tldr-the-reference-example) - prints the functions in
     the input module and prints the number of arguments for each
   * [**StaticCallCounter**](#count-compile-time-function-calls-staticcallcounter) - counts
     direct function calls at compile time (only static calls, pure analysis
     pass)
   * [**DynamicCallCounter**](#count-run-time-function-calls-dynamiccallcounter)
     \- counts direct function calls at run-time ( analysis + instrumentation
       pass)
   * [**MBASub**](#mba-sub) - code transformation for integer `sub`
     instructions (transformation pass, parametrisable)
   * [**MBAAdd**](#mba-add) - code transformation for 8-bit integer `add`
     instructions (transformation pass, parametrisable)
   * [**RIV**](#reachable-integer-values-riv) - finds reachable integer values
     for each basic block (analysis pass)
   * [**DuplicateBB**](#duplicate-basic-blocks-duplicatebb) - duplicates basic
     blocks, requires **RIV** analysis results (transformation pass,
     parametrisable)

Once you've [built](#build-instructions) this project, you can experiment with
every pass separately. I assume that you have `clang` and `opt` available in
your `PATH`. All passes work with LLVM files. You can generate one like this:
```bash
# Textual form
clang  -emit-llvm input.c -S -o out.ll
# Binary/bit-code form
clang  -emit-llvm input.c -o out.bc
```
It doesn't matter whether your choose the textual or binary form, but obviously
the former is more human-friendly. The `HelloWorld` pass is described in
[TL;DR](#tldr-the-reference-example). All other passes are described below.

### Count Compile Time Function Calls (**StaticCallCounter**)
`StaticCallCounter` will count the number of function calls in the input
LLVM file that are visible during the compilation (i.e. if a function is
called within a loop, that counts as one call). Only direct function calls are
considered (TODO: Expand).
```bash
# Generate an LLVM file to analyze
clang  -emit-llvm -c <source_dir>/test/input_for_cc.c -o input_for_cc.bc
# Run the pass through opt
opt -load <build_dir>/lib/libStaticCallCounter.dylib -static-cc -analyze input_for_cc.bc
```
The `static` executable is a command line wrapper that allows you to run
`StaticCallCounter` without the need for `opt`:
```bash
<build_dir>/bin/static input_for_cc.bc
```

### Count Run-Time Function Calls (**DynamicCallCounter**)
`DynamicCallCounter` will count the number of run-time function calls. It does
so by instrumenting the input LLVM file - it injects call-counting code that is
executed every time a function is called. Although this pass _analyses_
function calls, as it does so by modifying the input file so it's a
transformation pass.  You can test it with one of the provided examples, e.g.:
```bash
# Generate an LLVM file to analyze
clang  -emit-llvm -c <source_dir>/test/input_for_cc.c -o input_for_cc.bc
# Instrument the input file first
<build_dir>/bin/dynamic  -dynamic  input_for_cc.bc -o instrumented_bin
# Now run the instrumented binary
./instrumented_bin
```

### Mixed Boolean Arithmetic Transformations
These passes implement [mixed
boolean-arithmetic](https://tel.archives-ouvertes.fr/tel-01623849/document)
transformations. Similar transformation are often used in code obfuscation (you
may also know them from [Hacker's
Delight](https://www.amazon.co.uk/Hackers-Delight-Henry-S-Warren/dp/0201914654))
and are a great illustration of what and how LLVM passes can be used for.

#### **MBASub**
The `MBASub` pass implements this rather basic expression:
```
a - b == (a + ~b) + 1
```
Basically, it replaces all instances of integer `sub` according to the above
formula. There are a few LIT tests that verify that indeed this is formula and
the attached implementations are correct. You can run this pass as follows:
```bash
opt -load <build_dir>/lib/libMBASub.so -mba-sub inputs/input_for_sub.c -o out.ll
```

#### **MBAAdd**
The `MBAAdd` pass implements a slightly more involved formula (this is only
valid for 8 bit integers):
```
a + b == (((a ^ b) + 2 * (a & b)) * 39 + 23) * 151 + 111
```
Similarly to `MBASub`, it replaces all instances of integer `add`according to
the above formula, but only for 8-bit integers. The LIT tests verify that
both the formula and the implementations are correct. You can run it like this:
```bash
opt -load <build_dir>/lib/libMBAAdd.so -mba-add inputs/input_for_mba.c -o out.ll
```
You can also specify the level of _obfuscation_ on a scale of `0.0` to `1.0`, with
`0` corresponding to no obfuscation and `1` meaning that all `add` instructions
are to be replaced with `(((a ^ b) + 2 * (a & b)) * 39 + 23) * 151 + 111`, e.g.:
```bash
opt -load <build_dir>/lib/libMBAAdd.so -mba-add -mba-ratio=0.3 inputs/input_for_mba.c -o out.ll
```

### Reachable Integer Values (**RIV**)
For each basic block in a module, calculates the reachable integer values (i.e.
values that can be used in the particular basic block).  There are a few LIT
tests that verify that indeed this is correct. You can run this pass as
follows:
```bash
opt -load <build_dir>/lib/libRIV.so -riv inputs/input_for_riv.c
```

Note that this pass, unlike previous ones, is only really useful when analysing
the underlying IR representation of the input module.

### Duplicate Basic Blocks (**DuplicateBB**)
This pass will duplicate all basic blocks in a module, with the exception of
basic blocks for which there are no reachable integer values (identified
through the `RIV` pass). This should only exclude the entry block in functions
that take no arguments and which are embedded in modules with no global values.
Duplicates (almost) every basic block in a module. This pass depends on the
`RIV` pass, hence you need to load to modules in order to run it:
```bash
opt -load <build_dir>/lib/libRIV.so -load <build_dir>/lib/libDuplicateBB.so -riv inputs/input_for_duplicate_bb.c
```
Basic blocks are duplicated by inserting an `if-then-else` construct and
cloning all the instructions (with the exception of PHI nodes) into the new
blocks.

Testing
-------
There's a handful of LIT tests and LIT configuration scripts in
[test](https://github.com/banach-space/llvm-tutor/blob/master/test).

### Test Requirements
Before running the tests you need to make sure that you have the following
tools installed:
  * [**FileCheck**](https://llvm.org/docs/CommandGuide/lit.html) (LIT
    requirement, it's used to check whether tests generate the expected output)
  * [**opt**](http://llvm.org/docs/CommandGuide/opt.html) (the modular LLVM
    optimizer and analyzer, used to load and run passes from shared objects)
  * [**lit**](https://llvm.org/docs/CommandGuide/lit.html) (LLVM tool for executing
    the tests)

Neither of the requirements are satisfied on Ubuntu Bionic - sadly there are no
packages that would provide `opt` or `FileCheck` for LLVM 9. Although older
versions are available (e.g. bundled with LLVM-4.0), this project has been
developed against LLVM 9 and ideally you want a matching version of both
`opt` and `FileCheck`.

### Running The Tests
First, you will have to specify the location of the tools required for
running the tests (e.g. `FileCheck`). This is done when configuring the
project:

```bash
$ cmake -DLT_LLVM_INSTALL_DIR=<installation/dir/of/llvm/9>
-DLT_LIT_TOOLS_DIR=<location/of/filecheck> <source_dir>
```
Next, you can run the tests like this:
```bash
$ lit <build_dir>/test
```
Voilà! (well, assuming that `lit` is in your _path_).

### Wee disclaimer
The requirements for running LIT tests are _not_ met on Ubuntu (unless you
build LLVM from sources) and for this reason the CI is configured to skip tests
there. As far as Linux is concerned, I've only been able to run the tests on my
host. If you encounter any problems on your machine - please let me know and I
will try to fix that. The CI _does_ run the tests when building on Mac OS X.

Credits
-------
This is first and foremost community effort. This project wouldn't be possible
without the amazing LLVM [online documentation](http://llvm.org/docs/),
plethora of great comments in the source code, and the llvm-dev mailing list.
Thank you!

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
this projects. The implementations available here are based on the latest
release of LLVM's API and have been refactored and documented to reflect what
**I** (aka. banach-space) found most challenging while studying them. However,
everything that's impressive, interesting and intriguing about those examples
should be credited to Adrien and Serge. I take credit for the confusing,
awkward and counter-intuitive bits :-)

License
--------
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
