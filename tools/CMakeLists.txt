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
  message("LLVM is installed as shared library on Arch Linux")
  target_link_libraries(static LLVM)
else()
  target_link_libraries(static
    LLVMCore LLVMPasses LLVMIRReader LLVMSupport
  )
endif()
