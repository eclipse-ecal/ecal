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

ecal_find_package(Threads      REQUIRED)
ecal_find_package(asio         REQUIRED)
ecal_find_package(tclap        REQUIRED)
ecal_find_package(simpleini    REQUIRED)

if (ECAL_JOIN_MULTICAST_TWICE)
  message(STATUS "eCAL ${PROJECT_NAME}: Enabling Specific Multicast Network Bug Workaround")
  add_definitions(-DECAL_JOIN_MULTICAST_TWICE)
endif(ECAL_JOIN_MULTICAST_TWICE)

# If we're currently doing a build within a git repository, we will configure the header files.
# Else, (e.g. for source packages such as debian source packages) we will use a preconfigured file.
# If there is really no information available, it will generate a dummy version file 0.0.0
if     (IS_GIT_TREE OR NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/ecal/ecal_defs.h")
  configure_file(src/ecal_defs.h.in "${CMAKE_CURRENT_BINARY_DIR}/include/ecal/ecal_defs.h" @ONLY)
endif  (IS_GIT_TREE OR NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/ecal/ecal_defs.h")

set(ecal_io_cpp_src
    src/io/rcv_sample.cpp
    src/io/snd_raw_buffer.cpp
    src/io/snd_sample.cpp
    src/io/udp_init.cpp
    src/io/udp_receiver.cpp
    src/io/udp_sender.cpp
)

if(NOT ECAL_LAYER_ICEORYX)
  set(ecal_io_mem_cpp_src
      src/io/ecal_memfile.cpp
      src/io/ecal_memfile_pool.cpp
  )
  if(UNIX)
    set(ecal_io_mem_cpp_linux_src
        src/io/linux/ecal_memfile_linux.cpp
    )
  endif()

  if(WIN32)
    set(ecal_io_mem_cpp_win_src
       src/io/win32/ecal_memfile_win32.cpp
    )
  endif()
endif(NOT ECAL_LAYER_ICEORYX)

set(ecal_mon_cpp_src
    src/mon/ecal_monitoring_def.cpp
    src/mon/ecal_monitoring_impl.cpp
    src/mon/ecal_monitoring_threads.cpp
)

set(ecal_pubsub_cpp_src
    src/pubsub/ecal_proto_dyn_json_sub.cpp
    src/pubsub/ecal_pubgate.cpp
    src/pubsub/ecal_publisher.cpp
    src/pubsub/ecal_subgate.cpp
    src/pubsub/ecal_subscriber.cpp
)

set(ecal_readwrite_cpp_src
    src/readwrite/ecal_reader.cpp
    src/readwrite/ecal_reader_inproc.cpp
    src/readwrite/ecal_reader_udp.cpp
    src/readwrite/ecal_reader_udp_mc.cpp
    src/readwrite/ecal_writer.cpp
    src/readwrite/ecal_writer_inproc.cpp
    src/readwrite/ecal_writer_udp_mc.cpp
)

if(ECAL_LAYER_ICEORYX)
  set(ecal_readwrite_shm_cpp_src
    src/readwrite/ecal_reader_iceoryx.cpp
    src/readwrite/ecal_writer_iceoryx.cpp
  )
else(ECAL_LAYER_ICEORYX)
  set(ecal_readwrite_shm_cpp_src
    src/readwrite/ecal_reader_shm.cpp
    src/readwrite/ecal_writer_shm.cpp
  )
endif(ECAL_LAYER_ICEORYX)

set(ecal_service_cpp_src
    src/service/ecal_service_client.cpp
    src/service/ecal_service_client_impl.cpp
    src/service/ecal_service_server.cpp
    src/service/ecal_service_server_impl.cpp
    src/service/ecal_tcpclient.cpp
    src/service/ecal_tcpserver.cpp
)

set(ecal_cmn_cpp_src
    src/convert_utf.cpp
    src/ecal.cpp
    src/ecal_clang.cpp
    src/ecal_config.cpp
    src/ecal_descgate.cpp
    src/ecal_event.cpp
    src/ecal_global_accessors.cpp
    src/ecal_globals.cpp
    src/ecal_log.cpp
    src/ecal_log_impl.cpp
    src/ecal_process.cpp
    src/ecal_reggate.cpp
    src/ecal_register.cpp
    src/ecal_servgate.cpp
    src/ecal_thread.cpp
    src/ecal_time.cpp
    src/ecal_timegate.cpp
    src/ecal_timer.cpp
    src/ecal_util.cpp
    src/ecalc.cpp
    src/sys_usage.cpp
)

set(ecal_io_header_src
    src/io/ecal_message.h
    src/io/ecal_receiver.h
    src/io/ecal_sender.h
    src/io/msg_type.h
    src/io/rcv_sample.h
    src/io/snd_raw_buffer.h
    src/io/snd_sample.h
    src/io/udp_init.h
    src/io/udp_receiver.h
    src/io/udp_sender.h
)

if(NOT ECAL_LAYER_ICEORYX)
  set(ecal_io_mem_header_src
      src/io/ecal_memfile.h
      src/io/ecal_memfile_mtx.h
      src/io/ecal_memfile_pool.h
  )

  if(WIN32)
    set(ecal_io_mem_header_win_src
        src/io/win32/ecal_memfile_mtx.h
    )
  endif()

  if(UNIX)
    set(ecal_io_mem_header_linux_src
        src/io/linux/ecal_memfile_mtx.h
  )
  endif()
endif(NOT ECAL_LAYER_ICEORYX)

set(ecal_mon_header_src
    src/mon/ecal_monitoring_def.h
    src/mon/ecal_monitoring_impl.h
    src/mon/ecal_monitoring_threads.h
)

set(ecal_pubsub_header_src
    src/pubsub/ecal_pubgate.h
    src/pubsub/ecal_subgate.h
)

set(ecal_readwrite_header_src
    src/readwrite/ecal_reader.h
    src/readwrite/ecal_reader_inproc.h
    src/readwrite/ecal_reader_udp.h
    src/readwrite/ecal_reader_udp_mc.h
    src/readwrite/ecal_writer.h
    src/readwrite/ecal_writer_base.h
    src/readwrite/ecal_writer_inproc.h
    src/readwrite/ecal_writer_udp_mc.h
)

if(ECAL_LAYER_ICEORYX)
  set(ecal_readwrite_shm_header_src
      src/readwrite/ecal_reader_iceoryx.h
      src/readwrite/ecal_writer_iceoryx.h
  )
else(ECAL_LAYER_ICEORYX)
  set(ecal_readwrite_shm_header_src
      src/readwrite/ecal_reader_shm.h
      src/readwrite/ecal_writer_shm.h
  )
endif(ECAL_LAYER_ICEORYX)

set(ecal_service_header_src
    src/service/asio_server.h
    src/service/ecal_service_client_impl.h
    src/service/ecal_service_server_impl.h
    src/service/ecal_tcpclient.h
    src/service/ecal_tcpserver.h
)

set(ecal_cmn_header_src
    src/convert_utf.h
    src/ecal_config.h
    src/ecal_config_hlp.h
    src/ecal_def.h
    src/ecal_def_ini.h
    src/ecal_descgate.h
    src/ecal_expmap.h
    src/ecal_global_accessors.h
    src/ecal_globals.h
    src/ecal_log_impl.h
    src/ecal_reggate.h
    src/ecal_register.h
    src/ecal_servgate.h
    src/ecal_thread.h
    src/ecal_timegate.h
    src/ecal_win_main.h
    src/ecal_win_socket.h
    src/getenvvar.h
    src/sys_usage.h
    src/topic2mcast.h
)

set(ecal_c_src
    src/ecalc.cpp
)

if(WIN32)
  set(ecal_c_win_src
      src/win32/dll/dllmain.cpp
      src/win32/dll/ecal.rc
  )
endif()

set(ecal_header_cmn
    include/ecal/ecal.h
    include/ecal/ecal_callback.h
    include/ecal/ecal_clang.h
    include/ecal/ecal_client.h
    include/ecal/ecal_core.h
    include/ecal/ecal_event.h
    include/ecal/ecal_eventhandle.h
    include/ecal/ecal_init.h
    include/ecal/ecal_log.h
    include/ecal/ecal_log_level.h
    include/ecal/ecal_monitoring.h
    include/ecal/ecal_os.h
    include/ecal/ecal_process.h
    include/ecal/ecal_process_mode.h
    include/ecal/ecal_process_severity.h
    include/ecal/ecal_publisher.h
    include/ecal/ecal_qos.h
    include/ecal/ecal_server.h
    include/ecal/ecal_service.h
    include/ecal/ecal_service_info.h
    include/ecal/ecal_subscriber.h
    include/ecal/ecal_time.h
    include/ecal/ecal_timed_cb.h
    include/ecal/ecal_timer.h
    include/ecal/ecal_tlayer.h
    include/ecal/ecal_util.h
    include/ecal/ecalc.h
)

set(ecal_header_cimpl
    include/ecal/cimpl/ecal_callback_cimpl.h
    include/ecal/cimpl/ecal_core_cimpl.h
    include/ecal/cimpl/ecal_event_cimpl.h
    include/ecal/cimpl/ecal_init_cimpl.h
    include/ecal/cimpl/ecal_log_cimpl.h
    include/ecal/cimpl/ecal_monitoring_cimpl.h
    include/ecal/cimpl/ecal_process_cimpl.h
    include/ecal/cimpl/ecal_proto_dyn_json_subscriber_cimpl.h
    include/ecal/cimpl/ecal_publisher_cimpl.h
    include/ecal/cimpl/ecal_qos_cimpl.h
    include/ecal/cimpl/ecal_service_cimpl.h
    include/ecal/cimpl/ecal_service_info_cimpl.h
    include/ecal/cimpl/ecal_subscriber_cimpl.h
    include/ecal/cimpl/ecal_time_cimpl.h
    include/ecal/cimpl/ecal_timer_cimpl.h
    include/ecal/cimpl/ecal_tlayer_cimpl.h
    include/ecal/cimpl/ecal_util_cimpl.h
)

set(ecal_header_msg
    include/ecal/msg/capnproto/dynamic.h
    include/ecal/msg/capnproto/helper.h
    include/ecal/msg/capnproto/publisher.h
    include/ecal/msg/capnproto/subscriber.h
    include/ecal/msg/flatbuffers/publisher.h
    include/ecal/msg/flatbuffers/subscriber.h
    include/ecal/msg/messagepack/publisher.h
    include/ecal/msg/messagepack/subscriber.h
    include/ecal/msg/protobuf/client.h
    include/ecal/msg/protobuf/dynamic_json_subscriber.h
    include/ecal/msg/protobuf/dynamic_subscriber.h
    include/ecal/msg/protobuf/publisher.h
    include/ecal/msg/protobuf/server.h
    include/ecal/msg/protobuf/subscriber.h
    include/ecal/msg/string/publisher.h
    include/ecal/msg/string/subscriber.h
    include/ecal/msg/dynamic.h
    include/ecal/msg/publisher.h
    include/ecal/msg/subscriber.h
)

set(ecal_header_base
    ${ecal_header_cmn}
    ${ecal_header_cimpl}
    ${ecal_header_msg}
)

ecal_add_ecal_shared_library(${PROJECT_NAME} 
    ${ecal_io_cpp_src} 
    ${ecal_io_mem_cpp_src} 
    ${ecal_io_mem_cpp_win_src} 
    ${ecal_io_mem_cpp_linux_src} 
    ${ecal_mon_cpp_src} 
    ${ecal_pubsub_cpp_src} 
    ${ecal_readwrite_cpp_src} 
    ${ecal_readwrite_shm_cpp_src}
    ${ecal_service_cpp_src}
    ${ecal_cmn_cpp_src}

    ${ecal_cmn_header_src}
    ${ecal_io_header_src}
    ${ecal_io_mem_header_src} 
    ${ecal_io_mem_header_win_src} 
    ${ecal_io_mem_header_linux_src} 
    ${ecal_mon_header_src}
    ${ecal_pubsub_header_src}
    ${ecal_readwrite_header_src}
    ${ecal_readwrite_shm_header_src}
    ${ecal_service_header_src}

    ${ecal_header_base}
    ${ecal_header_cimpl}
    ${ecal_header_msg}
    
    ${CMAKE_CURRENT_BINARY_DIR}/include/ecal/ecal_defs.h
)

if(UNIX)
  set_source_files_properties(convert_utf.cpp PROPERTIES COMPILE_FLAGS -Wno-implicit-fallthrough)
endif()

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

if(ECAL_LAYER_ICEORYX)
  if(UNIX)
    ecal_find_package(iceoryx_posh REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE iceoryx_posh::iceoryx_posh)
  endif()
    target_compile_definitions(${PROJECT_NAME} PUBLIC ECAL_LAYER_ICEORYX)
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
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME} 
  PUBLIC
    eCAL::proto
  PRIVATE
    $<$<BOOL:${UNIX}>:dl>
    $<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>>:rt>
    $<$<BOOL:${WIN32}>:iphlpapi>
    $<$<BOOL:${WIN32}>:psapi> 
    $<$<BOOL:${WIN32}>:shlwapi.lib> 
    $<$<BOOL:${WIN32}>:winmm> 
    $<$<BOOL:${WIN32}>:ws2_32> 
    $<$<BOOL:${WIN32}>:wsock32>
    asio::asio
    tclap::tclap
    CustomTclap
    simpleini::simpleini
    eCAL::pb
    Threads::Threads
    eCAL::apps
    eCAL::utils
)

set_property(TARGET ${PROJECT_NAME}   PROPERTY FOLDER ecal/core)
set_property(TARGET ${PROJECT_NAME}_c PROPERTY FOLDER ecal/core)

ecal_install_ecal_shared_library(${PROJECT_NAME}_c)
ecal_install_ecal_shared_library(${PROJECT_NAME})

install(DIRECTORY
   "include/" DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT sdk
    FILES_MATCHING PATTERN "*.h")

#Install generated ecal_defs.h file    
install(DIRECTORY
   "${CMAKE_CURRENT_BINARY_DIR}/include/" DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT sdk
    FILES_MATCHING PATTERN "*.h")
    

if(NOT ${CMAKE_VERSION} VERSION_LESS "3.8.0") 
  source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES 
    ${ecal_io_cpp_src}
    ${ecal_io_mem_cpp_src}
    ${ecal_io_mem_cpp_linux_src} 
    ${ecal_io_mem_cpp_win_src} 
    ${ecal_io_cpp_linux_src}
    ${ecal_io_cpp_win32_src}
    ${ecal_mon_cpp_src}
    ${ecal_pubsub_cpp_src}
    ${ecal_readwrite_cpp_src}
    ${ecal_readwrite_shm_cpp_src}
    ${ecal_service_cpp_src}
    ${ecal_cmn_cpp_src}
    ${ecal_c_src}
    ${ecal_c_win_src}
  )
  
  source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Header Files\\internal" FILES 
    ${ecal_cmn_header_src}
    ${ecal_io_header_src}
    ${ecal_io_mem_header_src}
    ${ecal_io_mem_header_win_src}
    ${ecal_io_header_linux_src}
    ${ecal_io_header_win_src}
    ${ecal_mon_header_src}
    ${ecal_pubsub_header_src}
    ${ecal_readwrite_header_src}
    ${ecal_readwrite_shm_header_src}
    ${ecal_service_header_src}
  )
  
  source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "Header Files\\public" FILES 
    ${ecal_header_cmn}
    ${ecal_header_cimpl}
    ${ecal_header_msg}
  )
endif()

# eCAL Process stub
if(UNIX)
    set(PROJECT_NAME_PROCESS_STUB process_stub)

    ecal_add_app_console(${PROJECT_NAME_PROCESS_STUB}
        src/ecal_process_stub.cpp
        src/ecal_process_stub.h
    )

    ecal_install_app(${PROJECT_NAME_PROCESS_STUB})

    set_property(TARGET ${PROJECT_NAME_PROCESS_STUB} PROPERTY FOLDER ecal/core)
endif(UNIX)
