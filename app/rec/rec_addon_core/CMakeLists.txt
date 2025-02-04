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

find_package(Threads REQUIRED)

set(PROJECT_NAME rec_addon_core)

set(source_files
    include/ecal/rec/recorder_impl_base_types.h
    include/ecal/rec/recorder_impl_base.h
    include/ecal/rec/concurrent_status_interface.h

    src/io_stream_server.cpp
    src/io_stream_server.h
    src/request_handler.cpp
    src/request_handler.h
    src/recorder.cpp
    src/recorder.h
    src/recorder_types.h
    src/time_limited_queue.h
    src/main.cpp
    src/function_descriptors.h
    src/common_types.h
) 

ecal_add_library (${PROJECT_NAME} ${source_files})
add_library (eCAL::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} 
  PRIVATE  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Threads::Threads
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14) 

ecal_install_library(${PROJECT_NAME})
install(DIRECTORY
  "include/" DESTINATION "${eCAL_install_include_dir}" COMPONENT sdk
   FILES_MATCHING PATTERN "*.h")

source_group(TREE "${CMAKE_CURRENT_LIST_DIR}"
    FILES
        ${source_files}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER app/rec)
