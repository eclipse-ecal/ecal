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

project(monster_measurement)

find_package(eCAL REQUIRED)
find_package(FlatBuffers REQUIRED)

# Create library coontaining the fbs files, which are used by the samples
flatbuffers_generate_headers(
  TARGET monster_measurement
  INCLUDE_PREFIX monster
  SCHEMAS monster/monster.fbs
  #BINARY_SCHEMAS_DIR "${MY_BINARY_SCHEMA_DIRECTORY}"
  FLAGS --gen-object-api
)
set_property(TARGET monster_measurement PROPERTY FOLDER samples/cpp/pubsub/flatbuffers)

# Macro to build the samples
macro(create_measurement_sample_with_flatbuffers target_name sample_sources)
  ecal_add_sample(${target_name} ${sample_sources})  
  
  target_link_libraries(${target_name}
    PRIVATE 
      monster_snd_flatbuffers
      eCAL::flatbuffers_measurement
  )
  ecal_install_sample(${target_name})
  
  set_property(TARGET ${target_name} PROPERTY FOLDER samples/cpp/measurement/flatbuffers)
endmacro()

# Samples to statically read from / write to the measurement, this requires also running
# the flatbuffers compiler on the input file monster.fbs
create_measurement_sample_with_flatbuffers(monster_read  monster_read.cpp)
create_measurement_sample_with_flatbuffers(monster_write monster_write.cpp)