set(static_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/StaticMain.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../lib/StaticCallCounter.cpp"
)

add_executable(static ${static_SOURCES})

target_include_directories(
  static
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/../include")

if(UNIX AND EXISTS "/etc/arch-release")
  # LLVM is built as shared library on Arch Linux (*), so we need to link the
  # static executable against libLLVM.so. See #117
  # (*)  https://gitlab.archlinux.org/archlinux/packaging/packages/llvm/-/blob/main/PKGBUILD?ref_type=heads#L89
  message("LLVM is installed as shared library on Arch Linux")
  target_link_libraries(static LLVM)
else()
  target_link_libraries(static
    LLVMCore LLVMPasses LLVMIRReader LLVMSupport
  )
endif()
