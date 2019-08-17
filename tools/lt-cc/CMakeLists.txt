configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(lt-cc
  lt-cc-main.cpp
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
)

target_link_libraries(lt-cc CallCounter ${REQ_LLVM_LIBRARIES})

set_target_properties(lt-cc
                      PROPERTIES
                      LINKER_LANGUAGE CXX
                      PREFIX ""
)
