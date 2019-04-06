llvm-tutor
=========
[![Build Status](https://travis-ci.org/banach-space/llvm-tutor.svg?branch=add_first_pass)](https://travis-ci.org/banach-space/llvm-tutor)

Example LLVM passes - based on LLVM-8

A bunch of rather basic and self-contained LLVM passes, some of which are
wrapped into standalone executables. I wrote these examples when I was learning
how to work with LLVM. In terms of functionality you won't find anything particularly
original here - similar examples can be found elsewhere online. What's new is
that:
  * there's a CI set-up for this project - you and me can be
    confident that the examples do build and work fine
  * there's plenty of comments - it should speed-up your learning process
  * it's based on the latest version of LLVM -  you won't get stuck
    trying to download/build some ancient version of it

I created this project out of frustration. Although there's a lot of great
content available online, you'll find that most of it (at least the ones I
came across) lacks the above elements.

tl;dr
-----
The best place to start is the `lt` pass implemented in
`StaticCallCounter.cpp`. Build it first:
```
$ cmake -DLT_LLVM_INSTALL_DIR=<either_build_or_installation_dir_of_llvm_8>  <source_dir>
```
and then run:
```
$ opt -load <build_dir>/lib/liblt-lib.so --lt -analyze <bitcode-file>
```

Status
------
A list of currently available passes:
   * direct call counter: static and dynamic calls

Requirements
------------
The main requirement for **llvm-tutor** is a development version of LLVM-8. If
you're using `Ubuntu` then that boils down to installing `llvm-8-dev` (other
dependencies will be pulled automatically). Note that this very recent version
of LLVM is not yet available in the official repositories. On `Ubuntu Xenial`,
you can
[install](https://blog.kowalczyk.info/article/k/how-to-install-latest-clang-6.0-on-ubuntu-16.04-xenial-wsl.html)
it from the official LLVM [repo](http://apt.llvm.org/):
```bash
$ wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
$ sudo apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-8.0 main"
$ sudo apt-get update
$ sudo apt-get install -y llvm-8-dev
```

Keep in mind that you only need the components required for developing
LLVM-based projects, e.g. header files, development libraries, CMake scripts.
Installing `clang-8` (and other LLVM-based tools) won't hurt, but is neither
required nor sufficient.

You can also choose to [build LLVM](https://llvm.org/docs/CMake.html) from
sources. It might be required if there are no precompiled packages for your
Linux distribution. Installing it is not necessary.

The `C++` standard for **llvm-tutor** was set-up to `C++14`, so make sure that
the compiler that you use supports it.

Platform Support
----------------
This project is currently only supported on Linux, though it should build and
run seamlessly on Mac OS X. Porting to Windows might require some tweaks in
CMake. It is regularly tested against the following configurations (extracted
from the CI files:
`.travis.yml`):
  * Linux Ubuntu 16.04 (GCC-7 and LLVM-7)

Locally I used GCC-8.2.1 and LLVM-7 for development (i.e. for compiling). Please
refer to the CI logs (links at the top of the page) for reference setups.

Build Instructions
------------------
It is assumed that **llvm-tutor** will be built in `<build-dir>` and that the
top-level source directory is `<source-dir>`.

You can build all the examples as follows:
```bash
$ cd <build-dir>
$ cmake -DLT_LLVM_INSTALL_DIR=<either_build_or_installation_dir_of_llvm_8>  <source_dir>
$ make
```

The `LT_LLVM_INSTALL_DIR` variable should be set to either the installation or
build directory of LLVM-8. It is used to locate the corresponding
`LLVMConfig.cmake` script.

Usage
-----
Once you've [built](#build-instructions) this project, you can verify every
pass individually.

### Counting Function Calls
The `lt-cc` executable implements two basic direct call counters: static, and
dynamic. You can test it with one of the provided examples, e.g.:
```bash
$ clang  -emit-llvm -c <source_dir>/test/example_1.c
$ <build_dir>/bin/lt-cc -static example_1.bc
```

or, for dynamic call analysis:
```bash
$ <build_dir>/bin/lt-cc  -dynamic  example_1.bc -o example_1
$ ./example_1
```

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
I haven't been able to set-up the tests in my CI (because Travis runs Ubuntu
Xenial on which the basic requirements are not met). This means that I've only
been able to run the tests on my host. If you encounter any problems on your
machine - please let me know and I will try to fix that.

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
