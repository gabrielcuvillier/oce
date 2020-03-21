project(TKVCAF CXX)

OCCT_INCLUDE_CMAKE_FILE (adm/cmake/occt_toolkit)

if (LIGHT_BUILD)
    message (STATUS "Info: [TKVCAF] OCCT_DISABLE_PRESENTATION_DRIVERS_IN_VCAF is defined")
    target_compile_definitions(TKVCAF PRIVATE -DOCCT_DISABLE_PRESENTATION_DRIVERS_IN_VCAF)
endif()
