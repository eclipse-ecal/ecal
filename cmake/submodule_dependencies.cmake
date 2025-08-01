# Always ensure we have the policy settings this provider expects
cmake_minimum_required(VERSION 3.24)

set(ecal_submodule_dependency_provider_root_dir ${CMAKE_CURRENT_LIST_DIR})

set(ecal_submodule_dependencies 
  absl
  asio
  CMakeFunctions
  CURL
  ecaludp
  fineftp
  ftxui
  GTest
  HDF5
  #libssh2
  nanobind
  Protobuf
  qwt
  recycle
  spdlog
  tclap
  tcp_pubsub
  termcolor
  tsl-robin-map
  tinyxml2
  udpcap
  yaml-cpp
)

set(ecal_inproject_dependencies
  eCAL
)

macro(ecal_handle_submodule_dependency package_name)
  string(TOUPPER ${package_name} package_name_upper)
  string(TOLOWER ${package_name} package_name_lower)
  
  option(ECAL_THIRDPARTY_BUILD_${package_name_upper} "Build ${package_name} with eCAL" ON)
  
  if (ECAL_THIRDPARTY_BUILD_${package_name_upper})
    include(${ecal_submodule_dependency_provider_root_dir}/../thirdparty/${package_name_lower}/build-${package_name_lower}.cmake)
    set(${package_name}_FOUND TRUE)
  else ()  
    find_package(${package_name} BYPASS_PROVIDER ${ARGN})    
  endif ()
endmacro()

macro(ecal_handle_inproject_dependency package_name)
  set(${package_name}_FOUND TRUE)
endmacro()

macro(ecal_handle_other_dependency package_name)
  find_package(${package_name} BYPASS_PROVIDER ${ARGN})
endmacro()

macro(ecal_dependencies_provider method package_name)
  list(FIND ecal_submodule_dependencies ${package_name} is_submodule_dependency)
  list(FIND ecal_inproject_dependencies ${package_name} is_inproject_dependency)
  
  #inproject dependency
  if (NOT ${is_submodule_dependency} EQUAL -1)
    ecal_handle_submodule_dependency(${package_name} ${ARGN}) 
  #submodule dependency  
  elseif (NOT ${is_inproject_dependency} EQUAL -1)
    ecal_handle_inproject_dependency(${package_name} ${ARGN})
  else ()
  #other dependency 
    ecal_handle_other_dependency(${package_name} ${ARGN})
  endif ()
endmacro()

cmake_language(
  SET_DEPENDENCY_PROVIDER ecal_dependencies_provider
  SUPPORTED_METHODS FIND_PACKAGE
)
