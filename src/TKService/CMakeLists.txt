project(TKService)

OCCT_INCLUDE_CMAKE_FILE (adm/cmake/occt_toolkit)

if (EMSCRIPTEN)
  # Freetype is being used due to OpenGl_Font class
  #
  # Use the Freetype port provided by Emscripten
  # Keep this specific compile option private, as modules requiring TKOpenGl does not need to know about Freetype at compilation time
  target_compile_options(TKService PRIVATE -s USE_FREETYPE=1)
  # In an ideal world we would use "target_link_options" also, this time with PUBLIC instead of PRIVATE for linking purposes
  # but his command is only available starting CMake 3.13...
endif()
