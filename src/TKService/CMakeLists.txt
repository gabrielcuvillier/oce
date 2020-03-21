project(TKService CXX)

OCCT_INCLUDE_CMAKE_FILE (adm/cmake/occt_toolkit)

if (LIGHT_BUILD)
    message (STATUS "Info: [TKService] OCCT_MINIMAL_FREETYPE_BUILD is defined")
    target_compile_definitions(TKService PRIVATE -DOCCT_MINIMAL_FREETYPE_BUILD)
endif()
