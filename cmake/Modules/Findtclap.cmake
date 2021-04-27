find_path(tclap_INCLUDE_DIR
  NAMES tclap/Arg.h
  HINTS    
    "${CONAN_TCLAP_ROOT}/include"
    "${ECAL_PROJECT_ROOT}/thirdparty/tclap/include"
  NO_DEFAULT_PATH
  NO_CMAKE_FIND_ROOT_PATH
)

if(tclap_INCLUDE_DIR-NOTFOUND)
  message(FATAL_ERROR "Could not find tclap library")
  set(tclap_FOUND FALSE)
else()
  set(tclap_FOUND TRUE)
endif()

if(tclap_FOUND)
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(tclap
    REQUIRED_VARS tclap_INCLUDE_DIR)

  if(NOT TARGET tclap::tclap)
    set(tclap_INCLUDE_DIRS ${tclap_INCLUDE_DIR})
    add_library(tclap::tclap INTERFACE IMPORTED)
    set_target_properties(tclap::tclap PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${tclap_INCLUDE_DIR})
    mark_as_advanced(tclap_INCLUDE_DIRS)
  endif()
endif()
