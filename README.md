llvm-tutor
=========
[![Build Status](https://travis-ci.org/banach-space/llvm-tutor.svg?branch=master)](https://travis-ci.org/banach-space/llvm-tutor)

Example LLVM passes - based on **LLVM 8**

**llvm-tutor** (aka **lt**) is a collection of self-contained reference LLVM
passes developed as a tutorial. It targets novice and aspiring LLVM developers.
It strives to be:
  * **Complete** - includes `CMake` build scripts, LIT tests and CI set-up
  * **Out of source** - builds against a binary LLVM installation (no need to
    build LLVM from sources)
  * **Modern** - based on the latest version of LLVM (and updated with every release)

There is plenty of comments in source files, build scripts and tests - I hope
that you'll find them helpful.

tl;dr
-----
The best place to start is the `static-cc` pass implemented in
`StaticCallCounter.cpp`. Build it first:
```bash
cmake G Ninja -DLT_LLVM_INSTALL_DIR=<installation/dir/of/llvm/8> <source/dir/llvm/tutor>
ninja
```
and then run it through `opt`:
```bash
opt -load <build_dir>/lib/libCallCounter.so --static-cc -analyze <bitcode/file/to/analyze>
```

Status
------
**WORK IN PROGRESS**.

Expect typos, but code builds fine and all tests pass. This is still early
stages, so I might be moving the code around or renaming things. More
functionality to come.

Available Passes
-----------------
The list of implemented passes:
   * **CallCounter** - direct call counter (static and dynamic calls, pure
     analysis and instrumentation pass, parametrisable)
   * **MBAAdd** - obfuscation through Mixed Boolean Arithmetic (transformation
     pass, paramterisable)
   * **RIV** - reachable integer values (analysis pass)
   * **DuplicateBB** - reachable integer values (transformation pass,
     parametrisable)

For more details go to [usage](#usage).

Requirements
------------
These are the only requirement for **llvm-tutor**:
  * development version of LLVM-8
  * C++ compiler that supports C++14
  * CMake 3.4.3 or higher

Keep in mind that you only need the components required for developing
LLVM-based projects, e.g. header files, development libraries, CMake scripts.
Installing `clang-8` (and other LLVM-based tools) won't hurt, but is neither
required nor sufficient. There are additional requirements to be able to run
tests, which are documented [here](#test_requirements).

## Obtaining LLVM-8
There are two options:
* **build from sources** (works regardless of the operating system, but can
  be slow and tricky to debug if things don't go according to plan)
* **download pre-compilled packages** (very quick and easy, but works only on
  systems for which there are LLVM package maintainers).

The packages available for Ubuntu and Mac OS X are sufficient. Windows users
will have to build from sources.

### Installing on Ubuntu
If you're using `Ubuntu` then install `llvm-8-dev` (other dependencies will be
_pulled_ automatically). Note that this very recent version of LLVM is not yet
available in the official repositories. On `Ubuntu Xenial`, you can [install
LLVM](https://blog.kowalczyk.info/article/k/how-to-install-latest-clang-6.0-on-ubuntu-16.04-xenial-wsl.html)
from the official [repository](http://apt.llvm.org/):
```bash
$ wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
$ sudo apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-8.0 main"
$ sudo apt-get update
$ sudo apt-get install -y llvm-8-dev
```
Note that this won't install the dependencies required for [testing](#testing).
Ubuntu maintainers have stopped those them starting with LLVM-3.8 (more info
[here](https://bugs.launchpad.net/ubuntu/+source/llvm-toolchain-3.8/+bug/1700630)).
In order to run LIT tests you will have to build the dependencies from sources.
However, you don't need them to be able to build and run the passes developed
here.

### Installing on Mac OS X
On Darwin you can install LLVM-8 with [Homebrew](https://brew.sh/):
```bash
$ brew install llvm@8
```
This will install all the required header files, libraries and binaries in
`/usr/local/opt/llvm/`. Currently this will also install the binaries required
for testing.

### Building From Sources
You can also choose to [build LLVM](https://llvm.org/docs/CMake.html) from
sources. It might be required if there are no precompiled packages for your
OS. This will work on Linux, OS X and Windows:
```bash
cd <llvm-project/root/dir>/../
git clone https://github.com/llvm/llvm-project.git
cd llvm-project
git checkout release/8.x
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=X86 <llvm-project/root/dir>/llvm/
cmake --build .
```
These steps are not optimal. Please refer to the official documentaion for more
hints.

Platform Support
----------------
This project is regularly being tested on Linux and Mac OS X. It should build
and run seamlessly on Windows, though some tweaks in CMake might be required.
It is regularly tested against the following configurations (extracted from the
CI files: `.travis.yml`):
  * Linux Ubuntu 16.04
  * Mac OS X 10.14.4

Please refer to the CI logs (links at the top of the page) for reference
setups.

Build Instructions
------------------
It is assumed that **llvm-tutor** will be built in `<build-dir>` and that the
top-level source directory is `<source-dir>`.

You can build all the examples as follows:
```bash
cd <build-dir>
cmake -DLT_LLVM_INSTALL_DIR=<either_build_or_installation_dir_of_llvm_8>  <source_dir>
make
```

The `LT_LLVM_INSTALL_DIR` variable should be set to either the installation or
build directory of LLVM-8. It is used to locate the corresponding
`LLVMConfig.cmake` script.

Usage
-----
Once you've [built](#build-instructions) this project, you can experiment with
every pass separately.

### Counting Function Calls (**CallCounter**)
The `lt-cc` executable implements two (rather) basic direct call counters:
static, and dynamic. You can test it with one of the provided examples, e.g.:
```bash
clang  -emit-llvm -c <source_dir>/test/input_for_cc.c
<build_dir>/bin/lt-cc -static input_for_cc.bc
```

or, for dynamic call analysis:
```bash
<build_dir>/bin/lt-cc  -dynamic  input_for_cc.bc -o input_for_cc
./input_for_cc
```

### Obfuscation through mixed boolean arithmetic (**MBAAdd**)
The `mba-add` pass implements a very basic [obfuscation with mixed
boolean-arithmetic](https://tel.archives-ouvertes.fr/tel-01623849/document)
expression:
```
a + b == (a ^ b) + 2 * (a & b)
```
Basically, it replaces all instances of addition according to the above
formula. There are a few LIT tests that verify that indeed this is correct. You
can run this pass as follows:
```bash
opt -load <build_dir>/lib/libMBAAdd.so --mba inputs/input_for_mba.c -o MBAAdd_mod.ll
```
You can also specify the level of obfuscation on a scale of `0` to `1`, with
`0` corresponding to no obfuscation and `1` meaning that all `add` instructions
are to be replaced with `(a ^ b) + 2 * (a & b)`, e.g.:
```bash
opt -load <build_dir>/lib/libMBAAdd.so -mba -mba-ration=0.3 inputs/input_for_mba.c -o MBAAdd_mod.ll
```

### Reachable Integer Values (**RIV**)
For each basic block in a module, calculates the reachable integer values (i.e.
values that can be used in the particular basic block).  There are a few LIT
tests that verify that indeed this is correct. You can run this pass as
follows:
```bash
opt -load <build_dir>/lib/libRVI.so -rvi inputs/input_for_rvi.c
```

Note that this pass, unlike previous ones, is only really useful when analysing
the underlying IR representation of the input module.

### Duplicate Basic Blocks (**DuplicateBB**)
This pass will duplicate all basic blocks in a module, with the exception of
basic blocks for which there are no reachable integer values (identified
through the `RVI` pass). This should only exclude the entry block in functions
that take no arguments and which are embedded in modules with no global values.
Duplicates (almost) every basic block in a module. This pass depends on the
`RVI` pass, hence you need to load to modules in order to run it:
```bash
opt -load <build_dir>/lib/libRVI.so -load <build_dir>/lib/libDuplicateBB.so -rvi inputs/input_for_duplicate_bb.c
```
Basic blocks are duplicated by inserting an `if-then-else` construct and
cloning all the instructions (with the exception of PHI nodes) into the new
blocks.

Testing
-------
There's a handful of LIT tests added to this project. These tests are meant to
prevent any future breakage in **llvm-tutor**. They also serve as reference for
setting them up in the future (i.e. you will also find here LIT configuration
scripts which are part of the required set-up).

### Test Requirements

Before running the tests you need to make sure that you have the following
tools installed:
  * [**FileCheck**](https://llvm.org/docs/CommandGuide/lit.html) (LIT
    requirement, it's used to check whether tests generate the expected output)
  * [**opt**](http://llvm.org/docs/CommandGuide/opt.html) (the modular LLVM
    optimizer and analyzer, used to load and run passes from shared objects)
  * [**lit**](https://llvm.org/docs/CommandGuide/lit.html) (LLVM tool for executing
    the tests)

Neither of the requirements are satisfied on Ubuntu Xenial - sadly there are no
packages that would provide `opt` or `FileCheck` for LLVM-8.0. Although older
versions are available (e.g. bundled with LLVM-4.0), this project has been
developed against LLVM-8.0 and ideally you want a matching version of both
`opt` and `FileCheck`.

### Running The Tests
First, you will have to specify the location of the tools required for
running the tests (e.g. `FileCheck`). This is done when configuring the
project:

```bash
$ cmake -DLT_LLVM_INSTALL_DIR=<either_build_or_installation_dir>
-DLT_LIT_TOOLS_DIR=<location_of_filecheck> <source_dir>
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
