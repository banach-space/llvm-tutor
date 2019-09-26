configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(dynamic
  DynamicMain.cpp
)

add_executable(static
  StaticMain.cpp
)

llvm_map_components_to_libnames(REQ_LLVM_LIBRARIES ${LLVM_TARGETS_TO_BUILD}
        asmparser
        core
        linker
        support
        bitreader
        bitwriter
        irreader
        ipo
        scalaropts
        analysis
        target
        mc
        support
        passes
)

target_link_libraries(dynamic DynamicCallCounter ${REQ_LLVM_LIBRARIES})
target_link_libraries(static StaticCallCounter ${REQ_LLVM_LIBRARIES})

set_target_properties(dynamic
                      PROPERTIES
                      LINKER_LANGUAGE CXX
                      PREFIX ""
)
