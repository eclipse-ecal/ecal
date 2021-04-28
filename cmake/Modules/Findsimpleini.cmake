find_path(simpleini_INCLUDE_DIR
  NAMES SimpleIni.h
  HINTS
    "${CONAN_SIMPLEINI_ROOT}/include"
    "${ECAL_PROJECT_ROOT}/thirdparty/simpleini"
  NO_DEFAULT_PATH
  NO_CMAKE_FIND_ROOT_PATH
)

if(simpleini_INCLUDE_DIR-NOTFOUND)
  message(FATAL_ERROR "Could not find simpleini library")
  set(simpleini_FOUND FALSE)
else()
    set(simpleini_FOUND TRUE)
endif()

if(simpleini_FOUND)
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(simpleini
    REQUIRED_VARS simpleini_INCLUDE_DIR)

  if(NOT TARGET simpleini::simpleini)
    set(simpleini_INCLUDE_DIRS ${simpleini_INCLUDE_DIR})
    add_library(simpleini::simpleini INTERFACE IMPORTED)
    set_target_properties(simpleini::simpleini PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${simpleini_INCLUDE_DIR})
    mark_as_advanced(simpleini_INCLUDE_DIR)
  endif()
endif()
