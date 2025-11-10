add_llvm_pass_plugin(SimpleLICM
  SimpleLICM.cpp
)
# Ensure the built .dylib is placed in the build/lib directory
set_target_properties(SimpleLICM PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)
