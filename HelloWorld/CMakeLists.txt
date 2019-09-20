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

# Incorporate the CMake features provided by LLVM:
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

# TODO Remove once LLVM moves to C++14
option(LLVM_ENABLE_CXX1Y  "Enable C++14" ON)

# Now set the LLVM header and library paths:
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Workaround for Travis
OPTION(LT_TRAVIS_DISABLE "Option description" OFF)

if(LT_TRAVIS_DISABLE)
  add_definitions(-DLT_TRAVIS_DISABLE)
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
