set(TOOLKIT TKV3d)

set(TOOLKIT_MODULES
	V3d
	Graphic3d
	Visual3d
	Select3D
	Prs3d
	StdPrs
	SelectMgr
	PrsMgr
	AIS
	DsgPrs
	StdSelect
)
set(TOOLKIT_DEPENDS
	TKBRep
	TKMath
	TKernel
	TKService
	TKShHealing
	TKTopAlgo
	TKG2d
	TKG3d
	TKGeomBase
	TKMesh
	TKGeomAlgo
	TKHLR
)
set(TOOLKIT_LIBS ${CSF_user32_LIB} ${CSF_gdi32_LIB} ${OPENGL_LIBRARIES})

if(CMAKE_CONFIGURATION_TYPES OR NMAKE)
    set(TOOLKIT_LIBS_DEBUG ${FREETYPE_LIBRARY_DEBUG})
    set(TOOLKIT_LIBS_OPTIMIZED ${FREETYPE_LIBRARIES})
else(CMAKE_CONFIGURATION_TYPES OR NMAKE)
    set(TOOLKIT_LIBS ${TOOLKIT_LIBS} ${FREETYPE_LIBRARIES})
endif(CMAKE_CONFIGURATION_TYPES OR NMAKE)
set(TOOLKIT_INCLUDE_DIRECTORIES ${FREETYPE_INCLUDE_DIRS})

if(OCE_DISABLE_TKSERVICE_FONT)
	set(TOOLKIT_INCLUDE_DIRECTORIES ${TOOLKIT_INCLUDE_DIRECTORIES} "${OCE_SOURCE_DIR}/src/Font/")
endif()

include(../BuildToolkit.cmake)

if (EMSCRIPTEN AND NOT OCE_DISABLE_TKSERVICE_FONT)
  # Freetype is being used due to Brep_Font class
  #
  # Use the Freetype port provided by Emscripten
  # Keep this specific compile option private, as modules requiring TKV3d does not need to know about Freetype at compilation time
  target_compile_options(TKV3d PRIVATE -s USE_FREETYPE=1)
  # In an ideal world we would use "target_link_options" also, this time with PUBLIC instead of PRIVATE for linking purposes
  # but his command is only available starting CMake 3.13...
endif(EMSCRIPTEN AND NOT OCE_DISABLE_TKSERVICE_FONT)

