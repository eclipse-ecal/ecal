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

cmake_minimum_required(VERSION 3.13) # INTEFACE_LINK_OPTIONS is available since cmake 3.13

project(udpcap)

ecal_find_package(Npcap        REQUIRED)
ecal_find_package(PcapPlusPlus REQUIRED)
ecal_find_package(asio         REQUIRED)

set (source_files
    include/udpcap/host_address.h
    include/udpcap/npcap_helpers.h
    include/udpcap/udpcap_socket.h
    
    src/host_address.cpp
    src/log_debug.h
    src/npcap_helpers.cpp
    src/udpcap_socket.cpp
    src/udpcap_socket_private.cpp
    src/udpcap_socket_private.h
)

add_library (${PROJECT_NAME}
  ${source_files}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
    ${Npcap_INCLUDE_DIR}
    ${PcapPlusPlus_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    ${Npcap_LIBS}
    ${PcapPlusPlus_LIBS}
    wsock32
    ws2_32
    asio::asio
)

target_compile_definitions(${PROJECT_NAME}
  PUBLIC ASIO_STANDALONE
)


set_target_properties(${PROJECT_NAME}
  PROPERTIES INTERFACE_LINK_OPTIONS
  -DELAYLOAD:wpcap.dll
)

if(MSVC)
    # Create a source tree that mirrors the filesystem
    msvc_source_tree(${source_files})
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER lib/udpcap)