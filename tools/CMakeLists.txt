add_executable(static
  StaticMain.cpp
  ../lib/StaticCallCounter.cpp
)

target_link_libraries(static
  LLVMCore
  LLVMPasses
  LLVMIRReader
  LLVMSupport
  )

target_include_directories(
  static
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/../include")
