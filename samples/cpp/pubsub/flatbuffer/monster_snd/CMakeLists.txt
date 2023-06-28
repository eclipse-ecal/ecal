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

cmake_minimum_required(VERSION 3.10)

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

project(monster_snd)

find_package(FlatBuffers REQUIRED)
find_package(eCAL REQUIRED)


ecal_add_sample(${PROJECT_NAME} monster_snd.cpp)

flatbuffers_generate_headers(
  TARGET monster_snd_flatbuffers
  INCLUDE_PREFIX monster
  SCHEMAS monster/monster.fbs
  #BINARY_SCHEMAS_DIR "${MY_BINARY_SCHEMA_DIRECTORY}"
  FLAGS --gen-object-api
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE 
    monster_snd_flatbuffers
    flatbuffers::flatbuffers
    eCAL::core
)

ecal_install_sample(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER samples/cpp/monster)