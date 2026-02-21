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

project(proto_dyn_rec)

find_package(eCAL REQUIRED)
find_package(Protobuf REQUIRED)

set(proto_dyn_rec_src
    src/proto_dyn_rec.cpp
)

ecal_add_sample(${PROJECT_NAME} ${proto_dyn_rec_src})

target_include_directories(${PROJECT_NAME} PRIVATE .)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    eCAL::protobuf_core
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

ecal_install_sample(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER samples/cpp/pubsub/protobuf)
