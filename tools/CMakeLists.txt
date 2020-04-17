add_executable(static
  StaticMain.cpp
  ../lib/StaticCallCounter.cpp
)

target_link_libraries(static
  LLVMPasses)

target_include_directories(
  static
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/../include")
