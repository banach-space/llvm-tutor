set(static_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/StaticMain.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../lib/StaticCallCounter.cpp"
)

# Display header files in Xcode
if(XCODE)
  file(GLOB static_HEADERS ../include/StaticCallCounter.h)
  set_source_files_properties(${static_HEADERS} PROPERTIES HEADER_FILE_ONLY true)
  source_group("Header Files" FILES ${static_HEADERS})
else()
  set(static_HEADERS)
endif()

add_executable(static ${static_SOURCES} ${static_HEADERS})

target_include_directories(
  static
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/../include")

target_link_libraries(static
  LLVMCore LLVMPasses LLVMIRReader LLVMSupport
)
