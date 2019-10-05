cmake_minimum_required(VERSION 3.4.3)

# LLVM is soon to move to C++14
set(CMAKE_CXX_STANDARD 14 CACHE STRING "")

#===============================================================================
# 1. LLVM-TUTOR CONFIGURATION
#===============================================================================
# Set this to a valid LLVM installation dir
set(LT_LLVM_INSTALL_DIR "" CACHE PATH "LLVM installation directory")

# Add path to LLVMConfig.cmake (so that find_package can find LLVM)
set(LT_LLVM_CMAKE_CONFIG_DIR "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/")
list(APPEND CMAKE_PREFIX_PATH "${LT_LLVM_CMAKE_CONFIG_DIR}")

#===============================================================================
# 2. LLVM-CONFIGURATION
#===============================================================================
find_package(LLVM REQUIRED CONFIG)

# HelloWorld includes headers from LLVM - update the include path accordingly
include_directories(${LLVM_INCLUDE_DIRS})

# Registration with any of the existing pipelines causes 'opt' to seg-fault
# when building against LLVM installed via HomeBrew. Similar issue is discussed
# here:  https://github.com/sampsyo/llvm-pass-skeleton/issues/7
# I have not experienced this when building from sources.
OPTION(LT_LEGACY_SKIP_PIPELINE_REGISTRATION "Don't register with any of the existing pipelines" OFF)
if(LT_LEGACY_SKIP_PIPELINE_REGISTRATION)
  add_definitions(-DLT_LEGACY_SKIP_PIPELINE_REGISTRATION)
endif()

# LLVM is normally built without RTTI and there has to be consistency between
# LLVm and the plugins.
if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

#===============================================================================
# 3. ADD THE TARGET
#===============================================================================
add_library(HelloWorld SHARED HelloWorld.cpp)
# Allow undefined symbols in shared objects on Darwin (this is the default
# behaviour on Linux)
target_link_libraries(HelloWorld
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")
