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

cmake_minimum_required(VERSION 3.15)

project(ping)

find_package(eCAL REQUIRED)
find_package(Protobuf REQUIRED)

set(ping_proto
    ${CMAKE_CURRENT_SOURCE_DIR}/ping.proto
)

ecal_add_sample(ping_client ping_client.cpp)
PROTOBUF_TARGET_CPP(ping_client ${CMAKE_CURRENT_SOURCE_DIR} ${ping_proto})
target_link_libraries(ping_client PRIVATE eCAL::protobuf_core)
target_compile_features(ping_client PRIVATE cxx_std_14)
ecal_install_sample(ping_client)
set_property(TARGET ping_client PROPERTY FOLDER samples/cpp/services/ping)

ecal_add_sample(ping_client_dyn
  ping_client_dyn.cpp
  proto_json_conv.cpp
  proto_json_conv.h
)
target_link_libraries(ping_client_dyn PRIVATE eCAL::protobuf_core)
target_compile_features(ping_client_dyn PRIVATE cxx_std_14)
ecal_install_sample(ping_client_dyn)
set_property(TARGET ping_client_dyn PROPERTY FOLDER samples/cpp/services/ping)

ecal_add_sample(ping_server ping_server.cpp)
PROTOBUF_TARGET_CPP(ping_server ${CMAKE_CURRENT_SOURCE_DIR} ${ping_proto})
target_link_libraries(ping_server PRIVATE eCAL::protobuf_core)
target_compile_features(ping_server PRIVATE cxx_std_14)
ecal_install_sample(ping_server)
set_property(TARGET ping_server PROPERTY FOLDER samples/cpp/services/ping)