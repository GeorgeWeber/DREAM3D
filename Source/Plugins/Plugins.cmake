

if( "${DREAM3D_EXTRA_PLUGINS}" STREQUAL "")
 set(DREAM3D_EXTRA_PLUGINS "" CACHE STRING "")
else()
  set(DREAM3D_EXTRA_PLUGINS ${DREAM3D_EXTRA_PLUGINS} CACHE STRING "")
endif()


# --------------------------------------------------------------------
# This function optionally compiles a named plugin when compiling DREAM3D
# This function will add in an Option "DREAM3D_BUILD_PLUGIN_${NAME} which
# the programmer can use to enable/disable the compiling of specific plugins
# Arguments:
# PLUGIN_NAME The name of the Plugin
# PLUGIN_SOURCE_DIR The source directory for the plugin
function(DREAM3D_COMPILE_PLUGIN)
    set(options)
    set(oneValueArgs PLUGIN_NAME PLUGIN_SOURCE_DIR)
    cmake_parse_arguments(PLUG  "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    option(DREAM3D_BUILD_PLUGIN_${PLUG_PLUGIN_NAME} "Build the ${PLUG_PLUGIN_NAME}" ON)
    if(DREAM3D_BUILD_PLUGIN_${PLUG_PLUGIN_NAME})
        message(STATUS "Plugin  [ENABLED]: ${PLUG_PLUGIN_NAME}")
        add_subdirectory(${PLUG_PLUGIN_SOURCE_DIR} ${PROJECT_BINARY_DIR}/Plugins/${PLUG_PLUGIN_NAME})
        #- Now set up the dependency between the main application and each of the plugins so that
        #- things like Visual Studio are forced to rebuild the plugins when launching
        #- the DREAM3D application
        if(DREAM3D_BUILD_PLUGIN_${p})
            add_dependencies(DREAM3D ${p})
        endif()
    else()
        message(STATUS "Plugin [DISABLED]: ${PLUG_PLUGIN_NAME}")
    endif()
endfunction()

# --------------------------------------------------------------------
#
#
#
#
function(DREAM3D_ADD_PLUGINS)
    set(options)
    set(multiValueArgs PLUGIN_NAMES)
    cmake_parse_arguments(PLUG  "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
    get_filename_component(DREAM3D_PARENT_DIR  ${DREAM3DProj_SOURCE_DIR} DIRECTORY)
    #message(STATUS "DREAM3D_PARENT_DIR: ${DREAM3D_PARENT_DIR}")

    #-- Attempt to look in our local source directory for the plugin. Anywhere else
    # and the user will have to put the entire path into CMake manually.
    foreach(d3dPlugin ${PLUG_PLUGIN_NAMES})

      # message(STATUS "Evaluating Plugin: ${d3dPlugin}...")

      if(DEFINED ${d3dPlugin}_SOURCE_DIR AND "${${d3dPlugin}_SOURCE_DIR}" STREQUAL "")
        set(pluginSearchDir ${PROJECT_CODE_DIR}/Plugins/${d3dPlugin})
        set(${d3dPlugin}_SOURCE_DIR ${pluginSearchDir} CACHE PATH "")
      endif()


      if(NOT DEFINED ${d3dPlugin}_SOURCE_DIR)
          set(pluginSearchDir ${PROJECT_CODE_DIR}/Plugins/${d3dPlugin})
          if(EXISTS ${pluginSearchDir})
            set(${d3dPlugin}_SOURCE_DIR ${pluginSearchDir} CACHE PATH "")
            message(STATUS "Plugin: Defining ${d3dPlugin}_SOURCE_DIR to ${${d3dPlugin}_SOURCE_DIR}")
          else()

            set(pluginSearchDir ${DREAM3D_PARENT_DIR}/${d3dPlugin})
            if(EXISTS ${pluginSearchDir})
                set(${d3dPlugin}_SOURCE_DIR ${pluginSearchDir} CACHE PATH "")
                include_directories( ${pluginSearchDir} )
                include_directories(${DREAM3D_PARENT_DIR})
                message(STATUS "Plugin: Defining ${d3dPlugin}_SOURCE_DIR to ${${d3dPlugin}_SOURCE_DIR}")
            endif()
            set(pluginSearchDir ${DREAM3D_PARENT_DIR}/DREAM3D_Plugins/${d3dPlugin})
            if(EXISTS ${pluginSearchDir})
                set(${d3dPlugin}_SOURCE_DIR ${pluginSearchDir} CACHE PATH "")
                include_directories(${DREAM3D_PARENT_DIR}/DREAM3D_Plugins)
                include_directories( ${pluginSearchDir} )
                message(STATUS "Plugin: Defining ${d3dPlugin}_SOURCE_DIR to ${${d3dPlugin}_SOURCE_DIR}")
            endif()
          endif()
      endif()


    # Now that we have defined where the user's plugin directory is at we
    # need to make sure it have a CMakeLists.txt file in it
      if(EXISTS ${${d3dPlugin}_SOURCE_DIR}/CMakeLists.txt)
        set(${d3dPlugin}_IMPORT_FILE ${d3dPlugin}_SOURCE_DIR/CMakeLists.txt)
      endif()


    # By this point we should have everything defined and ready to go...
      if(DEFINED ${d3dPlugin}_SOURCE_DIR AND DEFINED ${d3dPlugin}_IMPORT_FILE)
          #message(STATUS "Plugin: Adding Plugin ${${d3dPlugin}_SOURCE_DIR}")
          DREAM3D_COMPILE_PLUGIN(PLUGIN_NAME ${d3dPlugin}
                                 PLUGIN_SOURCE_DIR ${${d3dPlugin}_SOURCE_DIR})
      else()
          set(${d3dPlugin}_SOURCE_DIR ${pluginSearchDir} CACHE PATH "" FORCE)
          message(FATAL_ERROR "Plugin: The directory for plugin ${d3dPlugin} was not found. Please set this directory manually")
      endif()

    endforeach()

endfunction()


#-----------------
# These are the core plugins that need to be built
set(DREAM3D_BASE_PLUGINS
  Generic
  IO
  ImageIO
  OrientationAnalysis
  Processing
  Reconstruction
  Sampling
  Statistics
  SurfaceMeshing
  SyntheticBuilding
  EMMPM
)


get_filename_component(DREAM3D_PARENT_DIR  ${DREAM3DProj_SOURCE_DIR} DIRECTORY)
include_directories(${DREAM3D_PARENT_DIR}/DREAM3D_Plugins)
include_directories(${DREAM3D_PARENT_DIR})


message(STATUS "Base plugins being compiled ......")
DREAM3D_ADD_PLUGINS(PLUGIN_NAMES ${DREAM3D_BASE_PLUGINS})


message(STATUS "User defined plugins being compiled ......")
DREAM3D_ADD_PLUGINS(PLUGIN_NAMES ${DREAM3D_EXTRA_PLUGINS})

