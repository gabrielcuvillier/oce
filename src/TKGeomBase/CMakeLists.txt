project(TKGeomBase CXX)

OCCT_INCLUDE_CMAKE_FILE (adm/cmake/occt_toolkit)

if (LIGHT_BUILD)
  message (STATUS "Info: [TKGeomBase] OCCT_DISABLE_APPROX_FIT_AND_DIVIDE is defined")
  target_compile_definitions(TKGeomBase PRIVATE -DOCCT_DISABLE_APPROX_FIT_AND_DIVIDE)

  #message (STATUS "Info: [TKGeomBase] OCCT_ENABLE_APPCONTMATRIX_FILE_STORAGE is defined")
  #target_compile_definitions(TKGeomBase PRIVATE -DOCCT_ENABLE_APPCONTMATRIX_FILE_STORAGE)
endif()
