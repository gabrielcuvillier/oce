##

if(FLAGS_ALREADY_INCLUDED)
  return()
endif()
set(FLAGS_ALREADY_INCLUDED 1)

if(EMSCRIPTEN)
  message(STATUS "Info: Building for Emscripten/WebAssembly.")
endif()

# force option /fp:precise for Visual Studio projects.
#
# Note that while this option is default for MSVC compiler, Visual Studio
# project can be switched later to use Intel Compiler (ICC).
# Enforcing -fp:precise ensures that in such case ICC will use correct
# option instead of its default -fp:fast which is harmful for OCCT.
if (MSVC)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:precise")
  set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   /fp:precise")
endif()

# set compiler short name and choose SSE2 option for appropriate MSVC compilers
# ONLY for 32-bit
if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  if (MSVC80 OR MSVC90 OR MSVC10)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
    set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   /arch:SSE2")
  endif()
endif()

if (WIN32)
  add_definitions (-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)
else()
  if (EMSCRIPTEN)
    message (STATUS "Info: Exceptions are disabled")
    add_compile_options(-fno-exceptions)

    message (STATUS "Info: PIC disabled")
    add_compile_options(-fno-PIC)

    message (STATUS "Info: OCCT_DISABLE_MULTITHREADING is defined due to unsupported on Browsers")
    add_definitions(-DOCCT_DISABLE_MULTITHREADING)

    message (STATUS "Info: OCCT_DISABLE_UNICODE_CONVERSIONS is defined")
    add_definitions(-DOCCT_DISABLE_UNICODE_CONVERSIONS)

    #Keep Meshing in Vizu for now
    #message (STATUS "Info: OCCT_DISABLE_MESHING_IN_VISUALIZATION is defined")
    #add_definitions(-DOCCT_DISABLE_MESHING_IN_VISUALIZATION)

    message (STATUS "Info: OCCT_DISABLE_EXACTHLR_IN_VISUALIZATION is defined")
    add_definitions(-DOCCT_DISABLE_EXACTHLR_IN_VISUALIZATION)

    message (STATUS "Info: OCCT_DISABLE_OPTIMIZED_MEMORY_ALLOCATOR is defined")
    add_definitions(-DOCCT_DISABLE_OPTIMIZED_MEMORY_ALLOCATOR)

    message (STATUS "Info: OCCT_DISABLE_SHAREDLIBRARY is defined")
    add_definitions(-DOCCT_DISABLE_SHAREDLIBRARY)

    message (STATUS "Info: OCCT_HANDLE_NOCAST is defined to prevent some unsafe methods with Handles")
    add_definitions(-DOCCT_HANDLE_NOCAST)

    message (STATUS "Info: OCCT_IGNORE_NO_ATOMICS is defined due to unsupported atomics on Emscripten")
    add_definitions(-DOCCT_IGNORE_NO_ATOMICS)

    message (STATUS "Info: OCCT_CONVERT_SIGNALS is NOT defined")
  else()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -fPIC")
    set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fexceptions -fPIC")
    add_definitions(-DOCCT_CONVERT_SIGNALS)
  endif()
endif()

# enable structured exceptions for MSVC
string (REGEX MATCH "EHsc" ISFLAG "${CMAKE_CXX_FLAGS}")
if (ISFLAG)
  string (REGEX REPLACE "EHsc" "EHa" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
elseif (MSVC)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHa")
endif()

# remove _WINDOWS flag if it exists
string (REGEX MATCH "/D_WINDOWS" IS_WINDOWSFLAG "${CMAKE_CXX_FLAGS}")
if (IS_WINDOWSFLAG)
  message (STATUS "Info: /D_WINDOWS has been removed from CMAKE_CXX_FLAGS")
  string (REGEX REPLACE "/D_WINDOWS" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

# remove WIN32 flag if it exists
string (REGEX MATCH "/DWIN32" IS_WIN32FLAG "${CMAKE_CXX_FLAGS}")
if (IS_WIN32FLAG)
  message (STATUS "Info: /DWIN32 has been removed from CMAKE_CXX_FLAGS")
  string (REGEX REPLACE "/DWIN32" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

# remove _WINDOWS flag if it exists
string (REGEX MATCH "/D_WINDOWS" IS_WINDOWSFLAG "${CMAKE_C_FLAGS}")
if (IS_WINDOWSFLAG)
  message (STATUS "Info: /D_WINDOWS has been removed from CMAKE_C_FLAGS")
  string (REGEX REPLACE "/D_WINDOWS" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
endif()

# remove WIN32 flag if it exists
string (REGEX MATCH "/DWIN32" IS_WIN32FLAG "${CMAKE_C_FLAGS}")
if (IS_WIN32FLAG)
  message (STATUS "Info: /DWIN32 has been removed from CMAKE_C_FLAGS")
  string (REGEX REPLACE "/DWIN32" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
endif()

# remove DEBUG flag if it exists
string (REGEX MATCH "-DDEBUG" IS_DEBUG_CXX "${CMAKE_CXX_FLAGS_DEBUG}")
if (IS_DEBUG_CXX)
  message (STATUS "Info: -DDEBUG has been removed from CMAKE_CXX_FLAGS_DEBUG")
  string (REGEX REPLACE "-DDEBUG" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
endif()

string (REGEX MATCH "-DDEBUG" IS_DEBUG_C "${CMAKE_C_FLAGS_DEBUG}")
if (IS_DEBUG_C)
  message (STATUS "Info: -DDEBUG has been removed from CMAKE_C_FLAGS_DEBUG")
  string (REGEX REPLACE "-DDEBUG" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
endif()
# enable parallel compilation on MSVC 9 and above
if (MSVC AND NOT MSVC70 AND NOT MSVC80)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

# generate a single response file which enlist all of the object files
SET(CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS 1)
SET(CMAKE_CXX_USE_RESPONSE_FILE_FOR_OBJECTS 1)
# increase compiler warnings level (-W4 for MSVC, -Wextra for GCC)
if (MSVC)
  if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string (REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
  if (BUILD_SHARED_LIBS)
    if (APPLE)
      set (CMAKE_SHARED_LINKER_FLAGS "-lm ${CMAKE_SHARED_LINKER_FLAGS}")
    elseif(NOT WIN32)
      set (CMAKE_SHARED_LINKER_FLAGS "-lm ${CMAKE_SHARED_LINKER_FLAGS}")
    endif()
  endif()
endif()

if(MINGW)
  # Set default release optimization option to O2 instead of O3, since in
  # some OCCT related examples, this gives significantly smaller binaries
  # at comparable performace with MinGW-w64.
  string (REGEX MATCH "-O3" IS_O3_CXX "${CMAKE_CXX_FLAGS_RELEASE}")
  if (IS_O3_CXX)
    string (REGEX REPLACE "-O3" "-O2" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  else()
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
  endif()

  set (CMAKE_CXX_FLAGS "-std=gnu++0x ${CMAKE_CXX_FLAGS}")
  add_definitions(-D_WIN32_WINNT=0x0501)
  # workaround bugs in mingw with vtable export
  set (CMAKE_SHARED_LINKER_FLAGS "-Wl,--export-all-symbols")
elseif ("x${CMAKE_CXX_COMPILER_ID}" STREQUAL "xClang")
  if (APPLE)
    # CLang can be used with both libstdc++ and libc++, however on OS X libstdc++ is outdated.
    set (CMAKE_CXX_FLAGS "-std=c++0x -stdlib=libc++ ${CMAKE_CXX_FLAGS}")
  else()
    if (EMSCRIPTEN)
      set(CMAKE_CXX_STANDARD 17)
    else()
      set (CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")
    endif()
  endif()
elseif (DEFINED CMAKE_COMPILER_IS_GNUCXX)
  set (CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")
endif()

# Optimize size of binaries
if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR MINGW)
  set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
  set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
endif()

set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNo_Exception")
set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNo_Exception")
set (CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -DNo_Exception")
set (CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -DNo_Exception")

set (ENABLE_LTO OFF CACHE BOOL "Enable LTO")
if (ENABLE_LTO)
  if (EMSCRIPTEN)
    set(COMPILE_LTO_FLAGS "-s WASM_OBJECT_FILES=0 -flto")
    set(LINK_LTO_FLAGS    "-s WASM_OBJECT_FILES=0 --llvm-lto 1 -flto")
  else()
    set(COMPILE_LTO_FLAGS "-flto")
    set(LINK_LTO_FLAGS    "-flto")
  endif()
else()
  unset(COMPILE_LTO_FLAGS)
  unset(LINK_LTO_FLAGS)
endif()

set(CMAKE_C_FLAGS_RELEASE             "${CMAKE_C_FLAGS_RELEASE} ${COMPILE_LTO_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE           "${CMAKE_CXX_FLAGS_RELEASE} ${COMPILE_LTO_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE    "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${LINK_LTO_FLAGS}")
set(CMAKE_C_FLAGS_MINSIZEREL          "${CMAKE_C_FLAGS_MINSIZEREL} ${COMPILE_LTO_FLAGS}")
set(CMAKE_CXX_FLAGS_MINSIZEREL        "${CMAKE_CXX_FLAGS_MINSIZEREL} ${COMPILE_LTO_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} ${LINK_LTO_FLAGS}")

set (ENABLE_OZ OFF CACHE BOOL "Enable Oz build for MinSizeRel")
if (ENABLE_OZ)
  # Use 'Oz' optimization level (instead of Os)
  string (REGEX MATCH "-Os" IS_Os_CXX "${CMAKE_CXX_FLAGS_MINSIZEREL}")
  if (IS_Os_CXX)
    string (REGEX REPLACE "-Os" "-Oz" CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL}")
  else()
    set (CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -Oz")
  endif()

  string (REGEX MATCH "-Os" IS_Os_C "${CMAKE_C_FLAGS_MINSIZEREL}")
  if (IS_Os_C)
    string (REGEX REPLACE "-Os" "-Oz" CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL}")
  else()
    set (CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -Oz")
  endif()
endif()

set (ENABLE_O3 OFF CACHE BOOL "Enable O3 build for Release")
if (ENABLE_O3)
  # Use 'O2' optimization level (instead of O3)
  string (REGEX MATCH "-O2" IS_O2_CXX "${CMAKE_CXX_FLAGS_RELEASE}")
  if (IS_O2_CXX)
    string (REGEX REPLACE "-O2" "-O3" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  else()
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
  endif()

  string (REGEX MATCH "-O2" IS_O2_C "${CMAKE_C_FLAGS_RELEASE}")
  if (IS_O2_C)
    string (REGEX REPLACE "-O2" "-O3" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
  else()
    set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
  endif()
endif()

set (ENABLE_O1 OFF CACHE BOOL "Enable O1 build for Release")
if (ENABLE_O1)
  # Use 'O2' optimization level (instead of O3)
  string (REGEX MATCH "-O2" IS_O2_CXX "${CMAKE_CXX_FLAGS_RELEASE}")
  if (IS_O2_CXX)
    string (REGEX REPLACE "-O2" "-O1" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  else()
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O1")
  endif()

  string (REGEX MATCH "-O2" IS_O2_C "${CMAKE_C_FLAGS_RELEASE}")
  if (IS_O2_C)
    string (REGEX REPLACE "-O2" "-O1" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
  else()
    set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O1")
  endif()
endif()
