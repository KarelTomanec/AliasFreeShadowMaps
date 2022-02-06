cmake_minimum_required(VERSION 2.8)
Project("PGR2Examples")

#####################################################################################
# Set CPU architectures
#
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  set ( ARCH "x64" CACHE STRING "CPU Architecture" )
  message( STATUS "Building for x64." )
else ()
  message( STATUS "Building for x86." )
  set ( ARCH "x86" CACHE STRING "CPU Architecture" )
endif()

#####################################################################################
# General compile flags
#

#####################################################################################
# Project directories
#
SET( BASE_PATH          ${CMAKE_CURRENT_SOURCE_DIR} )
SET( SHARED_DATA_PATH   ${BASE_PATH}/shared )
SET( RESOURCE_DIRECTORY ${SHARED_DATA_PATH}/resources )
set( EXECUTABLE_OUTPUT_PATH ${BASE_PATH}/bin_${ARCH} CACHE PATH "Directory where executables will be stored" )
set( CMAKE_FIND_ROOT_PATH "" )
set( CMAKE_MODULE_PATH ${SHARED_DATA_PATH}/cmake )


#####################################################################################
# Set default build to Release
#
if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE )
endif( NOT CMAKE_BUILD_TYPE )


#####################################################################################
# OS related defines and settings
#
if( UNIX ) 
  set( OS "linux" )
  add_definitions( -DLINUX -g -std=c++11 )
else( UNIX )
  if( WIN32 )
    set( OS "win" )
    add_definitions( -DNOMINMAX )
  endif( WIN32 )
endif( UNIX )


#####################################################################################
# Macro for adding new project to the solution
#
macro(_add_project_definitions name)  
  add_definitions(-DPROJECT_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/")
  add_definitions(-DPROJECT_NAME="${name}")  
endmacro(_add_project_definitions)


#####################################################################################
# Macro for loading shared library
#
macro( _find_shared_library target folder libName )
  if( ARCH STREQUAL "x86" )
    message( STATUS "Looking for ${folder}x86/${libName}" )
    file( GLOB SHARED_LIBRARIES "${folder}x86/${libName}" )
  else()
    message( STATUS "Looking for ${folder}x64/${libName}" )
    file( GLOB SHARED_LIBRARIES "${folder}x64/${libName}" )
  endif()
  
  list( LENGTH SHARED_LIBRARIES NUMD_LIBRARIES )
  if( NUMD_LIBRARIES EQUAL 0 )
    if( ARCH STREQUAL "x86" )
      message( STATUS "Following file was found but requested ${folder}x86/${libName}" )
	else()
      message( STATUS "Following file was found but requested ${folder}x64/${libName}" )
	endif()
    set ( ${target} "NOTFOUND" )
  else()
    list( GET SHARED_LIBRARIES 0 ${target} )
  endif()
endmacro()


#####################################################################################
# Macro for copying shared files (libraries) to output directory
#
macro( _copy_files_to_target target fileList )
  if( WIN32 )
    foreach ( _file ${fileList} )
      if( EXISTS "${_file}" )
        add_custom_command(
          TARGET ${target} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${_file}
            $<TARGET_FILE_DIR:${target}>
            VERBATIM
        )
      endif()
    endforeach()
  endif()
endmacro()


#####################################################################################
# Macro that copies various binaries that need to be close to the exe files
#
macro( _copy_binaries_to_target PROJECT_NAME )
  if( GLFW_FOUND )
    _copy_files_to_target( ${PROJECT_NAME} "${GLFW_DLL}" )
    install( FILES "${GLFW_DLL}" DESTINATION bin )
  endif()
  if(GLEW_FOUND)
    _copy_files_to_target( ${PROJECT_NAME} "${GLEW_DLL}")
    install(FILES "${GLEW_DLL}" DESTINATION bin)
  endif()
  if(OPENCL_DLL)
    _copy_files_to_target( ${PROJECT_NAME} "${OPENCL_DLL}")
    install(FILES "${OPENCL_DLL}" DESTINATION bin)
  endif()
  install(TARGETS ${PROJECT_NAME} DESTINATION bin CONFIGURATIONS Release)
  install(TARGETS ${PROJECT_NAME} DESTINATION bin CONFIGURATIONS Debug)
endmacro()


#####################################################################################
# GLFW
#
macro( _add_package_GLFW )
  find_package( GLFW )
  if( GLFW_FOUND )
    Message( STATUS "--> using package GLFW" )
    include_directories( ${GLFW_INCLUDE_DIR} )
    LIST( APPEND PACKAGE_SOURCE_FILES ${GLFW_HEADERS} )
    LIST( APPEND LIBRARIES_OPTIMIZED ${GLFW_LIB} )
    LIST( APPEND LIBRARIES_DEBUG ${GLFW_LIB} )
    source_group( GLFW FILES ${GLFW_HEADERS} )
  else()
    Message( STATUS "--> NOT using package GLFW" )
  endif()
endmacro()


#####################################################################################
# GLEW
#
macro( _add_package_GLEW )
  find_package( GLEW )
  if( GLEW_FOUND )
    Message( STATUS "--> using package GLEW" )
    include_directories( ${GLEW_INCLUDE_DIR} )
    LIST( APPEND PACKAGE_SOURCE_FILES ${GLEW_HEADERS} )
    LIST( APPEND LIBRARIES_OPTIMIZED ${GLEW_LIB} )
    LIST( APPEND LIBRARIES_DEBUG ${GLEW_LIB} )
    source_group( GLEW FILES ${GLEW_HEADERS} )
  else()
    Message( STATUS "--> NOT using package GLEW" )
  endif()
endmacro()


#####################################################################################
# Optional VulkanSDK package
#
macro( _add_package_VulkanSDK )
  find_package( VulkanSDK )
  if( VULKANSDK_FOUND )
    Message( STATUS "--> using package VulkanSDK" )
    add_definitions( -DUSEVULKANSDK )
    include_directories( ${VULKANSDK_INCLUDE_DIR} )
    LIST( APPEND LIBRARIES_OPTIMIZED ${VULKAN_LIB} )
    LIST( APPEND LIBRARIES_DEBUG ${VULKAN_LIB} )
    LIST( APPEND PACKAGE_SOURCE_FILES ${VULKANSDK_HEADERS} )
    source_group( OPTIX FILES  ${VULKANSDK_HEADERS} )
  else()
    Message( STATUS "--> NOT using package VulkanSDK" )
  endif()
endmacro()


#####################################################################################
# Optional CUDA package
# WARNING: also used for OpenCL at this point. Rather clunky but
# CUDA contains OpenCL stuff...
#
macro( _add_package_CUDA )
  find_package( CUDA )  
  if( CUDA_FOUND )
    Message( STATUS "--> using package CUDA" )
    add_definitions( -DUSECUDA )
    include_directories( ${CUDA_INCLUDE_DIR} )
    LIST( APPEND LIBRARIES_OPTIMIZED ${CUDA_LIB} )
    LIST( APPEND LIBRARIES_DEBUG ${CUDA_LIB} )
    LIST( APPEND PACKAGE_SOURCE_FILES ${CUDA_HEADERS} )    
    source_group( CUDA FILES ${CUDA_HEADERS} )  
  else()
    Message( STATUS "--> NOT using package CUDA" ) 
  endif()
endmacro()


#####################################################################################
# Optional OpenCL package
#
macro( _add_package_OpenCL )
  find_package( OpenCL )  
  if( OpenCL_FOUND )
    Message( STATUS "--> using package OpenCL : ${OpenCL_LIBRARIES}" )
    add_definitions( -DUSEOPENCL )
    include_directories( ${OpenCL_INCLUDE_DIRS} )
    # just do the copy only if we pointed to the local OpenCL package
    string( FIND ${OpenCL_INCLUDE_DIRS} "shared" OFFSET )
    if( ( OFFSET GREATER -1 ) AND WIN32 )
      if( ARCH STREQUAL "x86" )
        set( OPENCL_DLL ${SHARED_DATA_PATH}/OpenCL/lib/x86/OpenCL.dll )
      else()
        set( OPENCL_DLL ${SHARED_DATA_PATH}/OpenCL/lib/x64/OpenCL.dll )
      endif()
    endif()
    LIST( APPEND LIBRARIES_OPTIMIZED ${OpenCL_LIBRARIES} )
    LIST( APPEND LIBRARIES_DEBUG ${OpenCL_LIBRARIES} )
  else()
    Message( STATUS "--> NOT using package OpenCL" ) 
  endif()
endmacro()


#####################################################################################
# Macro to download a file from a URL
#
macro( _download_file _URL _TARGET _FORCE )
  if( ${_FORCE} OR (NOT EXISTS ${_TARGET}) )
    Message( STATUS "downloading ${_URL} ==> ${_TARGET}" )
    file( DOWNLOAD ${_URL} ${_TARGET} SHOW_PROGRESS )
  else()
    Message( STATUS "model ${_TARGET} already loaded..." )
  endif()
endmacro()
#
# example: _download_files("${FILELIST}"  "http://..." "${SHARED_DATA_PATH}/..." ${MODELS_DOWNLOAD_FORCE} )
#
macro( _download_files _FILELIST _URL _TARGET _FORCE )
  foreach( _FILE ${_FILELIST} )
    if( ${_FORCE} OR ( NOT EXISTS "${_TARGET}/${_FILE}" ) )
      Message( STATUS "*******************************************" )
      Message( STATUS "downloading ${_URL}/${_FILE}\n ==>\n ${_TARGET}" )
      Message( STATUS "*******************************************" )
      file( DOWNLOAD ${_URL}/${_FILE} ${_TARGET}/${_FILE} SHOW_PROGRESS )
    else()
      Message( STATUS "model ${_FILE} already loaded..." )
    endif()
  endforeach( _FILE )
endmacro()


#####################################################################################
# Macro to add custom build for Spir-V
# _SOURCE can be more than one file (.vert + .frag)
# _OUTPUT is the .spv file, resulting from the linkage
#
macro( _compile_GLSL _SOURCE _OUTPUT SOURCE_LIST )
  LIST( APPEND ${SOURCE_LIST} ${_SOURCE} )
  Message( STATUS "${GLSLANGVALIDATOR} -o ${_OUTPUT} -V ${_SOURCE}" )
  Message( STATUS "${_OUTPUT} : ${_SOURCE}" )
  if( GLSLANGVALIDATOR )
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${_OUTPUT}
      COMMAND echo ${GLSLANGVALIDATOR} -o ${_OUTPUT} -V ${_SOURCE}
      COMMAND ${GLSLANGVALIDATOR} -o ${_OUTPUT} -V ${_SOURCE}
      MAIN_DEPENDENCY ${_SOURCE}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      )
  else( GLSLANGVALIDATOR )
    MESSAGE( WARNING "could not find GLSLANGVALIDATOR to compile shaders" )
  endif( GLSLANGVALIDATOR )
endmacro()


#####################################################################################
# OpenGL libraries
#
find_package(OpenGL)


#####################################################################################
# Create list of source files 
include_directories(
  ${SHARED_DATA_PATH}
)
set( PLATFORM_LIBRARIES )
if( UNIX )
LIST( APPEND PLATFORM_LIBRARIES
  ${OPENGL_LIBRARY}
  "-lpthread -lm"
  "-ldl"
)
else( UNIX )
   LIST( APPEND PLATFORM_LIBRARIES
     ${OPENGL_LIBRARY}
   )
endif( UNIX )

set( COMMON_SOURCE_FILES )
LIST( APPEND COMMON_SOURCE_FILES
  ${SHARED_DATA_PATH}/common.h
  ${SHARED_DATA_PATH}/tools.h
  ${SHARED_DATA_PATH}/resources.h
  ${SHARED_DATA_PATH}/resources.rc
)
file(GLOB MODEL_SOURCE_FILES 
  ${SHARED_DATA_PATH}/models/*.cpp 
  ${SHARED_DATA_PATH}/models/*.h
)

# ImGui & ImPlot source files
set( GUI_SOURCE_FILES )
LIST( APPEND GUI_SOURCE_FILES
  ${SHARED_DATA_PATH}/imgui/imconfig.h
  ${SHARED_DATA_PATH}/imgui/imgui.cpp
  ${SHARED_DATA_PATH}/imgui/imgui.h
  ${SHARED_DATA_PATH}/imgui/imgui_draw.cpp
  ${SHARED_DATA_PATH}/imgui/imgui_impl_glfw.cpp
  ${SHARED_DATA_PATH}/imgui/imgui_impl_glfw.h
  ${SHARED_DATA_PATH}/imgui/imgui_impl_opengl3.cpp
  ${SHARED_DATA_PATH}/imgui/imgui_impl_opengl3.h
  ${SHARED_DATA_PATH}/imgui/imgui_internal.h
  ${SHARED_DATA_PATH}/imgui/imgui_widgets.cpp
  ${SHARED_DATA_PATH}/imgui/imstb_rectpack.h
  ${SHARED_DATA_PATH}/imgui/imstb_textedit.h
  ${SHARED_DATA_PATH}/imgui/imstb_truetype.h
  ${SHARED_DATA_PATH}/implot/implot.cpp
  ${SHARED_DATA_PATH}/implot/implot.h
  ${SHARED_DATA_PATH}/implot/implot_internal.h
  ${SHARED_DATA_PATH}/implot/implot_items.cpp
)

#####################################################################################
# Process PGR2 projects
FILE (GLOB samples "./*")
FOREACH( sample ${samples} )
  if( IS_DIRECTORY ${sample} )
    if( EXISTS ${sample}/CMakeLists.txt )
      # extract the name from the path
      string( REGEX REPLACE "^.*/([^/]*)$" "\\1" SAMPLE_NAME ${sample} )
      if( (NOT (${SAMPLE_NAME} MATCHES "^__")) AND (NOT(${SAMPLE_NAME} STREQUAL "build_all")) AND (NOT(${SAMPLE_NAME} STREQUAL "shared")) AND (NOT(${SAMPLE_NAME} STREQUAL "downloaded_resources")))
        # build the option to include the project or not
        set( "BUILD_${SAMPLE_NAME}" TRUE CACHE BOOL "Include the ${SAMPLE_NAME} sample" )
        # add the project
        if( BUILD_${SAMPLE_NAME})
          add_subdirectory( ./${SAMPLE_NAME} ./${SAMPLE_NAME})
        endif()
      endif()
    endif()
  endif()
ENDFOREACH( sample ${samples} )


#####################################################################################
# Create defines 
#
add_definitions( -DRESOURCE_DIRECTORY="${RESOURCE_DIRECTORY}/" )
if( MSVC )
  add_definitions( -D_CRT_SECURE_NO_WARNINGS )
endif( MSVC )


#####################################################################################
# Print cmake configuration info
#
Message( STATUS "BASE_PATH = ${BASE_PATH}" )
Message( STATUS "CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}" )
