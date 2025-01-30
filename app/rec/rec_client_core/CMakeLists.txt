# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2025 Continental Corporation
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

find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(spdlog REQUIRED)

if (ECAL_USE_CURL)
    find_package(CURL REQUIRED)
    add_compile_definitions(ECAL_HAS_CURL)
endif ()

set(PROJECT_NAME rec_client_core)

set(source_files
    include/rec_client_core/ecal_rec.h
    include/rec_client_core/ecal_rec_defs.h
    include/rec_client_core/ecal_rec_logger.h
    include/rec_client_core/job_config.h
    include/rec_client_core/proto_helpers.h
    include/rec_client_core/rec_error.h
    include/rec_client_core/upload_config.h
    include/rec_client_core/record_mode.h
    include/rec_client_core/state.h
    include/rec_client_core/topic_info.h

    src/ecal_rec.cpp
    src/ecal_rec_impl.cpp
    src/ecal_rec_impl.h
    src/frame.h
    src/frame_buffer.cpp
    src/frame_buffer.h
    src/garbage_collector_trigger_thread.cpp
    src/garbage_collector_trigger_thread.h
    src/job_config.cpp
    src/monitoring_thread.cpp
    src/monitoring_thread.h
    src/proto_helpers.cpp

    src/addons/addon.cpp
    src/addons/addon.h
    src/addons/addon_manager.cpp
    src/addons/addon_manager.h
    src/addons/concurrent_queue.h
    src/addons/pipe_handler.cpp
    src/addons/pipe_handler.h
    src/addons/common_types.h
    src/addons/response_handler.cpp
    src/addons/response_handler.h
    src/addons/function_descriptors.h

    src/job/record_job.cpp
    src/job/record_job.h
    src/job/hdf5_writer_thread.cpp
    src/job/hdf5_writer_thread.h
) 

if (ECAL_USE_CURL)
    set(source_files
            ${source_files}
            src/job/ftp_upload_thread.cpp
            src/job/ftp_upload_thread.h
        )
endif()

add_library (${PROJECT_NAME} ${source_files})
add_library (eCAL::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_include_directories(${PROJECT_NAME} PUBLIC  include)

create_targets_protobuf()

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    spdlog::spdlog
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    protobuf::libprotobuf
    eCAL::protobuf_core
    eCAL::core_pb
    eCAL::app_pb
  PRIVATE
    eCAL::hdf5
    ThreadingUtils
    Threads::Threads
    eCAL::ecal-utils
    EcalParser
)

if(ECAL_USE_CURL)
    target_link_libraries(${PROJECT_NAME}
      PRIVATE
        CURL::libcurl
    )
endif()

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)

source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${source_files})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER app/rec)


#if (WIN32)
#    add_custom_target(libcurl_copy_dlls ALL
#        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:CURL::libcurl>              ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>
#        COMMAND ${CMAKE_COMMAND} -E copy $ENV{CTZS_LIBS}/bin/libcrypto-1_1.dll     ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>
#        COMMAND ${CMAKE_COMMAND} -E copy $ENV{CTZS_LIBS}/bin/libcrypto-1_1-x64.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>
#        COMMAND ${CMAKE_COMMAND} -E copy $ENV{CTZS_LIBS}/bin/libssl-1_1.dll        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>
#        COMMAND ${CMAKE_COMMAND} -E copy $ENV{CTZS_LIBS}/bin/libssl-1_1-x64.dll    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>
#    )
#    set_property(TARGET libcurl_copy_dlls PROPERTY FOLDER app/rec)
#
#    add_dependencies(${PROJECT_NAME} libcurl_copy_dlls)
#endif()
