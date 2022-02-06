cmake_minimum_required( VERSION 2.8 )
get_filename_component( PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME )
Project( ${PROJECT_NAME} )
Message( STATUS "-----------------------------------------------------------------" )
Message( STATUS "Processing project ${PROJECT_NAME}:" )

_add_project_definitions( ${PROJECT_NAME} )

#####################################################################################
# Add requested packages
#
_add_package_GLFW()
_add_package_GLEW()

#####################################################################################
# Add source files and shaders
#
file( GLOB SOURCE_FILES *.cpp *.hpp *.inl *.h *.c )
file( GLOB SHADER_FILES *.vs *.fs *.gs *.tcs *.tes )

#####################################################################################
# Some build related definitions
#
if(WIN32)
  add_definitions( /wd4996 ) #remove printf warning
  add_definitions( /wd4244 ) #remove double to float conversion warning
  add_definitions( /wd4305 ) #remove double to float truncation warning
endif()

#####################################################################################
# Add executables
#
add_executable( ${PROJECT_NAME} 
			    ${SOURCE_FILES}
			    ${COMMON_SOURCE_FILES}
			    ${PACKAGE_SOURCE_FILES}
				${GUI_SOURCE_FILES}
			    ${SHADER_FILES}
			    ${MODEL_SOURCE_FILES}
)

#####################################################################################
# Project filters
#
source_group( common FILES 
  ${COMMON_SOURCE_FILES}
  ${PACKAGE_SOURCE_FILES}
)
source_group( shaders FILES 
  ${SHADER_FILES}
)
source_group( models FILES 
  ${MODEL_SOURCE_FILES}
)
source_group( imgui FILES
  ${GUI_SOURCE_FILES}
)

#####################################################################################
# Linkages
#
target_link_libraries( ${PROJECT_NAME} optimized
    ${LIBRARIES_OPTIMIZED}
    ${PLATFORM_LIBRARIES}
)
target_link_libraries( ${PROJECT_NAME} debug
    ${LIBRARIES_DEBUG}
    ${PLATFORM_LIBRARIES}
)

#####################################################################################
# Copies binaries that need to be in output directory
#
_copy_binaries_to_target( ${PROJECT_NAME} )
