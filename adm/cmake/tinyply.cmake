# TinyPly

if (NOT DEFINED INSTALL_TINYPLY)
  set (INSTALL_TINYPLY OFF CACHE BOOL "${INSTALL_TINYPLY_DESCR}")
endif()

# TINYPLY directory
if (NOT DEFINED 3RDPARTY_TINYPLY_DIR)
  set (3RDPARTY_TINYPLY_DIR "" CACHE PATH "The directory containing TinyPly")
endif()

# search for TinyPly in user defined directory
if (3RDPARTY_DIR AND EXISTS "${3RDPARTY_DIR}")
  if (NOT 3RDPARTY_TINYPLY_DIR OR NOT EXISTS "${3RDPARTY_TINYPLY_DIR}")
    FIND_PRODUCT_DIR("${3RDPARTY_DIR}" TinyPly TINYPLY_DIR_NAME)
    if (TINYPLY_DIR_NAME)
      set (3RDPARTY_TINYPLY_DIR "${3RDPARTY_DIR}/${TINYPLY_DIR_NAME}" CACHE PATH "The directory containing TinyPly" FORCE)
    endif()
  endif()
endif()

if (NOT DEFINED 3RDPARTY_TINYPLY_INCLUDE_DIR)
  set (3RDPARTY_TINYPLY_INCLUDE_DIR  "" CACHE FILEPATH "The directory containing headers of the TINYPLY")
endif()

if (NOT 3RDPARTY_TINYPLY_INCLUDE_DIR OR NOT EXISTS "${3RDPARTY_TINYPLY_INCLUDE_DIR}")

  set (HEADER_NAMES tinyply.h)

  set (3RDPARTY_TINYPLY_INCLUDE_DIR "3RDPARTY_TINYPLY_INCLUDE_DIR-NOTFOUND" CACHE PATH "the path to TinyPly header file" FORCE)

  if (3RDPARTY_TINYPLY_DIR AND EXISTS "${3RDPARTY_TINYPLY_DIR}")
    find_path (3RDPARTY_TINYPLY_INCLUDE_DIR NAMES ${HEADER_NAMES}
                                              PATHS ${3RDPARTY_TINYPLY_DIR}
                                              PATH_SUFFIXES source
                                              CMAKE_FIND_ROOT_PATH_BOTH
                                              NO_DEFAULT_PATH)
  else()
    find_path (3RDPARTY_TINYPLY_INCLUDE_DIR NAMES ${HEADER_NAMES}
                                              PATH_SUFFIXES source
                                              CMAKE_FIND_ROOT_PATH_BOTH)
  endif()

  # use default (CMake) TinyPly search
  if (NOT 3RDPARTY_TINYPLY_INCLUDE_DIR OR NOT EXISTS "${3RDPARTY_TINYPLY_INCLUDE_DIR}")
    if (3RDPARTY_TINYPLY_DIR AND EXISTS "${3RDPARTY_TINYPLY_DIR}")
      set (CACHED_TINYPLY_DIR $ENV{TinyPly_DIR})
      set (ENV{TinyPly_DIR} "${3RDPARTY_TINYPLY_DIR}")
    endif()

    find_package(TinyPly QUIET)

    # restore ENV{TinyPly_DIR}
    if (3RDPARTY_TINYPLY_DIR AND EXISTS "${3RDPARTY_TINYPLY_DIR}")
      set (ENV{TinyPly_DIR} ${CACHED_TINYPLY_DIR})
    endif()

    if (${TINYPLY_FOUND})
      set (3RDPARTY_TINYPLY_INCLUDE_DIR "${TINYPLY_INCLUDE_DIR}" CACHE PATH "the path to TinyPly header file" FORCE)
      set (3RDPARTY_TINYPLY_DIR         "${TINYPLY_ROOT_DIR}"    CACHE PATH "The directory containing TinyPly" FORCE)
    endif()
  endif()
endif()

if (3RDPARTY_TINYPLY_INCLUDE_DIR AND EXISTS "${3RDPARTY_TINYPLY_INCLUDE_DIR}")
  list (APPEND 3RDPARTY_INCLUDE_DIRS "${3RDPARTY_TINYPLY_INCLUDE_DIR}")

  # Install header files
  if (INSTALL_TINYPLY)
    file(GLOB TINYPLY_SUBDIRS "${3RDPARTY_TINYPLY_INCLUDE_DIR}/*")
    foreach(SUBDIR ${TINYPLY_SUBDIRS})
      if(IS_DIRECTORY "${SUBDIR}")
        install (DIRECTORY "${SUBDIR}" DESTINATION "${INSTALL_DIR_INCLUDE}")
      else()
        install (FILES "${SUBDIR}" DESTINATION "${INSTALL_DIR_INCLUDE}")
      endif()
    endforeach()
  endif()
else()
  list (APPEND 3RDPARTY_NOT_INCLUDED 3RDPARTY_TINYPLY_INCLUDE_DIR)

  set (3RDPARTY_TINYPLY_INCLUDE_DIR "" CACHE PATH "the path to TinyPly header file" FORCE)
endif()

# unset all redundant variables
OCCT_CHECK_AND_UNSET(TinyPly_DIR)
