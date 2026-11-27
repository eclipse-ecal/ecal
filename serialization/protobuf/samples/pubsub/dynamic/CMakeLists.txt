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

project(dynamic)

find_package(eCAL REQUIRED)
find_package(Protobuf REQUIRED)

ecal_add_sample(dynamic_receive dynamic_receive.cpp)
target_include_directories(dynamic_receive PRIVATE .)
target_link_libraries(dynamic_receive PRIVATE eCAL::protobuf_core)
target_compile_features(dynamic_receive PRIVATE cxx_std_17)
ecal_install_sample(dynamic_receive)
set_property(TARGET dynamic_receive PROPERTY FOLDER samples/cpp/pubsub/protobuf)

ecal_add_sample(dynamic_json_receive dynamic_json_receive.cpp)
target_link_libraries(dynamic_json_receive PRIVATE eCAL::protobuf_core)
target_compile_features(dynamic_json_receive PRIVATE cxx_std_17)
ecal_install_sample(dynamic_json_receive)
set_property(TARGET dynamic_json_receive PROPERTY FOLDER samples/cpp/pubsub/protobuf)