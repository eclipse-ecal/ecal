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
  modules/module_client.h
  modules/module_core.cpp
  modules/module_core.h
  modules/module_datatypeinfo.cpp
  modules/module_datatypeinfo.h
  modules/module_publisher.cpp
  modules/module_publisher.h
  modules/module_monitoring_config.cpp
  modules/module_monitoring_config.h
  modules/module_publisher_config.cpp
  modules/module_publisher_config.h
  modules/module_application_config.cpp
  modules/module_application_config.h
  modules/module_logging_config.cpp
  modules/module_logging_config.h
  modules/module_server.cpp
  modules/module_server.h
  modules/module_subscriber.cpp
  modules/module_subscriber.h
  modules/module_service_config.cpp
  modules/module_service_config.h
  modules/module_subscriber_config.cpp
  modules/module_subscriber_config.h
  modules/module_time_config.cpp
  modules/module_time_config.h
  modules/module_user_arguments_config.cpp
  modules/module_user_arguments_config.h
  modules/module_util.cpp
  modules/module_util.h

  wrappers/wrapper_client.cpp
  wrappers/wrapper_client.h
  wrappers/wrapper_datatypeinfo.cpp
  wrappers/wrapper_datatypeinfo.h
  wrappers/wrapper_publisher.cpp
  wrappers/wrapper_publisher.h
  wrappers/wrapper_publisher_config.cpp
  wrappers/wrapper_publisher_config.h
  wrappers/wrapper_server.cpp
  wrappers/wrapper_server.h
  wrappers/wrapper_subscriber.cpp
  wrappers/wrapper_subscriber.h
  wrappers/wrapper_application_config.cpp
  wrappers/wrapper_application_config.h
  wrappers/wrapper_service_config.cpp
  wrappers/wrapper_service_config.h
  wrappers/wrapper_subscriber_config.cpp
  wrappers/wrapper_subscriber_config.h
  wrappers/wrapper_time_config.cpp
  wrappers/wrapper_time_config.h
  wrappers/wrapper_user_arguments_config.cpp
  wrappers/wrapper_user_arguments_config.h
  wrappers/wrapper_logging_config.cpp
  wrappers/wrapper_logging_config.h

  nanobind_core.cpp
)

target_include_directories(nanobind_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(nanobind_core PRIVATE eCAL::core)

# Install directive for scikit-build-core
install(TARGETS nanobind_core LIBRARY DESTINATION nanobind_core)

if(WIN32)
  set_property(TARGET nanobind_core PROPERTY FOLDER lang/python/core)
endif()