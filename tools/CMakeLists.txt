add_executable(static
  StaticMain.cpp
)

llvm_map_components_to_libnames(REQ_LLVM_LIBRARIES ${LLVM_TARGETS_TO_BUILD}
        passes
)

target_link_libraries(static StaticCallCounter ${REQ_LLVM_LIBRARIES})
