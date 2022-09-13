find_path(YAML_CPP_INCLUDE_DIRS
  NAMES yaml-cpp/yaml.h
  HINTS "${ECAL_PROJECT_ROOT}/thirdparty/yaml-cpp/include"
  NO_DEFAULT_PATH
  NO_CMAKE_FIND_ROOT_PATH
)

if(YAML_CPP_INCLUDE_DIRS-NOTFOUND)
  message(FATAL_ERROR "Could not find yaml-cpp library")
  set(yaml-cpp_FOUND FALSE)
else()
  set(yaml-cpp_FOUND TRUE)
endif()

find_library(YAML_CPP_LIBRARIES
  NAMES yaml-cpp
)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args(yaml-cpp REQUIRED_VARS YAML_CPP_INCLUDE_DIRS YAML_CPP_LIBRARIES)

if (yaml-cpp_FOUND AND NOT TARGET yaml-cpp)
  add_library(yaml-cpp UNKNOWN IMPORTED)
  set_target_properties(yaml-cpp PROPERTIES
                        INTERFACE_INCLUDE_DIRECTORIES "${YAML_CPP_INCLUDE_DIRS}"
                        IMPORTED_LOCATION "${YAML_CPP_LIBRARIES}")
endif ()

mark_as_advanced (
  YAML_CPP_INCLUDE_DIRS
  YAML_CPP_LIBRARIES
)
