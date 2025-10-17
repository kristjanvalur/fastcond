#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "fastcond::fastcond" for configuration "Release"
set_property(TARGET fastcond::fastcond APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(fastcond::fastcond PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libfastcond.a"
  )

list(APPEND _cmake_import_check_targets fastcond::fastcond )
list(APPEND _cmake_import_check_files_for_fastcond::fastcond "${_IMPORT_PREFIX}/lib/libfastcond.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
