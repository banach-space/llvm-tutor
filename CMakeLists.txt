cmake_minimum_required(VERSION 3.6)
project(llvm-tutor)

set(PACKAGE_NAME lt-cc)
set(PACKAGE_VERSION 0.1)
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")

#===============================================================================
# 1. LLVM-TUTOR CONFIGURATION
# The key (and the only really required) thing here is getting a valid value
# for LT_LLVM_CMAKE_CONFIG_DIR
#===============================================================================
# Set this to a valid LLVM installation dir
set(LT_LLVM_INSTALL_DIR "" CACHE PATH "LLVM installation directory")

# Set this to a valid LLVM installation dir
set(LT_LIT_TOOLS_DIR "" CACHE PATH "The location of tools required for testing
with LIT ( FileCheck, not, count)")

# A bit of a sanity checking
set(LT_LLVM_INCLUDE_DIR "${LT_LLVM_INSTALL_DIR}/include/llvm")
if(NOT EXISTS "${LT_LLVM_INCLUDE_DIR}")
message(FATAL_ERROR
  " LT_LLVM_INSTALL_DIR (${LT_LLVM_INCLUDE_DIR}) is invalid.")
endif()

set(LT_LLVM_CMAKE_FILE "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/LLVMConfig.cmake")
if(NOT EXISTS "${LT_LLVM_CMAKE_FILE}")
message(FATAL_ERROR
    " LT_LLVM_CMAKE_FILE (${LT_LLVM_CMAKE_FILE}) is invalid.")
endif()

# Add path to LLVMConfig.cmake (so that find_package can find LLVM)
set(LT_LLVM_CMAKE_CONFIG_DIR "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/")
list(APPEND CMAKE_PREFIX_PATH "${LT_LLVM_CMAKE_CONFIG_DIR}")

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE
      STRING "Build type (default Debug):" FORCE)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++14 -fno-rtti -Wall\
    -fdiagnostics-color=always")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++14 -fno-rtti -Wall\
    -fdiagnostics-color=always -fno-omit-frame-pointer")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

# And the project header and library paths
include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${LIBRARY_OUTPUT_PATH})
set(LT_RT_LIBRARY_PATH "${PROJECT_BINARY_DIR}/lib")

#===============================================================================
# 2. LLVM-CONFIGURATION
# As per: http://llvm.org/docs/CMake.html#embedding-llvm-in-your-project
#===============================================================================
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LT_LLVM_INSTALL_DIR}")

# We incorporate the CMake features provided by LLVM:
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

option(LLVM_ENABLE_CXX1Y  "Enable C++14" ON)
option(LLVM_INCLUDE_TOOLS "Generate build targets for the LLVM tools." ON)
option(LLVM_BUILD_TOOLS "Build the LLVM tools. If OFF, just generate build targets." ON)

message("LLVM STATUS:
  Definitions ${LLVM_DEFINITIONS}
  Includes    ${LLVM_INCLUDE_DIRS}
  Libraries   ${LLVM_LIBRARY_DIRS}
  Targets     ${LLVM_TARGETS_TO_BUILD}"
)

# Now set the LLVM header and library paths:
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_DEFINITIONS})

#===============================================================================
# 3. ADD SUBDIRECTORIES
# Finally, add targets. Doing this at the end so that all definitions and
# link/include paths are available for the sub-projects.
#===============================================================================
add_subdirectory(lib)
add_subdirectory(tools)
add_subdirectory(test)
