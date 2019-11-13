project(TKernel)

OCCT_INCLUDE_CMAKE_FILE (adm/cmake/occt_toolkit)

if (LIGHT_BUILD)
  message (STATUS "Info: [TKernel] OCCT_DISABLE_THREADS is defined")
  target_compile_definitions(TKernel PRIVATE -DOCCT_DISABLE_THREADS)

  message (STATUS "Info: [TKernel] OCCT_DISABLE_UNICODE_CONVERSIONS is defined")
  target_compile_definitions(TKernel PRIVATE -DOCCT_DISABLE_UNICODE_CONVERSIONS)

  message (STATUS "Info: [TKernel] OCCT_DISABLE_OPTIMIZED_MEMORY_ALLOCATOR is defined")
  target_compile_definitions(TKernel PRIVATE -DOCCT_DISABLE_OPTIMIZED_MEMORY_ALLOCATOR)

  message (STATUS "Info: [TKernel] OCCT_DISABLE_SHAREDLIBRARY is defined")
  target_compile_definitions(TKernel PRIVATE -DOCCT_DISABLE_SHAREDLIBRARY)
endif()
