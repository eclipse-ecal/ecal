# Try to import all Python components potentially needed by nanobind
find_package(Python
  REQUIRED COMPONENTS Interpreter Development.Module
  OPTIONAL_COMPONENTS Development.SABIModule)

# Import nanobind through CMake's find_package mechanism
find_package(nanobind CONFIG REQUIRED)

# We are now ready to compile the actual extension module
nanobind_add_module(
  # Name of the extension
  nanobind_core

  # Target the stable ABI for Python 3.12+, which reduces
  # the number of binary wheels that must be built. This
  # does nothing on older Python versions
  STABLE_ABI

  # Source code goes here
  modules/module_client.cpp
  modules/module_core.cpp
  modules/module_publisher.cpp
  modules/module_server.cpp
  modules/module_subscriber.cpp
  modules/module_util.cpp

  wrappers/client.cpp
  wrappers/datatypeinfo.cpp
  wrappers/publisher.cpp
  wrappers/server.cpp
  wrappers/subscriber.cpp
  nanobind_core.cpp
)

target_include_directories(nanobind_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(nanobind_core PRIVATE eCAL::core)

# Install directive for scikit-build-core
install(TARGETS nanobind_core LIBRARY DESTINATION nanobind_core)

if(WIN32)
  set_property(TARGET nanobind_core PROPERTY FOLDER lang/python/core)
endif()