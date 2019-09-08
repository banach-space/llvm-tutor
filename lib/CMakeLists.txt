add_library(StaticCallCounter SHARED StaticCallCounter.cpp)
add_library(DynamicCallCounter SHARED DynamicCallCounter.cpp)
add_library(DynamicCallCounterStatic STATIC DynamicCallCounter.cpp)

add_library(MBAAdd SHARED MBAAdd.cpp Ratio.cpp)
add_library(MBASub SHARED MBASub.cpp Ratio.cpp)
add_library(RIV SHARED RIV.cpp)
add_library(DuplicateBB SHARED DuplicateBB.cpp Ratio.cpp)

# On Darwin (unlike on Linux), undefined symbols in shared objects are not
# allowed at the end of the link-edit. The plugins defined here:
#  - _are_ shared objects
#  - reference symbols from LLVM shared libraries, i.e. symbols which are
#    undefined until those shared objects are loaded in memory (and hence
#    _undefined_ during static linking)
# The build will fail with errors like this:
#    "Undefined symbols for architecture x86_64"
# with various LLVM symbols being undefined. Since those symbols are later
# loaded and resolved at runtime, these errors are false positives.
# This behaviour can be modified via the '-undefined' OS X linker flag as
# follows.
target_link_libraries(StaticCallCounter
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")
target_link_libraries(DynamicCallCounter
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")
target_link_libraries(MBAAdd
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")
target_link_libraries(MBASub
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")
target_link_libraries(RIV
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")
target_link_libraries(DuplicateBB
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")

# This LT_RUNTIME_LIB used by lt-cc-main.cpp
# FIXME Create a better name for this
add_library(lt-cc-rt DynamicCallCounterRT.cpp)
