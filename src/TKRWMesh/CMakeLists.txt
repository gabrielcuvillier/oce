project(TKRWMesh CXX)

OCCT_INCLUDE_CMAKE_FILE (adm/cmake/occt_toolkit)

if (LIGHT_BUILD)
  #message (STATUS "Info: [TKRWMesh] OCCT_DISABLE_MESHING_IN_RWOBJ is defined")
  #target_compile_definitions(TKRWMesh PRIVATE -DOCCT_DISABLE_MESHING_IN_RWOBJ)
endif()
