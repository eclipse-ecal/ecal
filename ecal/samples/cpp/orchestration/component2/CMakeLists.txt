# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2022 Eclipse Foundation
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

cmake_minimum_required(VERSION 3.10)

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

project(component2)

find_package(eCAL REQUIRED)
find_package(Protobuf REQUIRED)

set(component2_src
    src/component2.cpp
)

set(component2_proto
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protobuf/component.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protobuf/orchestrator.proto
)
ecal_add_sample(${PROJECT_NAME} ${component2_src})
PROTOBUF_TARGET_CPP(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/protobuf ${component2_proto})

target_link_libraries(${PROJECT_NAME}
    eCAL::core
    protobuf::libprotobuf
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

ecal_install_sample(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER samples/cpp/orchestration)
