#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "hcl" for configuration "Debug"
set_property(TARGET hcl APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(hcl PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib64/libhcl.so.0.1.0"
  IMPORTED_SONAME_DEBUG "libhcl.so.1"
  )

list(APPEND _IMPORT_CHECK_TARGETS hcl )
list(APPEND _IMPORT_CHECK_FILES_FOR_hcl "${_IMPORT_PREFIX}/lib64/libhcl.so.0.1.0" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
