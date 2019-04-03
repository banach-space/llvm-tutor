llvm-tutor
=========
[![Build Status](https://travis-ci.org/banach-space/llvm-tutor.svg?branch=add_first_pass)](https://travis-ci.org/banach-space/llvm-tutor)

Example LLVM passes - based on LLVM-8

A bunch of rather basic and self-contained LLVM passes, some of which are
wrapped into standalone executables. I wrote these examples when I was learning
how to work with LLVM. In terms of functionality you won't find anything particularly
original here - similar examples can be found elsewhere online. What's new is
that:
  * there's a CI set-up for this project - you can be confident that the
    example build and work fine
  * there's plenty of comments - it should speed-up your learning process
  * it's based on the latest version of LLVM -  you won't get stuck
    trying to download/build some ancient version of it

I created this project out of frustration. Although there's a lot of great
content available online, you'll find that most of it (at least the ones I
came across) lacks the above elements.

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
```
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
```
$ cd <build-dir>
$ cmake -DLT_LLVM_INSTALL_DIR=<either_build_or_installation_dir>  <source_dir>
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
```
$ clang  -emit-llvm -c <source_dir>/test/example_1.c
$ <build_dir>/bin/lt-cc -static example_1.bc
```

or, for dynamic call analysis:
```
$ <build_dir>/bin/lt-cc  -dynamic  example_1.bc -o example_1
$ ./example_1
```

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
