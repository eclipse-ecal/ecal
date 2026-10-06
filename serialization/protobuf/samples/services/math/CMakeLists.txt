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

project(math)

find_package(eCAL REQUIRED)
find_package(Protobuf REQUIRED)

set(math_proto
    ${CMAKE_CURRENT_SOURCE_DIR}/math.proto
)

ecal_add_sample(math_client math_client.cpp)
PROTOBUF_TARGET_CPP(math_client ${CMAKE_CURRENT_SOURCE_DIR} ${math_proto})
target_link_libraries(math_client PRIVATE eCAL::protobuf_core)
target_compile_features(math_client PRIVATE cxx_std_14)
ecal_install_sample(math_client)
set_property(TARGET math_client PROPERTY FOLDER samples/cpp/services/math)

ecal_add_sample(math_server math_server.cpp)
PROTOBUF_TARGET_CPP(math_server ${CMAKE_CURRENT_SOURCE_DIR} ${math_proto})
target_link_libraries(math_server PRIVATE eCAL::protobuf_core)
target_compile_features(math_server PRIVATE cxx_std_14)
ecal_install_sample(math_server)
set_property(TARGET math_server PROPERTY FOLDER samples/cpp/services/math)