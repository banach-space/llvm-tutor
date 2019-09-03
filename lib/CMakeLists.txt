add_library(CallCounter SHARED StaticCallCounter.cpp DynamicCallCounter.cpp)
add_library(CallCounterStatic STATIC StaticCallCounter.cpp DynamicCallCounter.cpp)

add_library(MBAAdd SHARED MBAAdd.cpp Ratio.cpp)
add_library(RIV SHARED RIV.cpp)
add_library(DuplicateBB SHARED DuplicateBB.cpp Ratio.cpp)

# On Darwin (unlike on Linux), undefined symbols in shared objects are not
# allowed at the end of the link-edit. Hence the following extra flag is
# needed. Otherwise, the build will fail with errors like:
#    "Undefined symbols for architecture x86_64"
# with various LLVM symbols being undefined. Those symbols are later loaded at
# runtime, so this error is a false positive.
target_link_libraries(CallCounter
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")
target_link_libraries(MBAAdd
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")
target_link_libraries(RIV
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")
target_link_libraries(DuplicateBB
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")

# This LT_RUNTIME_LIB used by lt-cc-main.cpp
# FIXME Create a better name for this
add_library(lt-cc-rt DynamicCallCounterRT.cpp)
