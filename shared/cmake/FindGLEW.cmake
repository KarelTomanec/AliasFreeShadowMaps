# Try to find GLEW library
#
unset( GLEW_DLL CACHE )
unset( GLEW_INCLUDE_DIR CACHE )
unset( GLEW_FOUND CACHE )

find_path( GLEW_INCLUDE_DIR glew.h
  ${PROJECT_SOURCE_DIR}/shared/glew/include
  ${PROJECT_SOURCE_DIR}/../shared/glew/include
  ${PROJECT_SOURCE_DIR}/../../shared/glew/include
  ${GLEW_LOCATION}/include
  $ENV{GLEW_LOCATION}/include
)


if( GLEW_INCLUDE_DIR )
  set( GLEW_LIB_PATH "${GLEW_INCLUDE_DIR}/../lib/" )
  if( UNIX )
    _find_shared_library( GLEW_DLL ${GLEW_LIB_PATH} "libGLEW.so" )
    _find_shared_library( GLEW_LIB ${GLEW_LIB_PATH} "libGLEW.a" )
  else( UNIX )
    _find_shared_library( GLEW_DLL ${GLEW_LIB_PATH} "glew32.dll" )
    _find_shared_library( GLEW_LIB ${GLEW_LIB_PATH} "glew32.lib" )
  endif( UNIX )

  if( GLEW_DLL )
    set( GLEW_FOUND "YES" )
    set( GLEW_HEADERS "${GLEW_INCLUDE_DIR}/glew.h")
  endif( GLEW_DLL )
else( GLEW_INCLUDE_DIR )
  message( WARNING "
    GLEW not found. 
    The GLEW folder you would specify with GLEW_LOCATION should contain:
    - lib folder: containing the glew32.dll or libglew.so
    - include folder: containing the include files"
  )
endif( GLEW_INCLUDE_DIR )
include( FindPackageHandleStandardArgs )

find_package_handle_standard_args( GLEW DEFAULT_MSG GLEW_INCLUDE_DIR GLEW_DLL )

SET( GLEW_DLL ${GLEW_DLL} CACHE PATH "path" )

mark_as_advanced( GLEW_FOUND )
