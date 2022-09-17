# DEFINE THE TARGET
#==================
set(static_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/StaticMain.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../lib/StaticCallCounter.cpp"
)

add_executable(static ${static_SOURCES})

target_include_directories(
  static
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/../include")

# DECIDE WHAT LIBRARIES TO LINK IN
#=================================
# The list of the libraries below is the required minimum. However, when using
# LLVM from Homebrew your system linker might complain when using these:
# ```
#   (...) Opaque pointers are only supported in -opaque-pointers mode (Producer: 'LLVM15.0.0' Reader: 'LLVM APPLE_1_1400.0.29.102_0')
# ```
# To work around this this, you can use `libLLVM` instead. That solves the
# problem because:
#  * `libLLVM` is always a dynamic library (and the problem above is only
#    triggered for static libs)
#  * libLLVM includes all LLVM libraries (i.e. all the libraries listed below
#    _and more_)
set(libs "LLVMCore LLVMPasses LLVMIRReader LLVMSupport")

try_compile(testMinimalCompilation "${CMAKE_BINARY_DIR}/temp" SOURCES ${static_SOURCES})

if(NOT ${testMinimalCompilation})
  set(libs "LLVM")
endif()

target_link_libraries(static
  ${libs}
  )
