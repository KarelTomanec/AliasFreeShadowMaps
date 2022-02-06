# Try to find GLFW library
#
unset( GLFW_DLL CACHE )
unset( GLFW_INCLUDE_DIR CACHE )
unset( GLFW_FOUND CACHE )

find_path( GLFW_INCLUDE_DIR glfw3.h
  ${PROJECT_SOURCE_DIR}/shared/glfw/include
  ${PROJECT_SOURCE_DIR}/../shared/glfw/include
  ${PROJECT_SOURCE_DIR}/../../shared/glfw/include
  ${GLFW_LOCATION}/include
  $ENV{GLFW_LOCATION}/include
)


if( GLFW_INCLUDE_DIR )
  set( GLFW_LIB_PATH "${GLFW_INCLUDE_DIR}/../lib/" )
  if( UNIX )
    _find_shared_library( GLFW_DLL ${GLFW_LIB_PATH} "libglfw3.so" )
    _find_shared_library( GLFW_LIB ${GLFW_LIB_PATH} "libglfw3.a" )
  else( UNIX )
    _find_shared_library( GLFW_DLL ${GLFW_LIB_PATH} "glfw3.dll" )
    _find_shared_library( GLFW_LIB ${GLFW_LIB_PATH} "glfw3.lib" )
  endif( UNIX )

  if( GLFW_DLL )
    set( GLFW_FOUND "YES" )
    set( GLFW_HEADERS "${GLFW_INCLUDE_DIR}/glfw3.h")
  endif( GLFW_DLL )
else( GLFW_INCLUDE_DIR )
  message( WARNING "
    GLFW not found. 
    The GLFW folder you would specify with GLFW_LOCATION should contain:
    - lib folder: containing the glfw3.dll
    - include folder: containing the include files"
  )
endif( GLFW_INCLUDE_DIR )
include( FindPackageHandleStandardArgs )

find_package_handle_standard_args( GLFW DEFAULT_MSG GLFW_INCLUDE_DIR GLFW_DLL )

SET( GLFW_DLL ${GLFW_DLL} CACHE PATH "path" )

mark_as_advanced( GLFW_FOUND )
