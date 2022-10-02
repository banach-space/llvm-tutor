# TODO: Once LLVM 16 is released, replace `try_compile` with a test for
# LLVM_ENABLE_LTO, see https://reviews.llvm.org/D134936.

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
# The minimal list of the required LLVM libraries.
set(libs LLVMCore LLVMPasses LLVMIRReader LLVMSupport)

# The list above won't work when using LLVM from Homebrew. Indeed, your system
# linker will most likely complain:
# ```
#   (...) Opaque pointers are only supported in -opaque-pointers mode (Producer: 'LLVM15.0.0' Reader: 'LLVM APPLE_1_1400.0.29.102_0')
# ```
# That's more or less due to Homebrew packages being built with LTO (i.e.
# LLVM_ENABLE_LTO set to "ON") To work around this, you can use `libLLVM`
# instead. That solves the problem because:
#  * `libLLVM` is always a dynamic library (and the problem above is only
#    triggered for static libs)
#  * libLLVM includes all LLVM libraries (i.e. all the libraries listed below
#    _and more_)
# Check whether the default set-up is sufficient ...
try_compile(testMinimalCompilation "${CMAKE_BINARY_DIR}/temp"
  SOURCES ${static_SOURCES}
  LINK_LIBRARIES ${libs}
  CMAKE_FLAGS 
    "-DINCLUDE_DIRECTORIES=${CMAKE_CURRENT_SOURCE_DIR}/../include;${LLVM_INCLUDE_DIRS}"
    "-DLINK_DIRECTORIES=${LLVM_LIBRARY_DIRS}")

# ... if not, use libLLVM
if(NOT ${testMinimalCompilation})
  set(libs LLVM)
endif()

# ADD THE LIBRARIES TO LINK
#==========================
target_link_libraries(static
  ${libs}
  )
