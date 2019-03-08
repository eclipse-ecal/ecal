find_path(tclap_INCLUDE_DIR
  NAMES tclap/Arg.h
  PATHS 
  include
  ${CMAKE_SOURCE_DIR}/thirdparty/tclap/include
)

if(tclap_INCLUDE_DIR-NOTFOUND)
  message(FATAL_ERROR "Could not find tclap library")
  set(tclap_FOUND False)
else()
  set(tclap_FOUND True)
endif()

if(tclap_FOUND)
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(tclap
    REQUIRED_VARS tclap_INCLUDE_DIR
  )

  if(NOT TARGET tclap::tclap)
    set(tclap_INCLUDE_DIRS ${tclap_INCLUDE_DIR})

    add_library(tclap::tclap INTERFACE IMPORTED)
    set_target_properties(tclap::tclap PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${tclap_INCLUDE_DIR})
    mark_as_advanced(tclap_INCLUDE_DIRS)
  endif()
endif()