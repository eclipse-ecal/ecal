# Try to import all Python components potentially needed by nanobind
find_package(Python
  REQUIRED COMPONENTS Interpreter Development.Module
  OPTIONAL_COMPONENTS Development.SABIModule)

# Import nanobind through CMake's find_package mechanism
find_package(nanobind CONFIG REQUIRED)

set(nanobind_sources
  core/py_config.cpp
  core/py_config.h
  core/py_core.cpp
  core/py_core.h
  core/py_init.cpp
  core/py_init.h
  core/py_log.cpp
  core/py_log.h
  core/py_log_level.cpp
  core/py_log_level.h
  core/py_monitoring.cpp
  core/py_monitoring.h
  core/py_process.cpp
  core/py_process.h
  core/py_process_severity.cpp
  core/py_process_severity.h
  core/py_registration.cpp
  core/py_registration.h
  core/py_time.cpp
  core/py_time.h
  core/py_timer.cpp
  core/py_timer.h
  core/py_types.cpp
  core/py_types.h
  core/py_util.cpp
  core/py_util.h
  core/config/py_application.cpp
  core/config/py_application.h
  core/config/py_configuration.cpp
  core/config/py_configuration.h
  core/config/py_logging.cpp
  core/config/py_logging.h
  core/config/py_publisher.cpp
  core/config/py_publisher.h
  core/config/py_registration.cpp
  core/config/py_registration.h
  core/config/py_subscriber.cpp
  core/config/py_subscriber.h
  core/config/py_time.cpp
  core/config/py_time.h
  core/config/py_transport_layer.cpp
  core/config/py_transport_layer.h
  core/pubsub/py_publisher.cpp
  core/pubsub/py_publisher.h
  core/pubsub/py_subscriber.cpp
  core/pubsub/py_subscriber.h
  core/pubsub/py_types.cpp
  core/pubsub/py_types.h
  core/service/py_client.cpp
  core/service/py_client.h
  core/service/py_client_instance.cpp
  core/service/py_client_instance.h
  core/service/py_server.cpp
  core/service/py_server.h
  core/service/py_types.cpp
  core/service/py_types.h
  core/types/py_custom_data_types.cpp
  core/types/py_custom_data_types.h
  core/types/py_logging.cpp
  core/types/py_logging.h
  core/types/py_monitoring.cpp
  core/types/py_monitoring.h
  nanobind_core.cpp
)

# We are now ready to compile the actual extension module
nanobind_add_module(
  # Name of the extension
  nanobind_core

  # Target the stable ABI for Python 3.12+, which reduces
  # the number of binary wheels that must be built. This
  # does nothing on older Python versions
  STABLE_ABI

  # Source code goes here
  ${nanobind_sources}
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES 
  ${nanobind_sources}
)

target_include_directories(nanobind_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(nanobind_core PRIVATE eCAL::core)

# Install directive for scikit-build-core
install(TARGETS nanobind_core LIBRARY DESTINATION nanobind_core)

if(WIN32)
  set_property(TARGET nanobind_core PROPERTY FOLDER lang/python/core)
endif()

ecal_python_set_output_directory(nanobind_core)