project(TKXCAF)

OCCT_INCLUDE_CMAKE_FILE (adm/cmake/occt_toolkit)

if (EMSCRIPTEN)
  message (STATUS "Info: [TKXCAF] OCCT_DISABLE_VISUALIZATION_IN_XDE is defined")
  target_compile_definitions(TKXCAF PRIVATE -DOCCT_DISABLE_VISUALIZATION_IN_XDE)
endif()
