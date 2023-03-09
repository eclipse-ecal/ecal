find_path(xxhash_INCLUDE_DIR
  NAMES xxhash64.h
  HINTS
    "${CONAN_XXHASH_ROOT}/include"
    "${ECAL_PROJECT_ROOT}/thirdparty/xxhash"
  NO_DEFAULT_PATH
  NO_CMAKE_FIND_ROOT_PATH
)

if(xxhash_INCLUDE_DIR-NOTFOUND)
  message(FATAL_ERROR "Could not find xxhash library")
  set(xxhash_FOUND FALSE)
else()
    set(xxhash_FOUND TRUE)
endif()

if(xxhash_FOUND)
  include(FindPackageHandleStandardArgs)
  
  find_package_handle_standard_args(xxhash
    REQUIRED_VARS xxhash_INCLUDE_DIR)

  if(NOT TARGET xxhash::xxhash)
    set(xxhash_INCLUDE_DIRS ${xxhash_INCLUDE_DIR})
    add_library(xxhash::xxhash INTERFACE IMPORTED)
    set_target_properties(xxhash::xxhash PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${xxhash_INCLUDE_DIR})
    mark_as_advanced(xxhash_INCLUDE_DIR)
  endif()
endif()
