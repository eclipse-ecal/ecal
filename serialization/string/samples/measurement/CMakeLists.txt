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

project(hello_measurement)

find_package(eCAL REQUIRED)
find_package(Protobuf REQUIRED)

macro(create_measurement_sample target_name sample_sources)
  ecal_add_sample(${target_name} ${sample_sources})

  target_link_libraries(${target_name}
    PRIVATE
      eCAL::string_measurement
  )
  
  target_compile_features(${target_name} PRIVATE cxx_std_14)
  
  ecal_install_sample(${target_name})
  
  set_property(TARGET ${target_name} PROPERTY FOLDER samples/cpp/measurement/string)
endmacro()

create_measurement_sample(hello_read   hello_read.cpp)
create_measurement_sample(hello_write  hello_write.cpp)


