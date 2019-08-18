project(TKService)

OCCT_INCLUDE_CMAKE_FILE (adm/cmake/occt_toolkit)

if (EMSCRIPTEN)
  # Freetype is being used due to Font class, so use the Freetype port provided by Emscripten
  # Keep this compile option private, as modules requiring TKService does not need to know about Freetype at compilation time
  target_compile_options(TKService PRIVATE -s USE_FREETYPE=1)
  # In an ideal world we would use "target_link_options" also with PUBLIC exposition, but his command is only available starting CMake 3.13...
endif()
