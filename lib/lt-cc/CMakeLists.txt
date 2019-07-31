add_library(lt-cc-static STATIC StaticCallCounter.cpp DynamicCallCounter.cpp)
add_library(lt-cc-shared SHARED StaticCallCounter.cpp DynamicCallCounter.cpp)

# On Darwin (unlike on Linux), undefined symbols in shared objects are not
# allowed at the end of the link-edit. Hence the following extra flag is
# needed. Otherwise, the build will fail with errors like:
#    "Undefined symbols for architecture x86_64"
# with various LLVM symbols being undefined. Those symbols are later loaded at
# runtime, so this error is a false positive.
target_link_libraries(lt-cc-shared
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")
