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
find_package(CapnProto REQUIRED)

macro(create_measurement_sample target_name sample_sources)
  ecal_add_sample(${target_name} ${sample_sources})  
  
  target_link_libraries(${target_name} PRIVATE eCAL::capnproto_measurement)
  target_link_options(${target_name} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/ignore:4099>)
  
  target_compile_features(${target_name} PRIVATE cxx_std_17)
  
  ecal_install_sample(${target_name})
  
  set_property(TARGET ${target_name} PROPERTY FOLDER samples/cpp/measurement/capnproto)
endmacro()

macro(create_measurement_sample_with_capnp target_name sample_sources)
  create_measurement_sample(${target_name} ${sample_sources})

  set(CAPNPC_IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR})  
  capnp_generate_cpp(CAPNP_SRCS CAPNP_HDRS addressbook.capnp)
  set_source_files_properties(${CAPNP_SRCS} PROPERTIES COMPILE_FLAGS $<$<CXX_COMPILER_ID:MSVC>:-W0>)
  target_sources(${target_name} PRIVATE ${CAPNP_SRCS} ${CAPNP_HDRS})  
  target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
endmacro()

# Samples to statically read from / write to the measurement, this requires also running
# the capnproto compiler on the input file addressbook.capnp
create_measurement_sample_with_capnp(addressbook_read   addressbook_read.cpp)
create_measurement_sample_with_capnp(addressbook_write  addressbook_write.cpp)

# Samples to dynamically read information from the measurement
create_measurement_sample(addressbook_dyn_read      addressbook_dyn_read.cpp)