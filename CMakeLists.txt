cmake_minimum_required(VERSION 3.10.2)
#------------------------------------------------------------------------------
# Version information
#------------------------------------------------------------------------------

set(HCL_VERSION_MAJOR "0")
set(HCL_VERSION_MINOR "1")
set(HCL_VERSION_PATCH "0")
set(HCL_PACKAGE ${PROJECT_NAME})
set(HCL_PACKAGE_NAME ${PROJECT_NAME})
set(HCL_PACKAGE_VERSION "${HCL_VERSION_MAJOR}.${HCL_VERSION_MINOR}.${HCL_VERSION_PATCH}")
set(HCL_PACKAGE_VERSION_MAJOR "${HCL_VERSION_MAJOR}.${HCL_VERSION_MINOR}")
set(HCL_PACKAGE_VERSION_MINOR "${HCL_VERSION_PATCH}")
set(HCL_PACKAGE_STRING "${HCL_PACKAGE_NAME} ${HCL_PACKAGE_VERSION}")
set(HCL_PACKAGE_TARNAME "${HCL_PACKAGE}")

project(hcl LANGUAGES C CXX)

# Convenience defines
string(TOUPPER "${PROJECT_NAME}" UPPER_PROJECT_NAME)
string(TOLOWER "${PROJECT_NAME}" LOWER_PROJECT_NAME)

#------------------------------------------------------------------------------
# Internal Includes for header and libraries
#------------------------------------------------------------------------------
# Get installation directories -- these get used in various places;
# best to just make them available
option (HCL_LIBDIR_AS_LIB OFF)
if (NOT HCL_LIBDIR_AS_LIB)
    include(GNUInstallDirs)
endif ()
#
if (CMAKE_INSTALL_LIBDIR)
   message(STATUS "CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR}")
   set(HCL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
   set(HCL_INSTALL_INCLUDE_DIR
       ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})
   set(HCL_INSTALL_DOCDIR
       ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DOCDIR})
   set(HCL_INSTALL_SYSCONFDIR
       ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/modulefiles)
else ()
   set(HCL_LIBDIR "lib")
   set(HCL_INSTALL_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include")
   set(HCL_INSTALL_DOCDIR "${CMAKE_INSTALL_PREFIX}/doc")
   set(HCL_INSTALL_SYSCONFDIR "${CMAKE_INSTALL_PREFIX}/etc/modulefiles")
   message(STATUS "HCL_LIBDIR set to ${HCL_LIBDIR}")
endif ()

#------------------------------------------------------------------------------
# Internal Paths for cmake libraries and Setup install and output Directories
#------------------------------------------------------------------------------
# This sets where to look for dependent libraries
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_BINARY_DIR} ${CMAKE_INSTALL_PREFIX})
# This sets where to look for dependent library's cmake files
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/${HCL_LIBDIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/share/cmake)

#------------------------------------------------------------------------------
if (NOT HCL_EXTERNALLY_CONFIGURED)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE PATH "Single Directory for all Executables.")
    set(CMAKE_INCLUDE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/include CACHE PATH "Store the headers.")
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${HCL_LIBDIR} CACHE PATH "Single Directory for all Libraries")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${HCL_LIBDIR} CACHE PATH "Single Directory for all static libraries.")
endif ()

#-----------------------------------------------------------------------------
# Targets built within this project are exported at Install time for use
# by other projects.
#-----------------------------------------------------------------------------
if (NOT HCL_EXPORTED_TARGETS)
    set(HCL_EXPORTED_TARGETS "hcl-targets")
endif ()
#------------------------------------------------------------------------------
# Build options
#------------------------------------------------------------------------------
# option(ENABLE_HCL_DEBUG "Include debugging prints and logging" OFF) # This is not needed as we have CMAKE_BUILD_TYPE
# This is verbose, maybe an alternate might help simplify
#option(BUILD_URPC "Build HCL's URPC code" OFF)
#option(ENABLE_PERFFLOW "Build with PerfFlow Aspect support" OFF)
#option(ENABLE_UCX_DTL "Build HCL's UCX data transport layer" OFF)

# This option is off by default because non-developers should not use
# this option under normal circumstances.
option(HCL_WARNINGS_AS_ERRORS "Build with warnings promoted to errors." OFF)
mark_as_advanced(HCL_WARNINGS_AS_ERRORS)

option (HCL_USE_CLANG_LIBCXX OFF)
if (HCL_USE_CLANG_LIBCXX)
    set (GCC_TOOLCHAIN "")
endif (HCL_USE_CLANG_LIBCXX)

# Public
set(HCL_COMMUNICATION "THALLIUM" CACHE STRING "Communications to use for HCL")
set_property(CACHE HCL_COMMUNICATION PROPERTY STRINGS THALLIUM)
set(HCL_COMMUNICATION_PROTOCOL "OFI" CACHE STRING "Communication Protocol to use for HCL")
set_property(CACHE HCL_COMMUNICATION_PROTOCOL PROPERTY STRINGS OFI UCX)

if(HCL_COMMUNICATION STREQUAL "THALLIUM")
    set(HCL_COMMUNICATION_ENABLE_THALLIUM 1)
endif()

if(HCL_COMMUNICATION_PROTOCOL STREQUAL "OFI")
    set(HCL_COMMUNICATION_PROTOCOL_ENABLE_OFI 1)
elseif(HCL_COMMUNICATION_PROTOCOL STREQUAL "UCX")
    set(HCL_COMMUNICATION_PROTOCOL_ENABLE_UCX 1)
endif()

# Profiling and logging
set(HCL_PROFILER "NONE" CACHE STRING "Profiler to use for HCL")
set_property(CACHE HCL_PROFILER PROPERTY STRINGS DLIO_PROFILER NONE)
set(HCL_LOGGER "NONE" CACHE STRING "Logger to use for HCL")
set_property(CACHE HCL_LOGGER PROPERTY STRINGS CPP_LOGGER NONE)
set(HCL_LOGGER_LEVEL "NONE" CACHE STRING "Logging level to use for HCL")
set_property(CACHE HCL_LOGGER_LEVEL PROPERTY STRINGS DEBUG INFO WARN ERROR NONE)

# Testing
option (HCL_ENABLE_TESTING "Enable tests for HCL." OFF)

# Options validation
if (HCL_COMMUNICATION STREQUAL "RPCLIB" AND NOT HCL_COMMUNICATION_PROTOCOL STREQUAL "TCP")
  message(FATAL_ERROR "-- [${PROJECT_NAME}] HCL_COMMUNICATION with RPCLIB only supports HCL_COMMUNICATION_PROTOCOL as TCP")
endif()

if(HCL_PROFILER STREQUAL "DLIO_PROFILER")
    set(HCL_PROFILER_DLIO_PROFILER 1)
else()
    set(HCL_PROFILER_NONE 1)
endif()

if(HCL_LOGGER STREQUAL "CPP_LOGGER")
    set(HCL_LOGGER_CPP_LOGGER 1)
endif()

#------------------------------------------------------------------------------
# Compiler setup
#------------------------------------------------------------------------------
include(ExternalProject)
# It requires include(GNUInstallDirs) before if that is needed
include(SetupCompiler)

#------------------------------------------------------------------------------
# Build Type based configurations
#------------------------------------------------------------------------------

if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release") # Setting default as Release
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if (HCL_LOGGER STREQUAL "NONE")
        message(STATUS "[${PROJECT_NAME}] disabling logger as no logger is selected")
        set(HCL_LOGGER_NO_LOG 1)
    else()
        if (HCL_LOGGER_LEVEL STREQUAL "NONE")
          set(HCL_LOGGER_LEVEL "DEBUG")
        endif()
        
        if (HCL_LOGGER_LEVEL STREQUAL "TRACE")
          set(HCL_LOGGER_LEVEL_ERROR 1)
          set(HCL_LOGGER_LEVEL_WARN 1)
          set(HCL_LOGGER_LEVEL_INFO 1)
          set(HCL_LOGGER_LEVEL_DEBUG 1)
          set(HCL_LOGGER_LEVEL_TRACE 1)
        elseif (HCL_LOGGER_LEVEL STREQUAL "DEBUG")
          set(HCL_LOGGER_LEVEL_ERROR 1)
          set(HCL_LOGGER_LEVEL_WARN 1)
          set(HCL_LOGGER_LEVEL_INFO 1)
          set(HCL_LOGGER_LEVEL_DEBUG 1)
        elseif(HCL_LOGGER_LEVEL STREQUAL "ERROR")
          set(HCL_LOGGER_LEVEL_ERROR 1)
        elseif(HCL_LOGGER_LEVEL STREQUAL "WARN")
          set(HCL_LOGGER_LEVEL_ERROR 1)
          set(HCL_LOGGER_LEVEL_WARN 1)
        elseif(HCL_LOGGER_LEVEL STREQUAL "INFO")
          set(HCL_LOGGER_LEVEL_ERROR 1)
          set(HCL_LOGGER_LEVEL_WARN 1)
          set(HCL_LOGGER_LEVEL_INFO 1)
        else()
          set(HCL_LOGGER_NO_LOG 1)
        endif()
    endif()
else()
  if (HCL_LOGGER_LEVEL STREQUAL "NONE")
    set(HCL_LOGGER_LEVEL "WARN")
  endif()
        
  if (HCL_LOGGER_LEVEL STREQUAL "TRACE")
    set(HCL_LOGGER_LEVEL_ERROR 1)
    set(HCL_LOGGER_LEVEL_WARN 1)
    set(HCL_LOGGER_LEVEL_INFO 1)
    set(HCL_LOGGER_LEVEL_DEBUG 1)
    set(HCL_LOGGER_LEVEL_TRACE 1)
  elseif (HCL_LOGGER_LEVEL STREQUAL "DEBUG")
    set(HCL_LOGGER_LEVEL_ERROR 1)
    set(HCL_LOGGER_LEVEL_WARN 1)
    set(HCL_LOGGER_LEVEL_INFO 1)
    set(HCL_LOGGER_LEVEL_DEBUG 1)
  elseif(HCL_LOGGER_LEVEL STREQUAL "ERROR")
    set(HCL_LOGGER_LEVEL_ERROR 1)
  elseif(HCL_LOGGER_LEVEL STREQUAL "WARN")
    set(HCL_LOGGER_LEVEL_ERROR 1)
    set(HCL_LOGGER_LEVEL_WARN 1)
  elseif(HCL_LOGGER_LEVEL STREQUAL "INFO")
    set(HCL_LOGGER_LEVEL_ERROR 1)
    set(HCL_LOGGER_LEVEL_WARN 1)
    set(HCL_LOGGER_LEVEL_INFO 1)
  else()
    set(HCL_LOGGER_NO_LOG 1)
  endif()
endif()

if (NOT DEFINED BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIBS ON)
endif ()
#------------------------------------------------------------------------------
# Disallow in-source build
#------------------------------------------------------------------------------
if ("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR
            "${PROJECT_NAME} requires an out of source Build. "
            "Please create a separate binary directory and run CMake there.")
endif ()
#------------------------------------------------------------------------------
# Set a default build type if none was specified
#------------------------------------------------------------------------------
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
            "MinSizeRel" "RelWithDebInfo")
endif ()


function(hcl_install_headers public_headers current_dir)
    #message("-- [${PROJECT_NAME}] " "installing headers ${public_headers}")
    foreach (header ${public_headers})
        file(RELATIVE_PATH header_file_path "${PROJECT_SOURCE_DIR}/src" "${header}")
        #message("-- [${PROJECT_NAME}] " "installing header ${header_file_path}")
        get_filename_component(header_directory_path "${header_file_path}" DIRECTORY)
        install(
                FILES ${header}
                DESTINATION "include/${header_directory_path}"
        )
        file(COPY ${header}
                DESTINATION "${CMAKE_INCLUDE_OUTPUT_DIRECTORY}/${header_directory_path}")
    endforeach ()
    set(INCLUDE_INSTALL_DIRS ${public_headers})
endfunction()


#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

# Install Dependencies
# =============================================================================
if (HCL_BUILD_DEPENDENCIES)
  add_subdirectory(dependency)

else()


#-----------------------------------------------------------------------------
# Source for HCL
#-----------------------------------------------------------------------------
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
include_directories(${CMAKE_SOURCE_DIR}/include)  # public header
include_directories(${CMAKE_SOURCE_DIR}/src)  # private header
include_directories(${CMAKE_BINARY_DIR}/include)  # build header

set(HCL_SRC   ${PROJECT_SOURCE_DIR}/src/hcl/common/configuration_manager.cpp
              ${PROJECT_SOURCE_DIR}/src/hcl/clock/global_clock.cpp
              ${PROJECT_SOURCE_DIR}/src/hcl/common/container.cpp
              ${PROJECT_SOURCE_DIR}/src/hcl/hcl_internal.cpp)
set(HCL_PRIVATE_HEADER  )
set(HCL_PUBLIC_HEADER   ${PROJECT_SOURCE_DIR}/include/hcl.h
                        ${PROJECT_SOURCE_DIR}/include/hcl/common/debug.h
                        ${PROJECT_SOURCE_DIR}/include/hcl/common/macros.h
                        ${PROJECT_SOURCE_DIR}/include/hcl/common/typedefs.h
                        ${PROJECT_SOURCE_DIR}/include/hcl/common/enumerations.h
                        ${PROJECT_SOURCE_DIR}/include/hcl/common/data_structures.h
                        ${PROJECT_SOURCE_DIR}/include/hcl/common/configuration_manager.h
                        ${PROJECT_SOURCE_DIR}/include/hcl/communication/rpc_lib.h
                        ${PROJECT_SOURCE_DIR}/include/hcl/unordered_map/unordered_map.h
                        ${PROJECT_SOURCE_DIR}/include/hcl/concurrent/unordered_map/unordered_map.h
                        ${PROJECT_SOURCE_DIR}/include/hcl/concurrent/queue/queue.h
                        ${PROJECT_SOURCE_DIR}/include/hcl/map/map.h
                        ${PROJECT_SOURCE_DIR}/include/hcl/multimap/multimap.h
                        ${PROJECT_SOURCE_DIR}/include/hcl/clock/global_clock.h
                        ${PROJECT_SOURCE_DIR}/include/hcl/queue/queue.h
                        ${PROJECT_SOURCE_DIR}/include/hcl/vector/vector.h
                        ${PROJECT_SOURCE_DIR}/include/hcl/priority_queue/priority_queue.h
                        ${PROJECT_SOURCE_DIR}/include/hcl/set/set.h
                        ${PROJECT_SOURCE_DIR}/include/hcl/sequencer/global_sequence.h
                        ${PROJECT_SOURCE_DIR}/include/hcl/hcl_internal.h
                        ${PROJECT_SOURCE_DIR}/include/hcl/common/container.h
                        ${PROJECT_SOURCE_DIR}/include/hcl/common/singleton.h 
                        ${PROJECT_SOURCE_DIR}/include/hcl/common/constants.h)
set(HCL_SRC_PRIVATE  
                    ${PROJECT_SOURCE_DIR}/include/hcl/communication/rpc_lib.cpp
                    ${PROJECT_SOURCE_DIR}/include/hcl/concurrent/queue/queue.cpp
                    ${PROJECT_SOURCE_DIR}/include/hcl/concurrent/skiplist/skiplist.cpp
                    ${PROJECT_SOURCE_DIR}/include/hcl/concurrent/unordered_map/unordered_map.cpp
                    ${PROJECT_SOURCE_DIR}/include/hcl/map/map.cpp
                    ${PROJECT_SOURCE_DIR}/include/hcl/multimap/multimap.cpp
                    ${PROJECT_SOURCE_DIR}/include/hcl/priority_queue/priority_queue.cpp
                    ${PROJECT_SOURCE_DIR}/include/hcl/queue/queue.cpp
                    ${PROJECT_SOURCE_DIR}/include/hcl/vector/vector.cpp
                    ${PROJECT_SOURCE_DIR}/include/hcl/set/set.cpp
                    ${PROJECT_SOURCE_DIR}/include/hcl/unordered_map/unordered_map.cpp
                    ${PROJECT_SOURCE_DIR}/include/hcl/communication/rpc_lib.cpp)

add_library(${PROJECT_NAME} SHARED ${HCL_SRC} ${HCL_PUBLIC_HEADER} ${HCL_PRIVATE_HEADER})
target_link_libraries(${PROJECT_NAME} ${DEPENDENCY_LIB})
set_target_properties(${PROJECT_NAME} PROPERTIES
    LINKER_LANGUAGE CXX
    VERSION ${HCL_PACKAGE_VERSION}
    SOVERSION 1
    PUBLIC_HEADER include/hcl.h)
target_compile_definitions(${PROJECT_NAME} PUBLIC HCL_HAS_CONFIG)
hcl_install_headers("${HCL_PUBLIC_HEADER};${HCL_SRC_PRIVATE}" ${CMAKE_CURRENT_SOURCE_DIR})

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

# Required Dependencies
# =============================================================================
set(DEPENDENCY_LIB -lpthread -lrt)
if(HCL_COMMUNICATION STREQUAL "THALLIUM")
    find_package(thallium REQUIRED)
    if (${THALLIUM_FOUND})
        message(STATUS "[${PROJECT_NAME}] found thallium at ${THALLIUM_INCLUDE_DIRS}")
        include_directories(SYSTEM ${THALLIUM_INCLUDE_DIRS})
        link_directories(${THALLIUM_LIBRARY_PATH})
        set(DEPENDENCY_LIB ${DEPENDENCY_LIB}  -L${THALLIUM_LIBRARY_PATH} ${THALLIUM_LIBRARIES})
    else ()
        message(FATAL_ERROR "-- [${PROJECT_NAME}] thallium is needed for ${PROJECT_NAME} build")
    endif ()
endif()

# Optional Dependencies
# =============================================================================
if (HCL_LOGGER STREQUAL "CPP_LOGGER")
    find_package(cpp-logger REQUIRED
                 HINTS ${CPP_LOGGER_DIR} ${cpp-logger_DIR}
                       ${CPP_LOGGER_PATH} ${cpp-logger_PATH}
                       $ENV{CPP_LOGGER_DIR} $ENV{cpp-logger_DIR}
                       $ENV{CPP_LOGGER_PATH} $ENV{cpp-logger_PATH})
    if (${cpp-logger_FOUND})
        message(STATUS "[${PROJECT_NAME}] found cpp-logger at ${CPP_LOGGER_INCLUDE_DIRS}")
        include_directories(SYSTEM ${CPP_LOGGER_INCLUDE_DIRS})
        set(DEPENDENCY_LIB ${DEPENDENCY_LIB} ${CPP_LOGGER_LIBRARIES})
    else ()
        message(FATAL_ERROR "-- [${PROJECT_NAME}] cpp-logger is not found but selected in cmake options for ${PROJECT_NAME} build")
    endif ()
endif()
if(HCL_PROFILER STREQUAL "DLIO_PROFILER")
    find_package(dlio_profiler REQUIRED IMPORTED)
    if (${DLIO_PROFILER_FOUND})
        message(STATUS "[${PROJECT_NAME}] found dlio_profiler at ${DLIO_PROFILER_INCLUDE_DIRS}")
        include_directories(SYSTEM ${DLIO_PROFILER_INCLUDE_DIRS})
        set(DEPENDENCY_LIB ${DEPENDENCY_LIB} ${DLIO_PROFILER_LIBRARIES})
    else ()
        message(FATAL_ERROR "-- [${PROJECT_NAME}] dlio_profiler is not found but selected in cmake options ${PROJECT_NAME} build")
    endif ()
endif()

target_link_libraries(${PROJECT_NAME} ${DEPENDENCY_LIB})

install(
        TARGETS ${PROJECT_NAME}
        EXPORT ${HCL_EXPORTED_TARGETS}
        LIBRARY DESTINATION ${HCL_INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${HCL_INSTALL_LIB_DIR}
        RUNTIME DESTINATION ${HCL_INSTALL_BIN_DIR}
)
#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------

#cmake_policy(SET CMP0079 NEW) # In case that we need more control over the target building order

if(HCL_ENABLE_TESTING)
  enable_testing()
    find_package(Catch2 REQUIRED)
    if (${Catch2_FOUND})
        message(STATUS "[${PROJECT_NAME}] found catch2")
        include_directories(SYSTEM Catch2::Catch2)
        set(TEST_LIBS ${DEPENDENCY_LIB} Catch2::Catch2)
    else ()
        message(FATAL_ERROR "-- [${PROJECT_NAME}] cpp-logger is not found but selected in cmake options for ${PROJECT_NAME} build")
    endif ()
    find_package(MPI REQUIRED COMPONENTS CXX C)
    if (${MPI_FOUND})
        message(STATUS "[${PROJECT_NAME}] found mpi at ${MPI_CXX_INCLUDE_DIRS}")
        # include_directories(${MPI_CXX_INCLUDE_DIRS})
        # set(TEST_LIBS ${TEST_LIBS}  ${MPI_CXX_LIBRARIES})
    else ()
        message(FATAL_ERROR "-- [${PROJECT_NAME}] mpi is needed for ${PROJECT_NAME} build")
    endif ()
  add_subdirectory(test)
endif()

#-----------------------------------------------------------------------------
# Configure the config.cmake file for the build directory
#-----------------------------------------------------------------------------
include(CMakePackageConfigHelpers)
configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/configure_files/${PROJECT_NAME}-config.cmake.build.in
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
        INSTALL_DESTINATION  ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cmake/${PROJECT_NAME}/${PROJECT_NAME}-config.cmake 
        PATH_VARS CMAKE_BINARY_DIR
)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/configure_files/${PROJECT_NAME}-config.cmake.install.in
        "${CMAKE_BINARY_DIR}/install/${PROJECT_NAME}-config.cmake"
        INSTALL_DESTINATION  ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cmake/${PROJECT_NAME}/install/${PROJECT_NAME}-config.cmake
        PATH_VARS CMAKE_BINARY_DIR
)

# configure_file(
#         ${CMAKE_CURRENT_SOURCE_DIR}/cmake/configure_files/${PROJECT_NAME}-config.cmake.install.in
#         ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cmake/${PROJECT_NAME}/install/${PROJECT_NAME}-config.cmake @ONLY
# )
install(
        FILES
        "${CMAKE_BINARY_DIR}/install/${PROJECT_NAME}-config.cmake"
        DESTINATION
        ${HCL_LIBDIR}/cmake/${PROJECT_NAME}
)
#-----------------------------------------------------------------------------
# Configure the ${PROJECT_NAME}-config-version .cmake file for the install directory
#-----------------------------------------------------------------------------
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/configure_files/${PROJECT_NAME}-config-version.cmake.in
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cmake/${PROJECT_NAME}/${PROJECT_NAME}-config-version.cmake @ONLY
)

install(
        FILES
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cmake/${PROJECT_NAME}/${PROJECT_NAME}-config-version.cmake
        DESTINATION
        ${HCL_LIBDIR}/cmake/${PROJECT_NAME}
)

export(EXPORT ${HCL_EXPORTED_TARGETS}
        FILE "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cmake/${PROJECT_NAME}/${PROJECT_NAME}Targets.cmake")

# Write the configure file
configure_file("${CMAKE_SOURCE_DIR}/cmake/configure_files/hcl_config.hpp.in"
               "${CMAKE_INCLUDE_OUTPUT_DIRECTORY}/hcl/hcl_config.hpp" @ONLY)


################################################################
# Install HCL
################################################################

include(CMakePackageConfigHelpers)

# Write the version file. This is independent of build/install tree.
write_basic_package_version_file(
  HclConfigVersion.cmake
  VERSION "${HCL_PACKAGE_VERSION}"
  COMPATIBILITY SameMajorVersion)

install(FILES
        "${CMAKE_INCLUDE_OUTPUT_DIRECTORY}/hcl/hcl_config.hpp"
        DESTINATION
        ${HCL_INSTALL_INCLUDE_DIR}/hcl)

install(EXPORT
        ${HCL_EXPORTED_TARGETS}
        DESTINATION
        ${HCL_LIBDIR}/cmake/${PROJECT_NAME}
        FILE
        ${HCL_EXPORTED_TARGETS}.cmake
        )

# Install export
install(EXPORT ${HCL_EXPORTED_TARGETS}
        NAMESPACE hcl::
        FILE HclTargets.cmake
        DESTINATION "${HCL_LIBDIR}/cmake/hcl")

# Install license and readme
install(FILES
        "${PROJECT_SOURCE_DIR}/NOTICE"
        "${PROJECT_SOURCE_DIR}/LICENSE"
        "${PROJECT_SOURCE_DIR}/README.md"
        DESTINATION
        ${HCL_INSTALL_DOCDIR})

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/hcl-utils.cmake"
        DESTINATION "${HCL_LIBDIR}/cmake/hcl")


###############################################################################
#                      Print out configuration summary
###############################################################################
include(hcl-utils)
if (CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
  set(HCL_GNU_LINUX TRUE)
endif ()

# NOTE: message() outputs to stderr by default. We now use a string to
# maintain this information and then have cmake echo it to stdout. The
# only side effects are that if you use the CMake GUI, you won't see
# this output anymore (they only report stderr) and that if you add
# something to the list, you must remember your newline!
set(_str "\n== HCL Configuration Summary ==\n\n")
string(APPEND _str "  PROJECT_SOURCE_DIR:   ${PROJECT_SOURCE_DIR}\n"
  "  PROJECT_BINARY_DIR:   ${PROJECT_BINARY_DIR}\n\n"
  "  CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}\n"
  "  CMAKE_BUILD_TYPE:     ${CMAKE_BUILD_TYPE}\n\n")
if (CMAKE_BUILD_TYPE MATCHES None)
  string(APPEND _str
    "  CXX FLAGS:            ${CMAKE_CXX_FLAGS}\n")
elseif (CMAKE_BUILD_TYPE MATCHES Release)
  string(APPEND _str
    "  CXX FLAGS:            ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}\n")
  string(APPEND _str
    "  C FLAGS:              ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}\n")
elseif (CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
  string(APPEND _str
    "  CXX FLAGS:            ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}\n")
  string(APPEND _str
    "  C FLAGS:              ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELWITHDEBINFO}\n")
elseif (CMAKE_BUILD_TYPE MATCHES Debug)
  string(APPEND _str
    "  CXX FLAGS:            ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}\n")
  string(APPEND _str
    "  C FLAGS:              ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}\n")
endif ()
string(APPEND _str
  "  DEPENDENCY_LIB:              ${DEPENDENCY_LIB}\n")
string(APPEND _str "\n")

string(APPEND _str
  "  HCL_PACKAGE_VERSION:        ${HCL_PACKAGE_VERSION}\n")
string(APPEND _str
  "  HCL_GIT_VERSION:            ${HCL_GIT_VERSION}\n")
string(APPEND _str
  "  HCL_COMMUNICATION:          ${HCL_COMMUNICATION}\n")
string(APPEND _str
  "  HCL_COMMUNICATION_PROTOCOL:                ${HCL_COMMUNICATION_PROTOCOL}\n")
  string(APPEND _str
    "  MPIEXEC_EXECUTABLE:                ${MPIEXEC_EXECUTABLE}\n")
string(APPEND _str
  "  HCL_PROFILER:               ${HCL_PROFILER}\n")
string(APPEND _str
  "  HCL_LOGGER:                 ${HCL_LOGGER}\n")
string(APPEND _str
  "  HCL_LOGGER_LEVEL:           ${HCL_LOGGER_LEVEL}\n")
string(APPEND _str
  "  GLIBC_VERSION:               ${GLIBC_VERSION}\n")
  string(APPEND _str
    "  TEST_LIBS:               ${TEST_LIBS}\n")

#Print the true/false variables
append_str_tf(_str
  HCL_GNU_LINUX
  HCL_COMMUNICATION_ENABLE_THALLIUM
  HCL_COMMUNICATION_PROTOCOL_ENABLE_OFI
  HCL_COMMUNICATION_PROTOCOL_ENABLE_UCX
  HCL_LIBDIR_AS_LIB
  HCL_USE_CLANG_LIBCXX
  HCL_WARNINGS_AS_ERRORS
  HCL_ENABLE_TESTING
  
)
string(APPEND _str
  "\n== End HCL Configuration Summary ==\n")

# Output to stdout
execute_process(COMMAND ${CMAKE_COMMAND} -E echo "${_str}")
set(_str)


###############################################################################
# Write a basic modulefile
###############################################################################

set(HCL_MODULEFILE_NAME "hcl-${HCL_PACKAGE_VERSION}.lua")
#  CACHE STRING
#  "The name of the HCL modulefile to install. Must end in .lua.")

if (NOT (HCL_MODULEFILE_NAME MATCHES ".+\.lua"))
  message(WARNING
    "HCL_MODULEFILE_NAME must have extension \".lua\". Appending.")
  set(HCL_MODULEFILE_NAME "${HCL_MODULEFILE_NAME}.lua"
    CACHE STRING "" FORCE)
endif ()

configure_file(
  "${CMAKE_SOURCE_DIR}/cmake/configure_files/hcl_module.lua.in"
  "${CMAKE_BINARY_DIR}/hcl_module.lua.install"
  @ONLY)

install(FILES "${CMAKE_BINARY_DIR}/hcl_module.lua.install"
  RENAME "${HCL_MODULEFILE_NAME}"
  DESTINATION
  "${HCL_INSTALL_SYSCONFDIR}")
endif()