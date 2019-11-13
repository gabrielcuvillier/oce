project(TKV3d)

OCCT_INCLUDE_CMAKE_FILE (adm/cmake/occt_toolkit)

if (LIGHT_BUILD)
  message (STATUS "Info: [TKV3d] OCCT_DISABLE_MESHING_IN_VISUALIZATION is defined")
  target_compile_definitions(TKV3d PRIVATE -DOCCT_DISABLE_MESHING_IN_VISUALIZATION)

  message (STATUS "Info: [TKV3d] OCCT_DISABLE_HLR_IN_VISUALIZATION is defined")
  target_compile_definitions(TKV3d PRIVATE -DOCCT_DISABLE_HLR_IN_VISUALIZATION)
endif()
