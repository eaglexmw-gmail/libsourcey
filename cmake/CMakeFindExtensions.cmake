#
### Macro: find_library_extended
#
# Finds libraries with finer control over search paths
# for compilers with multiple configuration types.
#
macro(find_library_extended prefix)
  include(CMakeParseArguments REQUIRED)
  # cmake_parse_arguments(prefix options singleValueArgs multiValueArgs ${ARGN})
  cmake_parse_arguments(${prefix} "" ""
    "NAMES;NAMES_DEBUG;NAMES_RELEASE;PATHS_DEBUG;PATHS_RELEASE;PATHS"
    ${ARGN}
    )

  # Reloading to ensure build always passes and picks up changes
  # This is more expensive but proves useful for fragmented libraries like WebRTC
  unset(${prefix}_LIBRARY_DEBUG CACHE)
  find_library(${prefix}_LIBRARY_DEBUG
    NAMES
      ${${prefix}_NAMES_DEBUG}
      ${${prefix}_NAMES}
    PATHS
      ${${prefix}_PATHS_DEBUG}
      ${${prefix}_PATHS}
    )

  #if(NOT ${prefix}_PATHS_RELEASE)
  #  list(APPEND ${prefix}_PATHS_RELEASE ${${prefix}_PATHS})
  #endif()

  unset(${prefix}_LIBRARY_RELEASE CACHE)
  find_library(${prefix}_LIBRARY_RELEASE
    NAMES
      ${${prefix}_NAMES_RELEASE}
      ${${prefix}_NAMES}
    PATHS
      ${${prefix}_PATHS_RELEASE}
      ${${prefix}_PATHS}
    )

  include(${CMAKE_ROOT}/Modules/SelectLibraryConfigurations.cmake)
  select_library_configurations(${prefix})

  # print_module_variables(${prefix})

  # message("*** Find library for ${prefix}")
  # message("Debug Library: ${${prefix}_LIBRARY_DEBUG}")
  # message("Release Library: ${${prefix}_LIBRARY_RELEASE}")
  # message("Library: ${${prefix}_LIBRARY}")
  # message("Debug Paths: ${${prefix}_PATHS_RELEASE}")
  # message("Release Paths: ${${prefix}_PATHS_DEBUG}")
  # message("Paths: ${${prefix}_PATHS}")
  # message("Debug Names: ${${prefix}_NAMES_RELEASE}")
  # message("Release Names: ${${prefix}_NAMES_DEBUG}")
  # message("Names: ${${prefix}_NAMES}")

endmacro(find_library_extended)


#
### Macro: set_component_alias
#
# Sets the current module component alias variables.
#
macro(set_component_alias module component)
  set(ALIAS                   ${module}_${component})
  set(ALIAS_FOUND             ${ALIAS}_FOUND)
  set(ALIAS_LIBRARIES         ${ALIAS}_LIBRARIES)
  set(ALIAS_RELEASE_LIBRARIES ${ALIAS}_RELEASE_LIBRARIES)
  set(ALIAS_DEBUG_LIBRARIES   ${ALIAS}_DEBUG_LIBRARIES)
  set(ALIAS_INCLUDE_DIRS      ${ALIAS}_INCLUDE_DIRS)
  set(ALIAS_LIBRARY_DIRS      ${ALIAS}_LIBRARY_DIRS)
  set(ALIAS_DEFINITIONS       ${ALIAS}_CFLAGS_OTHER)
  set(ALIAS_VERSION           ${ALIAS}_VERSION)
endmacro()


#
### Macro: set_module_found
#
# Marks the given module as found if all required components are present.
#
macro(set_module_found module)
  set(${module}_FOUND FALSE)
  # set(${module}_FOUND FALSE PARENT_SCOPE)

  # Compile the list of required vars
  set(_${module}_REQUIRED_VARS ${module}_LIBRARIES) # ${module}_INCLUDE_DIRS
  foreach (component ${${module}_FIND_COMPONENTS})
    # NOTE: Not including XXX_INCLUDE_DIRS as required var since it may be empty
    list(APPEND _${module}_REQUIRED_VARS ${module}_${component}_LIBRARIES) # ${module}_${component}_INCLUDE_DIRS
    if (NOT ${module}_${component}_FOUND)
      message(FATAL_ERROR "Required ${module} component ${component} missing. Please recompile ${module} with ${component} enabled.")
    #else()
      # message(STATUS "  - Required ${module} component ${component} found.")
    endif()
  endforeach()

  # Cache the vars.
  set(${module}_INCLUDE_DIRS   ${${module}_INCLUDE_DIRS} CACHE STRING   "The ${module} include directories." FORCE)
  set(${module}_LIBRARY_DIRS   ${${module}_LIBRARY_DIRS} CACHE STRING   "The ${module} library directories." FORCE)
  set(${module}_LIBRARIES      ${${module}_LIBRARIES}    CACHE STRING   "The ${module} libraries." FORCE)
  set(${module}_FOUND          ${${module}_FOUND}        CACHE BOOLEAN  "The ${module} found status." FORCE)
  # set(${module}_INCLUDE_DIRS ${${module}_INCLUDE_DIRS} PARENT_SCOPE)
  # set(${module}_LIBRARY_DIRS ${${module}_LIBRARY_DIRS} PARENT_SCOPE)
  # set(${module}_LIBRARIES    ${${module}_LIBRARIES}  PARENT_SCOPE)
  # set(${module}_FOUND        ${${module}_FOUND}    PARENT_SCOPE)

  # Ensure required variables have been set, or fail in error.
  # NOTE: Disabling find_package_handle_standard_args check since it's always
  # returning false even when variables are set. Maybe an internal CMake issue?
  # if (${module}_FIND_REQUIRED)
  #
  #   # Give a nice error message if some of the required vars are missing.
  #   include(FindPackageHandleStandardArgs)
  #   find_package_handle_standard_args(${module} DEFAULT_MSG ${_${module}_REQUIRED_VARS})
  # endif()

  # Set the module as found.
  if (${module}_LIBRARIES)
    set(${module}_FOUND TRUE)
    # set(${module}_FOUND TRUE PARENT_SCOPE)
  else()
    message("Failed to locate ${module}. Please specify paths manually.")
  endif()

  mark_as_advanced(${module}_INCLUDE_DIRS
                   ${module}_LIBRARY_DIRS
                   ${module}_LIBRARIES
                   ${module}_DEFINITIONS
                   ${module}_FOUND)
endmacro()


#
### Macro: set_component_found
#
# Marks the given component as found if both *_LIBRARIES AND *_INCLUDE_DIRS are present.
#
macro(set_component_found module component)
  set_component_alias(${module} ${component})

  # message("${ALIAS_LIBRARIES}=${${ALIAS_LIBRARIES}}")
  # message("${ALIAS_INCLUDE_DIRS}=${${ALIAS_INCLUDE_DIRS}}")
  # message("${ALIAS_LIBRARY_DIRS}=${${ALIAS_LIBRARY_DIRS}}")

  #if (${module}_${component}_LIBRARIES AND ${module}_${component}_INCLUDE_DIRS)
  if (${ALIAS_LIBRARIES}) # AND ${ALIAS_INCLUDE_DIRS} (XXX_INCLUDE_DIRS may be empty)
    # message(STATUS "  - ${module} ${component} found.")
    set(${ALIAS_FOUND} TRUE)
    # set(${ALIAS_FOUND} TRUE PARENT_SCOPE)

    # Add component vars to the parent module lists
    append_unique_list(${module}_INCLUDE_DIRS ${ALIAS_INCLUDE_DIRS})
    append_unique_list(${module}_LIBRARY_DIRS ${ALIAS_LIBRARY_DIRS})
    append_unique_list(${module}_LIBRARIES    ${ALIAS_LIBRARIES})
    append_unique_list(${module}_DEFINITIONS  ${ALIAS_DEFINITIONS})

    # set(${module}_INCLUDE_DIRS ${${module}_INCLUDE_DIRS} PARENT_SCOPE)
    # set(${module}_LIBRARY_DIRS ${${module}_LIBRARY_DIRS} PARENT_SCOPE)
    # set(${module}_LIBRARIES  ${${module}_LIBRARIES}  PARENT_SCOPE)
    # set(${module}_DEFINITIONS  ${${module}_DEFINITIONS}  PARENT_SCOPE)

    # message("Find Component Paths=${module}:${component}:${library}:${header}")
    # message("${ALIAS_INCLUDE_DIRS}=${${ALIAS_INCLUDE_DIRS}}")
    # message("${ALIAS_RELEASE_LIBRARIES}=${${ALIAS_RELEASE_LIBRARIES}}")
    # message("${ALIAS_DEBUG_LIBRARIES}=${${ALIAS_DEBUG_LIBRARIES}}")
    # message("${ALIAS_LIBRARIES}=${${ALIAS_LIBRARIES}}")
    # message("${module}_INCLUDE_DIRS=${${module}_INCLUDE_DIRS}")
    # message("${module}_LIBRARIES=${${module}_LIBRARIES}")

    # Only mark as advanced when found
    mark_as_advanced(${ALIAS_INCLUDE_DIRS}
                     ${ALIAS_LIBRARY_DIRS})

  else()
    # NOTE: an error message will be displayed in set_module_found
    # if the module is REQUIRED
    # message(STATUS "  - ${module} ${component} not found.")
  endif()

  set(HAVE_${ALIAS} ${${ALIAS_FOUND}})

  get_directory_property(hasParent PARENT_DIRECTORY)
  if(hasParent)
    set(HAVE_${ALIAS} ${${ALIAS_FOUND}} PARENT_SCOPE)
  endif()

  mark_as_advanced(${ALIAS_FOUND}
                   ${ALIAS_DEBUG_LIBRARIES}
                   ${ALIAS_RELEASE_LIBRARIES}
                   ${ALIAS_LIBRARIES}
                   ${ALIAS_DEFINITIONS}
                   ${ALIAS_VERSION})
endmacro()


#
### Macro: set_module_notfound
#
# Marks the given component as not found, and resets the cache for find_path and find_library results.
#
macro(set_module_notfound module)
  #message(STATUS "  - Setting ${module} not found.")
  set(${module}_FOUND FALSE)
  #set(${module}_FOUND FALSE PARENT_SCOPE)

  if (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
    set(${module}_RELEASE_LIBRARIES "")
    set(${module}_DEBUG_LIBRARIES "")
    set(${module}_LIBRARIES "")
    #set(${module}_RELEASE_LIBRARIES "" PARENT_SCOPE)
    #set(${module}_DEBUG_LIBRARIES "" PARENT_SCOPE)
    #set(${module}_LIBRARIES "" PARENT_SCOPE)
  else()
    set(${module}_LIBRARIES ${ALIAS_LIBRARIES}-NOTFOUND)
    #set(${module}_LIBRARIES ${ALIAS_LIBRARIES}-NOTFOUND PARENT_SCOPE)
  endif()

endmacro()


#
### Macro: set_component_notfound
#
# Marks the given component as not found, and resets the cache for find_path and find_library results.
#
macro(set_component_notfound module component)
  set_component_alias(${module} ${component})

  #message(STATUS "  - Setting ${module} ${component} not found.")
  set(${ALIAS_FOUND} FALSE)
  #set(${ALIAS_FOUND} FALSE PARENT_SCOPE)
  set(${ALIAS_INCLUDE_DIRS} ${ALIAS_INCLUDE_DIRS}-NOTFOUND)

  if (${module}_MULTI_CONFIGURATION AND (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE))
    set(${ALIAS_RELEASE_LIBRARIES} ${ALIAS_RELEASE_LIBRARIES}-NOTFOUND)
    set(${ALIAS_DEBUG_LIBRARIES} ${ALIAS_DEBUG_LIBRARIES}-NOTFOUND)
    set(${ALIAS_LIBRARIES} "") #${module}_${component}_LIBRARIES-NOTFOUND)
    #set(${ALIAS_RELEASE_LIBRARIES} ${ALIAS_RELEASE_LIBRARIES}-NOTFOUND PARENT_SCOPE)
    #set(${ALIAS_DEBUG_LIBRARIES} ${ALIAS_DEBUG_LIBRARIES}-NOTFOUND PARENT_SCOPE)
    #set(${ALIAS_LIBRARIES} "") #${module}_${component}_LIBRARIES-NOTFOUND PARENT_SCOPE)
  else()
    set(${ALIAS_LIBRARIES} ${ALIAS_LIBRARIES}-NOTFOUND)
    #set(${ALIAS_LIBRARIES} ${ALIAS_LIBRARIES}-NOTFOUND PARENT_SCOPE)
  endif()

endmacro()


#
### Macro: find_component_paths
#
# Finds the given component library and include paths.
#
macro(find_component_paths module component library header)
  # message(STATUS "Find Component Paths=${module}:${component}:${library}:${header}")

  # Reset alias namespace (force recheck)
  set_component_alias(${module} ${component})
  set_component_notfound(${module} ${component})

  find_path(${ALIAS_INCLUDE_DIRS} ${header}
    PATHS
      ${${module}_INCLUDE_DIR}   # try find from root module include dir
      ${${module}_INCLUDE_HINTS} # try find from root module include hints
    PATH_SUFFIXES
      ${${module}_INCLUDE_SUFFIXES} # try find from root module include suffixes
  )

  # print_module_variables(LibSourcey)

  # Create a Debug and a Release list for multi configuration builds.
  # NOTE: <module>_CONFIGURATION_TYPES must be set to use this.
  if (${module}_MULTI_CONFIGURATION AND (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE))
    find_library(${ALIAS_RELEASE_LIBRARIES}
      NAMES
        ${library}
      PATHS
        ${${ALIAS_LIBRARY_DIRS}}
        ${${module}_LIBRARY_DIR}
        ${${module}_LIBRARY_HINTS}
      PATH_SUFFIXES
        ${${module}_LIBRARY_SUFFIXES}
    )
    find_library(${ALIAS_DEBUG_LIBRARIES}
      NAMES
        ${library}d
      PATHS
        ${${ALIAS_LIBRARY_DIRS}}
        ${${module}_LIBRARY_DIR}
        ${${module}_LIBRARY_HINTS}
      PATH_SUFFIXES
        ${${module}_LIBRARY_SUFFIXES}
    )
    # if (${ALIAS_RELEASE_LIBRARIES})
    #   list(APPEND ${ALIAS_LIBRARIES} "optimized" ${${ALIAS_RELEASE_LIBRARIES}})
    # endif()
    # if (${ALIAS_DEBUG_LIBRARIES})
    #   list(APPEND ${ALIAS_LIBRARIES} "debug" ${${ALIAS_DEBUG_LIBRARIES}})
    # endif()
    include(${CMAKE_ROOT}/Modules/SelectLibraryConfigurations.cmake)
    select_library_configurations(${ALIAS})
  else()
    find_library(${ALIAS_LIBRARIES}
      NAMES
        ${library}
      PATHS
        ${${ALIAS_LIBRARY_DIRS}}
        ${${module}_LIBRARY_DIR}
        ${${module}_LIBRARY_HINTS}
      PATH_SUFFIXES
        ${${module}_LIBRARY_SUFFIXES}
    )
  endif()

  set_component_found(${module} ${component})
endmacro()


#
### Macro: find_component
#
# Checks for the given component by invoking pkg-config and then looking up the
# libraries and include directories.
#
macro(find_component module component pkgconfig library header)
  # message("Find Component=${module}:${component}:${pkgconfig}:${library}:${header}")

  # Reset component alias values (force recheck)
  set_component_alias(${module} ${component})

  # Use pkg-config to obtain directories for the find_path() and find_library() calls.
  find_package(PkgConfig QUIET)
  if (PKG_CONFIG_FOUND)
    pkg_search_module(${ALIAS} ${pkgconfig} QUIET)
    # message(STATUS "Find Component PkgConfig=${ALIAS}:${${ALIAS}_FOUND}:${${ALIAS}_LIBRARIES}:${${ALIAS}_INCLUDE_DIRS}:${${ALIAS}_LIBRARY_DIRS}:${${ALIAS}_LIBDIR}:${${ALIAS}_INCLUDEDIR}")
  endif()

  # message(STATUS "${ALIAS_FOUND}=${${ALIAS_FOUND}}")
  # message(STATUS "${ALIAS_LIBRARIES}=${${ALIAS_LIBRARIES}}")
  # message(STATUS "${ALIAS_INCLUDE_DIRS}=${${ALIAS_INCLUDE_DIRS}}")

  if(NOT ${ALIAS_FOUND})
    # message(STATUS "  - ${module} ${component} pkg-config not found, searching...")
    find_component_paths(${module} ${component} ${library} ${header})
  else()
    # message(STATUS "  - ${module} ${component} pkg-config found.")
    set_component_found(${module} ${component})
  endif()
endmacro()
