project(TKRWMesh)

OCCT_INCLUDE_CMAKE_FILE (adm/cmake/occt_toolkit)

if (EMSCRIPTEN)
  target_compile_options(TKRWMesh PRIVATE -s USE_RAPIDJSON=1)
endif()
