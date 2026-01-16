# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2025 Continental Corporation
# Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#      http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# ========================= eCAL LICENSE =================================

# Use smart configure to avoid rebuild / updated timestamps on generated files
# if the content hasn't changed
macro(smart_configure INPUT_FILE OUTPUT_FILE)
  # Define the temporary file name as OUTPUT_FILE.tmp
  set(TEMP_FILE "${OUTPUT_FILE}.tmp")
  
  # Generate the file into the temporary location using the @ONLY mode.
  configure_file("${INPUT_FILE}" "${TEMP_FILE}" @ONLY)
  
  # Copy the temporary file to the final destination if its content is different.
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${TEMP_FILE}" "${OUTPUT_FILE}"
  )
  
  # Remove the temporary file.
  file(REMOVE "${TEMP_FILE}")
endmacro()


project(core VERSION ${eCAL_VERSION_STRING})

find_package(asio             REQUIRED)
find_package(Threads          REQUIRED)
find_package(ecaludp          REQUIRED)
find_package(protozero        REQUIRED)

if (ECAL_CORE_CONFIGURATION)
  find_package(yaml-cpp       REQUIRED)
  include(${ECAL_PROJECT_ROOT}/thirdparty/yaml-cpp/compatibility-yaml-cpp.cmake)
  yaml_cpp_create_compatibility_targets()
endif()

if (ECAL_CORE_COMMAND_LINE)
  find_package(tclap          REQUIRED)
endif()

if (ECAL_CORE_TRANSPORT_TCP)
  find_package(tcp_pubsub 2   REQUIRED)
endif()

# If we're currently doing a build within a git repository, we will configure the header files.
# Else, (e.g. for source packages such as debian source packages) we will use a preconfigured file.
# If there is really no information available, it will generate a dummy version file 0.0.0
if     (IS_GIT_TREE OR NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/ecal/defs.h")
  smart_configure(src/defs.h.in "${CMAKE_CURRENT_BINARY_DIR}/include/ecal/defs.h")
endif  (IS_GIT_TREE OR NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/ecal/defs.h")

if     (IS_GIT_TREE OR NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/git_version.h")
  smart_configure(src/git_version.h.in "${CMAKE_CURRENT_BINARY_DIR}/src/git_version.h")
endif  (IS_GIT_TREE OR NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/git_version.h")


if(UNIX)
 include (CheckSymbolExists)
 set(CMAKE_REQUIRED_DEFINITIONS "-D__USE_GNU"
                                "-D_GNU_SOURCE")
  set(CMAKE_REQUIRED_LIBRARIES "pthread")
  check_symbol_exists(pthread_mutex_clocklock "pthread.h" ECAL_HAS_CLOCKLOCK_MUTEX)
  check_symbol_exists(pthread_mutexattr_setrobust "pthread.h" ECAL_HAS_ROBUST_MUTEX)
  unset(CMAKE_REQUIRED_DEFINITIONS)
  unset(CMAKE_REQUIRED_LIBRARIES)
  if(NOT ECAL_HAS_ROBUST_MUTEX)
    if(NOT ECAL_HAS_CLOCKLOCK_MUTEX)
      message(WARNING "This OS does not support robust mutexes with monotonic clock which can cause dead locks under certain circumstances. (e.g. memfile monitoring).")
    else()
      message(WARNING "This OS does not support robust mutexes which can cause dead locks under certain circumstances (e.g. memfile monitoring).")
    endif()
  endif()
endif()

macro(add_ecal_serialization_object_library)
add_library(ecal_core_serialization OBJECT)
target_sources(ecal_core_serialization 
  PUBLIC
    src/serialization/ecal_serialize_logging.h
    src/serialization/ecal_serialize_monitoring.h
    src/serialization/ecal_serialize_sample_payload.h
    src/serialization/ecal_serialize_sample_registration.h
    src/serialization/ecal_serialize_service.h
    src/serialization/ecal_struct_sample_common.h
    src/serialization/ecal_struct_sample_payload.h
    src/serialization/ecal_struct_sample_registration.h
    src/serialization/ecal_struct_service.h
  PRIVATE
    src/serialization/protozero/ecal_helper.h
    src/serialization/protozero/ecal/core/pb/datatype.pbftags.h
    src/serialization/protozero/ecal/core/pb/ecal.pbftags.h
    src/serialization/protozero/ecal/core/pb/host.pbftags.h
    src/serialization/protozero/ecal/core/pb/layer.pbftags.h
    src/serialization/protozero/ecal/core/pb/logging.pbftags.h
    src/serialization/protozero/ecal/core/pb/monitoring.pbftags.h
    src/serialization/protozero/ecal/core/pb/process.pbftags.h
    src/serialization/protozero/ecal/core/pb/service.pbftags.h
    src/serialization/protozero/ecal/core/pb/topic.pbftags.h
    src/serialization/ecal_serialize_common.cpp
    src/serialization/ecal_serialize_common.h
    src/serialization/ecal_serialize_logging.cpp
    src/serialization/ecal_serialize_monitoring.cpp
    src/serialization/ecal_serialize_sample_payload.cpp
    src/serialization/ecal_serialize_sample_registration.cpp
    src/serialization/ecal_serialize_service.cpp
)

target_include_directories(ecal_core_serialization
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/serialization>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/serialization/protozero>
)

target_link_libraries(ecal_core_serialization 
  PRIVATE
    protozero::protozero
)

set_target_properties(ecal_core_serialization PROPERTIES 
  POSITION_INDEPENDENT_CODE ON  
  FOLDER core
)
endmacro()

macro(add_ecal_core_object_library)

######################################
# config
######################################
set(ecal_config_src
    src/config/default_configuration.cpp
    src/config/default_configuration.h
    src/config/ecal_config.cpp
    src/config/configuration.cpp
    src/config/ecal_path_processing.cpp
    src/config/ecal_path_processing.h
    src/types/ecal_custom_data_types.cpp
)
if (ECAL_CORE_CONFIGURATION)
  list(APPEND ecal_config_src
      src/config/configuration_to_yaml.cpp
      src/config/configuration_to_yaml.h
      src/config/configuration_reader.cpp
      src/config/configuration_reader.h
      src/config/configuration_writer.cpp
      src/config/configuration_writer.h
  )
endif()
######################################
# io/mtx
######################################
if(ECAL_CORE_REGISTRATION_SHM OR ECAL_CORE_TRANSPORT_SHM)
  set(ecal_io_mtx_src
      src/io/mtx/ecal_named_mutex.cpp
      src/io/mtx/ecal_named_mutex.h
      src/io/mtx/ecal_named_mutex_base.h
  )

  # io/mtx/linux
  if(UNIX)
    set(ecal_io_mtx_linux_src
        src/io/mtx/linux/ecal_named_mutex_impl.cpp
        $<$<OR:$<BOOL:${ECAL_HAS_CLOCKLOCK_MUTEX}>,$<BOOL:${ECAL_HAS_ROBUST_MUTEX}>>:src/io/mtx/linux/ecal_named_mutex_robust_clocklock_impl.cpp>
        src/io/mtx/linux/ecal_named_mutex_impl.h
        $<$<OR:$<BOOL:${ECAL_HAS_CLOCKLOCK_MUTEX}>,$<BOOL:${ECAL_HAS_ROBUST_MUTEX}>>:src/io/mtx/linux/ecal_named_mutex_robust_clocklock_impl.h>
    )
  endif()

  # io/mtx/win32
  if(WIN32)
    set(ecal_io_mtx_win_src
        src/io/mtx/win32/ecal_named_mutex_impl.cpp
        src/io/mtx/win32/ecal_named_mutex_impl.h
    )
  endif()
endif()

######################################
# io/shm
######################################
if(ECAL_CORE_REGISTRATION_SHM OR ECAL_CORE_TRANSPORT_SHM)
  set(ecal_io_shm_src
      src/io/shm/ecal_memfile.cpp
      src/io/shm/ecal_memfile_db.cpp
      src/io/shm/ecal_memfile_naming.cpp      
      src/io/shm/ecal_memfile_pool.cpp
      src/io/shm/ecal_memfile_sync.cpp
      src/io/shm/ecal_memfile.h
      src/io/shm/ecal_memfile_db.h
      src/io/shm/ecal_memfile_header.h
      src/io/shm/ecal_memfile_info.h
      src/io/shm/ecal_memfile_naming.h
      src/io/shm/ecal_memfile_os.h
      src/io/shm/ecal_memfile_pool.h
      src/io/shm/ecal_memfile_sync.h
  )

  # io/shm/linux
  if(UNIX)
    set(ecal_io_shm_linux_src
        src/io/shm/linux/ecal_memfile_os.cpp
    )
  endif()

  # io/shm/win32
  if(WIN32)
    set(ecal_io_shm_win_src
       src/io/shm/win32/ecal_memfile_os.cpp
    )
  endif()
endif()

######################################
# io/udp
######################################
# io/udp (npcap)
if(ECAL_CORE_NPCAP_SUPPORT)
set(ecal_io_udp_src_npcap 
    src/io/udp/ecal_udp_sample_receiver_npcap.cpp
    src/io/udp/ecal_udp_sample_receiver_npcap.h
)
endif()

# io/udp
set(ecal_io_udp_src
    src/io/udp/ecal_udp_configurations.cpp
    src/io/udp/ecal_udp_configurations.h
    src/io/udp/ecal_udp_receiver_attr.h
    src/io/udp/ecal_udp_sample_receiver.cpp
    src/io/udp/ecal_udp_sample_receiver.h
    src/io/udp/ecal_udp_sample_receiver_asio.cpp
    src/io/udp/ecal_udp_sample_receiver_asio.h
    src/io/udp/ecal_udp_sample_receiver_base.h
    src/io/udp/ecal_udp_sample_sender.cpp
    src/io/udp/ecal_udp_sample_sender.h
    src/io/udp/ecal_udp_sender_attr.h
    src/io/udp/ecal_udp_topic2mcast.h
    ${ecal_io_udp_src_npcap}
)

# io/udp/sendreceive/linux
if(UNIX)
  set(ecal_io_udp_linux_src
      src/io/udp/linux/socket_os.h
)
endif()

######################################
# logging
######################################
set(ecal_logging_src
    src/logging/ecal_log.cpp
    src/logging/ecal_log_provider.cpp
    src/logging/ecal_log_provider.h
    src/logging/ecal_log_receiver.cpp
    src/logging/ecal_log_receiver.h
)

######################################
# monitoring
######################################
if(ECAL_CORE_MONITORING)
set(ecal_monitoring_src
    src/monitoring/ecal_monitoring_def.cpp
    src/monitoring/ecal_monitoring_def.h
    src/monitoring/ecal_monitoring_impl.cpp
    src/monitoring/ecal_monitoring_impl.h
)
endif()

######################################
# pubsub
######################################
if(ECAL_CORE_PUBLISHER)
  set(ecal_pub_src
      src/pubsub/ecal_publisher.cpp
      src/pubsub/ecal_publisher_impl.cpp
      src/pubsub/ecal_publisher_impl.h
      src/pubsub/ecal_pubgate.cpp
      src/pubsub/ecal_pubgate.h
      src/v5/pubsub/ecal_publisher.cpp
  )
endif()

if(ECAL_CORE_SUBSCRIBER)
  set(ecal_sub_src
      src/pubsub/ecal_subscriber.cpp
      src/pubsub/ecal_subscriber_impl.cpp
      src/pubsub/ecal_subscriber_impl.h
      src/pubsub/ecal_subgate.cpp
      src/pubsub/ecal_subgate.h
      src/v5/pubsub/ecal_subscriber.cpp
  )
endif()

######################################
# readwrite
######################################
set(ecal_readwrite_src
    src/readwrite/ecal_transport_layer.h
)

if(ECAL_CORE_PUBLISHER)
  set(ecal_writer_src
      src/readwrite/ecal_writer_base.h
      src/readwrite/ecal_writer_buffer_payload.h
      src/readwrite/ecal_writer_data.h
      src/readwrite/ecal_writer_info.h
  )
  if(ECAL_CORE_TRANSPORT_UDP)
    list(APPEND ecal_writer_src
        src/readwrite/udp/ecal_writer_udp.cpp
        src/readwrite/udp/ecal_writer_udp.h
    )
  endif()
  if(ECAL_CORE_TRANSPORT_TCP)
    list(APPEND ecal_writer_src
        src/readwrite/tcp/ecal_tcp_pubsub_logger.h
        src/readwrite/tcp/ecal_writer_tcp.cpp
        src/readwrite/tcp/ecal_writer_tcp.h
    )
  endif()
  if(ECAL_CORE_TRANSPORT_SHM)
    list(APPEND ecal_writer_src
        src/readwrite/shm/ecal_writer_shm.cpp
        src/readwrite/shm/ecal_writer_shm.h
    )
  endif()
endif()

if(ECAL_CORE_SUBSCRIBER)
  set(ecal_reader_src
      src/readwrite/ecal_reader_layer.h
      src/readwrite/ecal_reader_manager.cpp
      src/readwrite/ecal_reader_manager.h
  )
  if(ECAL_CORE_TRANSPORT_UDP)
    list(APPEND ecal_reader_src
        src/readwrite/udp/ecal_reader_udp.cpp
        src/readwrite/udp/ecal_reader_udp.h
    )
  endif()
  if(ECAL_CORE_TRANSPORT_TCP)
    list(APPEND ecal_reader_src
        src/readwrite/tcp/ecal_reader_tcp.cpp
        src/readwrite/tcp/ecal_reader_tcp.h
    )
  endif()
  if(ECAL_CORE_TRANSPORT_SHM)
    list(APPEND ecal_reader_src
        src/readwrite/shm/ecal_reader_shm.cpp
        src/readwrite/shm/ecal_reader_shm.h
    )
  endif()
endif()

######################################
# registration
######################################
if (ECAL_CORE_REGISTRATION)
  set(ecal_registration_src
      src/registration/ecal_process_registration.cpp
      src/registration/ecal_process_registration.h
      src/registration/ecal_registration.cpp
      src/registration/ecal_registration_provider.cpp
      src/registration/ecal_registration_provider.h
      src/registration/ecal_registration_receiver.cpp
      src/registration/ecal_registration_receiver.h
      src/registration/ecal_registration_sample_applier.cpp
      src/registration/ecal_registration_sample_applier.h
      src/registration/ecal_registration_sample_applier_gates.cpp
      src/registration/ecal_registration_sample_applier_gates.h
      src/registration/ecal_registration_timeout_provider.cpp
      src/registration/ecal_registration_timeout_provider.h
      src/registration/ecal_registration_sender.h
      src/registration/udp/ecal_registration_receiver_udp.cpp
      src/registration/udp/ecal_registration_receiver_udp.h
      src/registration/udp/ecal_registration_sender_udp.cpp
      src/registration/udp/ecal_registration_sender_udp.h
  )
  if(ECAL_CORE_REGISTRATION_SHM)
    list(APPEND ecal_registration_src
        src/registration/shm/ecal_registration_receiver_shm.cpp
        src/registration/shm/ecal_registration_receiver_shm.h
        src/registration/shm/ecal_registration_sender_shm.cpp
        src/registration/shm/ecal_registration_sender_shm.h
        src/registration/shm/ecal_memfile_broadcast.cpp
        src/registration/shm/ecal_memfile_broadcast.h
        src/registration/shm/ecal_memfile_broadcast_reader.cpp
        src/registration/shm/ecal_memfile_broadcast_reader.h
        src/registration/shm/ecal_memfile_broadcast_writer.cpp
        src/registration/shm/ecal_memfile_broadcast_writer.h
        src/registration/shm/relocatable_circular_queue.h
    )
  endif()
endif()

######################################
# service
######################################
if(ECAL_CORE_SERVICE)
  set(ecal_service_src
      src/service/ecal_clientgate.cpp
      src/service/ecal_clientgate.h
      src/service/ecal_servicegate.cpp
      src/service/ecal_servicegate.h
      src/service/ecal_service_client.cpp
      src/service/ecal_service_client_impl.cpp
      src/service/ecal_service_client_impl.h
      src/service/ecal_service_client_instance.cpp
      src/service/ecal_service_server.cpp
      src/service/ecal_service_server_impl.cpp
      src/service/ecal_service_server_impl.h
      src/service/ecal_service_singleton_manager.cpp
      src/service/ecal_service_singleton_manager.h
      src/v5/service/ecal_service_client.cpp
      src/v5/service/ecal_service_client_impl.cpp
      src/v5/service/ecal_service_client_impl.h
      src/v5/service/ecal_service_server.cpp
      src/v5/service/ecal_service_server_impl.cpp
      src/v5/service/ecal_service_server_impl.h
  )
endif()

######################################
# time
######################################
set(ecal_time_src
    src/time/ecal_time.cpp
    src/time/ecal_timer.cpp
)
if(ECAL_CORE_TIMEPLUGIN)
  list(APPEND ecal_time_src
      src/time/ecal_timegate.cpp
      src/time/ecal_timegate.h
  )
endif()

######################################
# util
######################################
set(ecal_util_src
    src/util/ecal_expmap.h
    src/util/ecal_thread.h
    src/util/expanding_vector.h
    src/util/frequency_calculator.h
    src/util/statistics_calculator.h
    src/util/message_drop_calculator.cpp
    src/util/message_drop_calculator.h
    src/util/getenvvar.h
    src/util/counter_cache.h
)
if (ECAL_CORE_COMMAND_LINE)
  list(APPEND ecal_util_src
      src/util/advanced_tclap_output.cpp
      src/util/advanced_tclap_output.h
  )
endif()

######################################
# common
######################################
set(ecal_cmn_src
    src/ecal.cpp
    src/ecal_config_internal.h
    src/ecal_def.h
    src/ecal_descgate.cpp
    src/ecal_descgate.h
    src/ecal_event.cpp 
    src/ecal_event.h
    src/ecal_eventhandle.h
    src/ecal_global_accessors.cpp
    src/ecal_global_accessors.h
    src/ecal_globals.cpp
    src/ecal_globals.h
    src/ecal_process.cpp
    src/ecal_util.cpp
    src/ecal_win_main.h
)
if (WIN32)
  list(APPEND ecal_cmn_src
      src/ecal_win_main.h
  )
endif()

######################################
# builder
######################################
set (ecal_builder_src
    src/config/builder/logging_attribute_builder.cpp
    src/config/builder/logging_attribute_builder.h
    src/config/builder/registration_attribute_builder.cpp
    src/config/builder/registration_attribute_builder.h

    src/logging/config/attributes/ecal_log_provider_attributes.h
    src/logging/config/attributes/ecal_log_receiver_attributes.h
    src/logging/config/builder/udp_attribute_builder.cpp
    src/logging/config/builder/udp_attribute_builder.h

    src/pubsub/config/builder/reader_attribute_builder.cpp
    src/pubsub/config/builder/reader_attribute_builder.h
    src/pubsub/config/builder/writer_attribute_builder.cpp
    src/pubsub/config/builder/writer_attribute_builder.h

    src/readwrite/config/attributes/reader_attributes.h
    src/readwrite/config/attributes/writer_attributes.h
    src/readwrite/config/builder/shm_attribute_builder.cpp
    src/readwrite/config/builder/shm_attribute_builder.h
    src/readwrite/config/builder/tcp_attribute_builder.cpp
    src/readwrite/config/builder/tcp_attribute_builder.h
    src/readwrite/config/builder/udp_attribute_builder.cpp
    src/readwrite/config/builder/udp_attribute_builder.h

    src/readwrite/shm/config/attributes/reader_shm_attributes.h
    src/readwrite/shm/config/attributes/writer_shm_attributes.h

    src/readwrite/tcp/config/attributes/data_reader_tcp_attributes.h
    src/readwrite/tcp/config/attributes/data_writer_tcp_attributes.h
    src/readwrite/tcp/config/attributes/tcp_reader_layer_attributes.h
    src/readwrite/tcp/config/builder/data_reader_tcp_attribute_builder.cpp
    src/readwrite/tcp/config/builder/data_reader_tcp_attribute_builder.h

    src/readwrite/udp/config/attributes/reader_udp_attributes.h
    src/readwrite/udp/config/attributes/writer_udp_attributes.h
    src/readwrite/udp/config/builder/udp_attribute_builder.cpp
    src/readwrite/udp/config/builder/udp_attribute_builder.h

    src/registration/config/attributes/registration_attributes.h
    src/registration/config/attributes/sample_applier_attributes.h
    src/registration/config/builder/sample_applier_attribute_builder.cpp
    src/registration/config/builder/sample_applier_attribute_builder.h
    src/registration/config/builder/udp_shm_attribute_builder.cpp
    src/registration/config/builder/udp_shm_attribute_builder.h

    src/registration/shm/config/attributes/registration_shm_attributes.h
    src/registration/udp/config/attributes/registration_receiver_udp_attributes.h
    src/registration/udp/config/attributes/registration_sender_udp_attributes.h
    src/registration/udp/config/builder/udp_attribute_builder.cpp
    src/registration/udp/config/builder/udp_attribute_builder.h
)

######################################
# public header
######################################
set(ecal_public_headers
    include/ecal/config/application.h
    include/ecal/config/configuration.h
    include/ecal/config/logging.h
    include/ecal/config/publisher.h
    include/ecal/config/registration.h
    include/ecal/config/subscriber.h
    include/ecal/config/time.h
    include/ecal/config/transport_layer.h
    include/ecal/pubsub/subscriber.h
    include/ecal/pubsub/types.h
    include/ecal/pubsub/payload_writer.h
    include/ecal/pubsub/publisher.h
    include/ecal/service/client.h
    include/ecal/service/client_instance.h
    include/ecal/service/server.h
    include/ecal/service/types.h
    include/ecal/types/custom_data_types.h
    include/ecal/types/logging.h
    include/ecal/types/monitoring.h
    include/ecal/ecal.h
    include/ecal/config.h
    include/ecal/core.h
    include/ecal/deprecate.h
    include/ecal/init.h
    include/ecal/log.h
    include/ecal/log_level.h
    include/ecal/os.h
    include/ecal/monitoring.h
    include/ecal/namespace.h
    include/ecal/process.h
    include/ecal/process_mode.h
    include/ecal/process_severity.h
    include/ecal/registration.h
    include/ecal/time.h
    include/ecal/timer.h
    include/ecal/types.h
    include/ecal/util.h
)

set(ecal_public_v5_headers
    include/ecal/v5/ecal_callback.h
    include/ecal/v5/ecal_client.h
    include/ecal/v5/ecal_publisher.h
    include/ecal/v5/ecal_server.h
    include/ecal/v5/ecal_subscriber.h
)


set(ecal_sources
    ${ecal_config_src}
    ${ecal_io_mtx_src}
    ${ecal_io_mtx_linux_src}
    ${ecal_io_mtx_win_src}
    ${ecal_io_shm_src}
    ${ecal_io_shm_linux_src}
    ${ecal_io_shm_win_src}
    ${ecal_io_udp_src}
    ${ecal_io_udp_linux_src}
    ${ecal_logging_src}
    ${ecal_monitoring_src}
    ${ecal_pub_src}
    ${ecal_sub_src}
    ${ecal_readwrite_src}
    ${ecal_writer_src}
    ${ecal_reader_src}
    ${ecal_registration_src}
    ${ecal_serialization_src}
    ${ecal_service_src}
    ${ecal_time_src}
    ${ecal_util_src}
    ${ecal_cmn_src}
    ${ecal_builder_src}
)

set(ecal_generated_public_headers
  ${CMAKE_CURRENT_BINARY_DIR}/include/ecal/defs.h
)

set(ecal_generated_sources
    ${CMAKE_CURRENT_BINARY_DIR}/src/git_version.h
)

add_library(ecal_core_private OBJECT
    ${ecal_public_headers}
    ${ecal_public_v5_headers}
    ${ecal_generated_public_headers}
    ${ecal_sources}
    ${ecal_generated_sources}
)

if (ECAL_CORE_CONFIGURATION)
  target_compile_definitions(ecal_core_private PRIVATE ECAL_CORE_CONFIGURATION)
endif()

if(UNIX)
  set_source_files_properties(src/util/convert_utf.cpp PROPERTIES COMPILE_FLAGS -Wno-implicit-fallthrough)
endif()

target_compile_definitions(ecal_core_private
  PUBLIC
    ASIO_STANDALONE
    ASIO_DISABLE_VISIBILITY
  PRIVATE
    ECAL_CORE_EXPORTS
    $<$<BOOL:${ECAL_HAS_CLOCKLOCK_MUTEX}>:ECAL_HAS_CLOCKLOCK_MUTEX>
    $<$<BOOL:${ECAL_HAS_ROBUST_MUTEX}>:ECAL_HAS_ROBUST_MUTEX>
    $<$<BOOL:${ECAL_USE_CLOCKLOCK_MUTEX}>:ECAL_USE_CLOCKLOCK_MUTEX>
    ECAL_NO_DEPRECATION_WARNINGS
    _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS #Asio issues hundreds of deprecation warnings with VS2017
)

set(ECAL_CORE_FEATURES
  ECAL_CORE_CONFIGURATION
  ECAL_CORE_COMMAND_LINE
  ECAL_CORE_REGISTRATION
  ECAL_CORE_MONITORING
  ECAL_CORE_PUBLISHER
  ECAL_CORE_SUBSCRIBER
  ECAL_CORE_SERVICE
  ECAL_CORE_TIMEGATE
  ECAL_CORE_TIMEPLUGIN
  ECAL_CORE_REGISTRATION_SHM
  ECAL_CORE_TRANSPORT_UDP
  ECAL_CORE_TRANSPORT_TCP
  ECAL_CORE_TRANSPORT_SHM
  ECAL_CORE_NPCAP_SUPPORT
)

foreach(CORE_FEATURE ${ECAL_CORE_FEATURES})
  if(${CORE_FEATURE})
    target_compile_definitions(ecal_core_private
      PUBLIC ${CORE_FEATURE})
  endif()
endforeach()

target_include_directories(ecal_core_private
  PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(ecal_core_private 
  PRIVATE
    $<$<BOOL:${ECAL_CORE_CONFIGURATION}>:yaml-cpp::yaml-cpp>
    $<$<BOOL:${ECAL_CORE_COMMAND_LINE}>:tclap::tclap>
    $<$<BOOL:${ECAL_CORE_SERVICE}>:ecal_service $<TARGET_OBJECTS:ecal_service>>
    $<$<BOOL:${ECAL_CORE_SERVICE}>:dynamic_threadpool $<TARGET_OBJECTS:dynamic_threadpool>>
    $<$<BOOL:${ECAL_CORE_TRANSPORT_TCP}>:tcp_pubsub::tcp_pubsub>
    ecaludp::ecaludp
    $<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${QNXNTO}>>>:dl>
    $<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<NOT:$<BOOL:${QNXNTO}>>>:rt>
    $<$<STREQUAL:${CMAKE_SYSTEM_NAME},FreeBSD>:util>
    $<$<BOOL:${WIN32}>:iphlpapi>
    $<$<BOOL:${WIN32}>:psapi> 
    $<$<BOOL:${WIN32}>:shlwapi.lib> 
    $<$<BOOL:${WIN32}>:winmm> 
    $<$<BOOL:${WIN32}>:ws2_32> 
    $<$<BOOL:${WIN32}>:wsock32>
    $<$<BOOL:${QNXNTO}>:socket>
    asio::asio
    Threads::Threads
    eCAL::ecal-utils
    # we need this so that e.g. the test can just link ecal_core_private and will get the object files of this library.
    ecal_core_serialization $<TARGET_OBJECTS:ecal_core_serialization>
)

target_compile_features(ecal_core_private PUBLIC cxx_std_17)

set_property(TARGET ecal_core_private PROPERTY FOLDER core)
set_property(TARGET ecal_core_private PROPERTY POSITION_INDEPENDENT_CODE ON)

if(NOT ${CMAKE_VERSION} VERSION_LESS "3.8.0") 
  source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES 
    ${ecal_public_headers}
    ${ecal_public_v5_headers}
    ${ecal_sources}
  )
  source_group(TREE "${CMAKE_CURRENT_BINARY_DIR}" FILES 
    ${ecal_generated_public_headers}
    ${ecal_generated_sources}
    ${ecal_binary_sources}
  )
endif()

endmacro()


macro(add_ecal_core_library)

ecal_add_shared_library(core)
add_library(eCAL::core ALIAS core)
# We "just" link the objects to the ecal_core.dll/.so -> we manually need to set the linker language
set_target_properties(core PROPERTIES LINKER_LANGUAGE CXX)

target_sources(core
  PUBLIC
    FILE_SET core_headers
    TYPE HEADERS
    BASE_DIRS 
      ./include
      ${CMAKE_CURRENT_BINARY_DIR}/include
    FILES 
      ${ecal_public_headers}
      ${ecal_generated_public_headers}
  PRIVATE
    src/dummy.cpp
)

target_link_libraries(core
  PRIVATE
    ecal_core_private
)

target_compile_definitions(core
  INTERFACE
    ECAL_CORE_IMPORTS
)

foreach(CORE_FEATURE ${ECAL_CORE_FEATURES})
  if(${CORE_FEATURE})
    target_compile_definitions(core
      PUBLIC ${CORE_FEATURE})
  endif()
endforeach()

set_target_properties(core
  PROPERTIES 
    CXX_VISIBILITY_PRESET hidden
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

target_compile_features(core INTERFACE cxx_std_14)
target_compile_features(core PRIVATE cxx_std_17)

set_property(TARGET core PROPERTY FOLDER core)
endmacro()


add_ecal_serialization_object_library()
add_ecal_core_object_library()
add_ecal_core_library()


install(TARGETS core
  EXPORT eCALCoreTargets
  RUNTIME       DESTINATION "${eCAL_install_bin_dir}"         COMPONENT runtime
  LIBRARY       DESTINATION "${eCAL_install_lib_dir}"         COMPONENT sdk
  ARCHIVE       DESTINATION "${eCAL_install_archive_dyn_dir}" COMPONENT sdk
  FILE_SET core_headers
)
ecal_install_pdbs(TARGET core DESTINATION "${eCAL_install_bin_dir}" COMPONENT runtime) 

# eCAL process stub
if(UNIX)
    set(PROJECT_NAME_PROCESS_STUB process_stub)

    ecal_add_app_console(${PROJECT_NAME_PROCESS_STUB}
        src/ecal_process_stub.cpp
        src/ecal_process_stub.h
    )
    
    target_compile_features(${PROJECT_NAME_PROCESS_STUB} PUBLIC cxx_std_14)
    
    ecal_install_app(${PROJECT_NAME_PROCESS_STUB})

    set_property(TARGET ${PROJECT_NAME_PROCESS_STUB} PROPERTY FOLDER core)
endif(UNIX)
