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

cmake_minimum_required(VERSION 3.10)

project(person_measurement)

find_package(eCAL REQUIRED)
find_package(Protobuf REQUIRED)

macro(create_measurement_sample target_name sample_sources)
  set(person_proto
      ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/animal.proto
      ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/house.proto
      ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/person.proto
  )
  
  ecal_add_sample(${target_name} ${sample_sources})
  PROTOBUF_TARGET_CPP(${target_name} ${CMAKE_CURRENT_SOURCE_DIR}/protobuf ${person_proto})
  
  target_link_libraries(${target_name}
    PRIVATE
      eCAL::protobuf_measurement
  )
  
  target_compile_features(${target_name} PRIVATE cxx_std_14)
  
  ecal_install_sample(${target_name})
  
  set_property(TARGET ${target_name} PROPERTY FOLDER samples/cpp/measurement/protobuf)
endmacro()

create_measurement_sample(person_read   person_read.cpp)
create_measurement_sample(person_write  person_write.cpp)


