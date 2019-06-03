# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2019 Continental Corporation
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

project(core VERSION ${eCAL_VERSION_STRING})

find_package(Threads REQUIRED)
find_package(asio    REQUIRED)
find_package(tclap   REQUIRED)

if (ECAL_JOIN_MULTICAST_TWICE)
  message(STATUS "eCAL ${PROJECT_NAME}: Enabling Specific Multicast Network Bug Workaround")
  add_definitions(-DECAL_JOIN_MULTICAST_TWICE)
endif(ECAL_JOIN_MULTICAST_TWICE)

# If we're currently doing a build within a git repository, we will configure the header files.
# Else, (e.g. for source packages such as debian source packages) we will use a preconfigured file.
# If there is really no information available, it will generate a dummy version file 0.0.0
if     (IS_GIT_TREE OR NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../include/ecal/ecal_defs.h")
  configure_file(ecal_defs.h.in "${CMAKE_CURRENT_BINARY_DIR}/include/ecal/ecal_defs.h" @ONLY)
endif  (IS_GIT_TREE OR NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../include/ecal/ecal_defs.h")

set(ecal_io_cpp_src
    io/ecal_memfile.cpp
    io/ecal_memfile_pool.cpp
    io/rcv_sample.cpp
    io/snd_raw_buffer.cpp
    io/snd_sample.cpp
    io/udp_init.cpp
    io/udp_receiver.cpp
    io/udp_sender.cpp
)

set(ecal_mon_cpp_src
    mon/ecal_monitoring_def.cpp
    mon/ecal_monitoring_impl.cpp
    mon/ecal_monitoring_threads.cpp
)

set(ecal_pubsub_cpp_src
    pubsub/ecal_proto_dyn_json_sub.cpp
    pubsub/ecal_pubgate.cpp
    pubsub/ecal_publisher.cpp
    pubsub/ecal_subgate.cpp
    pubsub/ecal_subscriber.cpp
)

set(ecal_readwrite_lcm_cpp_src
    readwrite/lcm/lcmlite.cpp
)

if(ECAL_LAYER_FASTRTPS)
  set(ecal_readwrite_rtps_msg_cpp_src
      readwrite/fastrtps/string_message_.cxx
      readwrite/fastrtps/string_message_PubSubTypes.cxx
  )

  set(ecal_readwrite_rtps_cpp_src
    readwrite/ecal_reader_rtps.cpp
    readwrite/ecal_writer_rtps.cpp
  )
endif()

set(ecal_readwrite_cpp_src
    readwrite/ecal_reader.cpp
    readwrite/ecal_reader_inproc.cpp
    readwrite/ecal_reader_lcm.cpp
    readwrite/ecal_reader_metal.cpp
    readwrite/ecal_reader_shm.cpp
    readwrite/ecal_reader_udp.cpp
    readwrite/ecal_reader_udp_mc.cpp
    readwrite/ecal_reader_udp_uc.cpp
    readwrite/ecal_writer.cpp
    readwrite/ecal_writer_inproc.cpp
    readwrite/ecal_writer_lcm.cpp
    readwrite/ecal_writer_shm.cpp
    readwrite/ecal_writer_udp_mc.cpp
    readwrite/ecal_writer_udp_uc.cpp
)

set(ecal_service_cpp_src
    service/ecal_service_client.cpp
    service/ecal_service_client_impl.cpp
    service/ecal_service_server.cpp
    service/ecal_service_server_impl.cpp
    service/ecal_tcpclient.cpp
    service/ecal_tcpserver.cpp
)

set(ecal_cmn_cpp_src
    convert_utf.cpp
    ecal.cpp
    ecal_clang.cpp
    ecal_config.cpp
    ecal_descgate.cpp
    ecal_event.cpp
    ecal_global_accessors.cpp
    ecal_globals.cpp
    ecal_log.cpp
    ecal_log_impl.cpp
    ecal_process.cpp
    ecal_reggate.cpp
    ecal_register.cpp
    ecal_servgate.cpp
    ecal_thread.cpp
    ecal_time.cpp
    ecal_timegate.cpp
    ecal_timer.cpp
    ecal_util.cpp
    ecalc.cpp
    simpleini_template.cpp
    sys_usage.cpp
)

set(ecal_io_header_src
    io/ecal_memfile.h
    io/ecal_memfile_mtx.h
    io/ecal_memfile_pool.h
    io/ecal_message.h
    io/ecal_receiver.h
    io/ecal_sender.h
    io/msg_type.h
    io/rcv_sample.h
    io/snd_raw_buffer.h
    io/snd_sample.h
    io/udp_init.h
    io/udp_receiver.h
    io/udp_sender.h
)

set(ecal_mon_header_src
    mon/ecal_monitoring_def.h
    mon/ecal_monitoring_impl.h
    mon/ecal_monitoring_threads.h
)

set(ecal_pubsub_header_src
    pubsub/ecal_pubgate.h
    pubsub/ecal_subgate.h
)

set(ecal_readwrite_lcm_header_src
    readwrite/lcm/lcmlite.h
)

if(ECAL_LAYER_FASTRTPS)
  set(ecal_readwrite_rtps_msg_header_src
      readwrite/fastrtps/string_message_.h
      readwrite/fastrtps/string_message_PubSubTypes.h
)
set(ecal_readwrite_rtps_header_src
    readwrite/ecal_writer_rtps.h
)
endif()

set(ecal_readwrite_header_src
    readwrite/ecal_reader.h
    readwrite/ecal_reader_inproc.h
    readwrite/ecal_reader_lcm.h
    readwrite/ecal_reader_metal.h
    readwrite/ecal_reader_shm.h
    readwrite/ecal_reader_udp.h
    readwrite/ecal_reader_udp_mc.h
    readwrite/ecal_reader_udp_uc.h
    readwrite/ecal_writer.h
    readwrite/ecal_writer_base.h
    readwrite/ecal_writer_inproc.h
    readwrite/ecal_writer_lcm.h
    readwrite/ecal_writer_shm.h
    readwrite/ecal_writer_udp_mc.h
    readwrite/ecal_writer_udp_uc.h
)

set(ecal_service_header_src
    service/asio_server.h
    service/ecal_service_client_impl.h
    service/ecal_service_server_impl.h
    service/ecal_tcpclient.h
    service/ecal_tcpserver.h
)

set(ecal_cmn_header_src
    convert_utf.h
    ecal_config.h
    ecal_config_hlp.h
    ecal_def.h
    ecal_def_ini.h
    ecal_descgate.h
    ecal_expmap.h
    ecal_global_accessors.h
    ecal_globals.h
    ecal_log_impl.h
    ecal_reggate.h
    ecal_register.h
    ecal_servgate.h
    ecal_thread.h
    ecal_timegate.h
    ecal_win_main.h
    ecal_win_socket.h
    getenvvar.h
    simpleini.h
    simpleini_template.h
    sys_usage.h
    topic2mcast.h
)

set(ecal_c_src
    ecalc.cpp
)

if(WIN32)
  set(ecal_c_win_src
      win32/dll/dllmain.cpp
      win32/dll/ecal.rc
  )
endif()

set(ecal_header_cmn
    ../include/ecal/ecal.h
    ../include/ecal/ecal_callback.h
    ../include/ecal/ecal_clang.h
    ../include/ecal/ecal_client.h
    ../include/ecal/ecal_core.h
    ../include/ecal/ecal_event.h
    ../include/ecal/ecal_eventhandle.h
    ../include/ecal/ecal_init.h
    ../include/ecal/ecal_log.h
    ../include/ecal/ecal_log_level.h
    ../include/ecal/ecal_monitoring.h
    ../include/ecal/ecal_msg.h
    ../include/ecal/ecal_os.h
    ../include/ecal/ecal_process.h
    ../include/ecal/ecal_process_mode.h
    ../include/ecal/ecal_process_severity.h
    ../include/ecal/ecal_publisher.h
    ../include/ecal/ecal_qos.h
    ../include/ecal/ecal_server.h
    ../include/ecal/ecal_service.h
    ../include/ecal/ecal_service_info.h
    ../include/ecal/ecal_subscriber.h
    ../include/ecal/ecal_time.h
    ../include/ecal/ecal_timed_cb.h
    ../include/ecal/ecal_timer.h
    ../include/ecal/ecal_tlayer.h
    ../include/ecal/ecal_util.h
    ../include/ecal/ecalc.h
)

set(ecal_header_cimpl
    ../include/ecal/cimpl/ecal_core_cimpl.h
    ../include/ecal/cimpl/ecal_event_cimpl.h
    ../include/ecal/cimpl/ecal_init_cimpl.h
    ../include/ecal/cimpl/ecal_log_cimpl.h
    ../include/ecal/cimpl/ecal_monitoring_cimpl.h
    ../include/ecal/cimpl/ecal_process_cimpl.h
    ../include/ecal/cimpl/ecal_proto_dyn_json_subscriber_cimpl.h
    ../include/ecal/cimpl/ecal_publisher_cimpl.h
    ../include/ecal/cimpl/ecal_qos_cimpl.h
    ../include/ecal/cimpl/ecal_service_cimpl.h
    ../include/ecal/cimpl/ecal_subscriber_cimpl.h
    ../include/ecal/cimpl/ecal_time_cimpl.h
    ../include/ecal/cimpl/ecal_timer_cimpl.h
    ../include/ecal/cimpl/ecal_tlayer_cimpl.h
    ../include/ecal/cimpl/ecal_util_cimpl.h
)

set(ecal_header_msg
    ../include/ecal/msg/capnproto/publisher.h
    ../include/ecal/msg/capnproto/subscriber.h
    ../include/ecal/msg/flatbuffers/publisher.h
    ../include/ecal/msg/flatbuffers/subscriber.h
    ../include/ecal/msg/messagepack/publisher.h
    ../include/ecal/msg/messagepack/subscriber.h
    ../include/ecal/msg/protobuf/client.h
    ../include/ecal/msg/protobuf/dynamic_json_subscriber.h
    ../include/ecal/msg/protobuf/dynamic_subscriber.h
    ../include/ecal/msg/protobuf/publisher.h
    ../include/ecal/msg/protobuf/server.h
    ../include/ecal/msg/protobuf/subscriber.h
    ../include/ecal/msg/string/publisher.h
    ../include/ecal/msg/string/subscriber.h
    ../include/ecal/msg/publisher.h
    ../include/ecal/msg/subscriber.h
)

set(ecal_header_base
    ${ecal_header_cmn}
    ${ecal_header_cimpl}
    ${ecal_header_msg}
)

ecal_add_ecal_shared_library(${PROJECT_NAME} 
    ${ecal_io_cpp_src} 
    ${ecal_mon_cpp_src} 
    ${ecal_pubsub_cpp_src} 
    ${ecal_readwrite_lcm_cpp_src} 
    ${ecal_readwrite_cpp_src} 
    ${ecal_readwrite_rtps_msg_cpp_src}
    ${ecal_readwrite_rtps_cpp_src}
    ${ecal_service_cpp_src}
    ${ecal_cmn_cpp_src}

    ${ecal_cmn_header_src}
    ${ecal_io_header_src}
    ${ecal_mon_header_src}
    ${ecal_pubsub_header_src}
    ${ecal_readwrite_lcm_header_src}
    ${ecal_readwrite_header_src}
    ${ecal_readwrite_rtps_msg_header_src}
    ${ecal_readwrite_rtps_header_src}
    ${ecal_service_header_src}

    ${ecal_header_base}
    ${ecal_header_cimpl}
    ${ecal_header_msg}
    
    ${CMAKE_CURRENT_BINARY_DIR}/include/ecal/ecal_defs.h
)

ecal_add_ecal_shared_library(${PROJECT_NAME}_c ${ecal_c_src} ${ecal_c_win_src})

add_library(eCAL::${PROJECT_NAME}   ALIAS ${PROJECT_NAME})
add_library(eCAL::${PROJECT_NAME}_c ALIAS ${PROJECT_NAME}_c)

target_link_libraries(${PROJECT_NAME}_c ${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME}_c
  INTERFACE ECAL_C_DLL
  PUBLIC ASIO_STANDALONE
  PRIVATE eCAL_EXPORTS)

target_compile_definitions(${PROJECT_NAME}
  PUBLIC ASIO_STANDALONE
  PRIVATE eCAL_EXPORTS)

if(ECAL_LAYER_FASTRTPS)
  if(WIN32)
    find_package(fastrtps REQUIRED)
    target_include_directories(${PROJECT_NAME} PRIVATE ${FASTRTPS_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${FASTRTPS_LIBRARIES})
  endif()

  if(UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE fastcdr fastrtps)
  endif()
    target_compile_definitions(${PROJECT_NAME} PUBLIC ECAL_LAYER_FASTRTPS)
endif()

if(ECAL_NPCAP_SUPPORT)
  add_definitions(-DECAL_NPCAP_SUPPORT)
  target_link_libraries(${PROJECT_NAME}
    PRIVATE
      udpcap
  )
 endif(ECAL_NPCAP_SUPPORT)

target_include_directories(${PROJECT_NAME}
  PRIVATE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME} 
  PUBLIC
    eCAL::proto
  PRIVATE
    $<$<BOOL:${UNIX}>:rt>
    $<$<BOOL:${UNIX}>:dl>
    $<$<BOOL:${WIN32}>:iphlpapi>
    $<$<BOOL:${WIN32}>:psapi> 
    $<$<BOOL:${WIN32}>:shlwapi.lib> 
    $<$<BOOL:${WIN32}>:winmm> 
    $<$<BOOL:${WIN32}>:ws2_32> 
    $<$<BOOL:${WIN32}>:wsock32>
    asio::asio
    tclap::tclap
    eCAL::pb
    Threads::Threads
    eCAL::apps
    EcalUtils
)

set_property(TARGET ${PROJECT_NAME}   PROPERTY FOLDER ecal/core)
set_property(TARGET ${PROJECT_NAME}_c PROPERTY FOLDER ecal/core)

ecal_install_ecal_shared_library(${PROJECT_NAME}_c)
ecal_install_ecal_shared_library(${PROJECT_NAME})

install(DIRECTORY
   "../include/" DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT sdk
    FILES_MATCHING PATTERN "*.h")

#Install generated ecal_defs.h file    
install(DIRECTORY
   "${CMAKE_CURRENT_BINARY_DIR}/include/" DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT sdk
    FILES_MATCHING PATTERN "*.h")
    

if(NOT ${CMAKE_VERSION} VERSION_LESS "3.8.0") 
  source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES 
    ${ecal_io_cpp_src}
    ${ecal_mon_cpp_src}
    ${ecal_pubsub_cpp_src}
    ${ecal_readwrite_rtps_msg_cpp_src}
    ${ecal_readwrite_lcm_cpp_src}
    ${ecal_readwrite_cpp_src}
    ${ecal_readwrite_rtps_cpp_src}
    ${ecal_service_cpp_src}
  )
  
  source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Header Files\\internal" FILES 
    ${ecal_cmn_header_src}
    ${ecal_io_header_src}
    ${ecal_mon_header_src}
    ${ecal_pubsub_header_src}
    ${ecal_readwrite_rtps_msg_header_src}
    ${ecal_readwrite_lcm_header_src}
    ${ecal_readwrite_header_src}
    ${ecal_readwrite_rtps_header_src}
    ${ecal_service_header_src}
  )
  
  source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/../include" PREFIX "Header Files\\public" FILES 
    ${ecal_header_cmn}
    ${ecal_header_cimpl}
    ${ecal_header_msg}
  )
endif()

# eCAL Process stub
if(UNIX)
    set(PROJECT_NAME_PROCESS_STUB process_stub)

    ecal_add_app_console(${PROJECT_NAME_PROCESS_STUB}
        ecal_process_stub.cpp
        ecal_process_stub.h
    )

    ecal_install_app(${PROJECT_NAME_PROCESS_STUB})

    set_property(TARGET ${PROJECT_NAME_PROCESS_STUB} PROPERTY FOLDER ecal/core)
endif(UNIX)
