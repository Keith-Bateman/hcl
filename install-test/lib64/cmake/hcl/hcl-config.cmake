# This will create IMPORTED targets for Hcl. The executables will be
# hcl::<exe-name>-bin (e.g., hcl::hcl-bin) and the library will
# be hcl::hcl.

include("${CMAKE_CURRENT_LIST_DIR}/hcl-config-version.cmake")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/modules")
list(APPEND CMAKE_MODULE_PATH "")

#include(GNUInstallDirs)
include(ExternalProject)
include(hcl-utils)
include(CMakePackageConfigHelpers)


set(HCL_VERSION ${PACKAGE_VERSION})

# Record compiler information
set(HCL_C_COMPILER "/usr/tce/bin/gcc")
set(HCL_CXX_COMPILER "/usr/tce/bin/g++")

set(HCL_C_FLAGS " -fPIC -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-deprecated-declarations")
set(HCL_CXX_FLAGS " -fPIC -Wall -Wextra -pedantic -Wno-unused-parameter -Wnon-virtual-dtor -Wno-deprecated-declarations")

set(HCL_C_STANDARD "11")
set(HCL_CXX_STANDARD "17")

set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Record the various flags and switches accumlated in HCL
set(HCL_GNU_LINUX )
set(HCL_HAS_STD_FILESYSTEM TRUE)
set(HCL_HAS_STD_FSTREAM_FD TRUE)

# Setup dependencies



####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was hcl-config.cmake.install.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../../../../install-test" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

# Now actually import the HCL target
set(_TMP_INCLUDE_DIRS "/usr/workspace/haridev/hcl-2/install-test/include")
foreach (_DIR ${_TMP_INCLUDE_DIRS})
  set_and_check(_INCLUDE_DIR "${_DIR}")
  list(APPEND HCL_INCLUDE_DIRS "${_INCLUDE_DIR}")
endforeach (_DIR "${_TMP_INCLUDE_DIRS}")

set(_TMP_LIBRARY_DIRS "")
foreach (_DIR ${_TMP_LIBRARY_DIRS})
  set_and_check(_LIBRARY_DIR "${_DIR}")
  list(APPEND HCL_LIBRARY_DIRS "${_LIBRARY_DIR}")
endforeach (_DIR ${_TMP_LIBRARY_DIRS})

if (NOT TARGET hcl::hcl)
  include(${CMAKE_CURRENT_LIST_DIR}/hcl-targets.cmake)
endif (NOT TARGET hcl::hcl)


set(HCL_COMMUNICATION THALLIUM)
if(HCL_COMMUNICATION STREQUAL "THALLIUM")
    find_package(thallium REQUIRED)
    if (${THALLIUM_FOUND})
        message(STATUS "[hcl] found thallium at ${THALLIUM_INCLUDE_DIRS}")
        include_directories(SYSTEM ${THALLIUM_INCLUDE_DIRS})
        link_directories(${THALLIUM_LIBRARY_PATH})
        target_link_libraries(hcl INTERFACE ${THALLIUM_LIBRARIES})
    else ()
        message(FATAL_ERROR "-- [hcl] thallium is needed for hcl build")
    endif ()
endif()

set(HCL_LOGGER CPP_LOGGER)
if (HCL_LOGGER STREQUAL "CPP_LOGGER")
    find_package(cpp-logger REQUIRED
                 HINTS ${CPP_LOGGER_DIR} ${cpp-logger_DIR}
                       ${CPP_LOGGER_PATH} ${cpp-logger_PATH}
                       $ENV{CPP_LOGGER_DIR} $ENV{cpp-logger_DIR}
                       $ENV{CPP_LOGGER_PATH} $ENV{cpp-logger_PATH})
    if (${cpp-logger_FOUND})
        message(STATUS "[hcl] found cpp-logger at ${CPP_LOGGER_INCLUDE_DIRS}")
        include_directories(SYSTEM ${CPP_LOGGER_INCLUDE_DIRS})
        link_directories(${CPP_LOGGER_LIBRARY_DIRS})
        target_link_libraries(hcl INTERFACE ${CPP_LOGGER_LIBRARIES})
    else ()
        message(FATAL_ERROR "-- [hcl] cpp-logger is not found but selected in cmake options for importing hcl")
    endif ()
endif()
set(HCL_PROFILER DLIO_PROFILER)
if(HCL_PROFILER STREQUAL "DLIO_PROFILER")
    find_package(dlio_profiler REQUIRED IMPORTED)
    if (${DLIO_PROFILER_FOUND})
        message(STATUS "[hcl] found dlio_profiler at ${DLIO_PROFILER_INCLUDE_DIRS}")
        include_directories(SYSTEM ${DLIO_PROFILER_INCLUDE_DIRS})
        link_directories(${DLIO_PROFILER_LIBRARY_DIRS})
        target_link_libraries(hcl INTERFACE ${DLIO_PROFILER_LIBRARIES})
    else ()
        message(FATAL_ERROR "-- [hcl] dlio_profiler is not found but selected in cmake options hcl build")
    endif ()
endif()

check_required_components(hcl)

set(HCL_LIBRARIES hcl)
