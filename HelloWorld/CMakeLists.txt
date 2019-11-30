cmake_minimum_required(VERSION 3.4.3)

#===============================================================================
# 1. LOAD LLVM CONFIGURATION
#===============================================================================
# Set this to a valid LLVM installation dir
set(LT_LLVM_INSTALL_DIR "" CACHE PATH "LLVM installation directory")

# Add the location of LLVMConfig.cmake to CMake search paths (so that
# find_package can locate it)
list(APPEND CMAKE_PREFIX_PATH "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/")

find_package(LLVM 9.0.0 REQUIRED CONFIG)

# HelloWorld includes headers from LLVM - update the include paths accordingly
include_directories(${LLVM_INCLUDE_DIRS})

#===============================================================================
# 2. LLVM-TUTOR BUILD CONFIGURATION
#===============================================================================
# Use the same C++ standard as LLVM does
set(CMAKE_CXX_STANDARD 14 CACHE STRING "")

# LLVM is normally built without RTTI. Be consistent with that.
if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

# For LLVM installed via HomeBrew (on Darwin), 'opt' seg-faults when an
# out-of-tree pass is registered with an existing optimisation pipeline. I have
# not experienced this when building from sources. Similar issue is discussed
# here:
#   https://github.com/sampsyo/llvm-pass-skeleton/issues/7
# The following flag is used as a workaround for that - it guards the
# problematic code (which otherwise works fine on Linux and on Darwin when
# building LLVM from sources).
OPTION(LT_LEGACY_SKIP_PIPELINE_REGISTRATION "Don't register with any of the existing pipelines" OFF)
if(LT_LEGACY_SKIP_PIPELINE_REGISTRATION)
  add_definitions(-DLT_LEGACY_SKIP_PIPELINE_REGISTRATION)
endif()

#===============================================================================
# 3. ADD THE TARGET
#===============================================================================
add_library(HelloWorld SHARED HelloWorld.cpp)

# Allow undefined symbols in shared objects on Darwin (this is the default
# behaviour on Linux)
target_link_libraries(HelloWorld
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")
