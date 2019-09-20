# THE LIST OF PLUGINS AND THE CORRESPONDING SOURCE FILES
# ======================================================
set(LLVM_TUTOR_PLUGINS
    StaticCallCounter
    MBAAdd
    MBASub
    RIV
    DuplicateBB
    )

set(StaticCallCounter_SOURCES
  StaticCallCounter.cpp)
set(MBAAdd_SOURCES
  MBAAdd.cpp
  Ratio.cpp)
set(MBASub_SOURCES
  MBASub.cpp
  Ratio.cpp)
set(RIV_SOURCES
  RIV.cpp)
set(DuplicateBB_SOURCES
  DuplicateBB.cpp
  Ratio.cpp)

# CONFIGURE THE PLUGIN LIBRARIES
# ==============================
foreach( plugin ${LLVM_TUTOR_PLUGINS} )
    # Create a library corresponding to 'plugin'
    add_library(
      ${plugin}
      SHARED
      ${${plugin}_SOURCES}
      )

    # Configure include directories for 'plugin'
    target_include_directories(
      ${plugin}
      PUBLIC
      "${CMAKE_CURRENT_SOURCE_DIR}/../include"
    )

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
    target_link_libraries(
      ${plugin}
      "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>"
      )
endforeach()

# CONFIGURE LIBRARIES FOR 'dynamic'
# ================================
add_library(DynamicCallCounter STATIC DynamicCallCounter.cpp)
target_include_directories(
  DynamicCallCounter
  PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

# This is LT_RUNTIME_LIB used by lt-cc-main.cpp
# FIXME Create a better name for this
add_library(lt-cc-rt DynamicCallCounterRT.cpp)
