project(TKXCAF)

OCCT_INCLUDE_CMAKE_FILE (adm/cmake/occt_toolkit)

if (LIGHT_BUILD)
    message (STATUS "Info: [TXCAF] OCCT_DISABLE_MESHING_IN_XDE is defined")
    target_compile_definitions(TKXCAF PRIVATE -DOCCT_DISABLE_MESHING_IN_XDE)

    message (STATUS "Info: [TXCAF] OCCT_DISABLE_VCAF_IN_XDE is defined")
    target_compile_definitions(TKXCAF PRIVATE -DOCCT_DISABLE_VCAF_IN_XDE)
endif()
