project(TKJt)

OCCT_INCLUDE_CMAKE_FILE (adm/cmake/occt_toolkit)

if (EMSCRIPTEN)
  target_compile_options(TKJt PRIVATE -s USE_ZLIB=1)
endif()
