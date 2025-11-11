# ========================= eCAL LICENSE =================================
#
# Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
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

project(ecal_benchmark_internal)

add_executable(ecal_benchmark_descgate
 benchmark_descgate.cpp
 ${ECAL_CORE_PROJECT_ROOT}/core/src/ecal_descgate.cpp
)

target_include_directories(ecal_benchmark_descgate
  PRIVATE
    $<TARGET_PROPERTY:eCAL::core,INCLUDE_DIRECTORIES>
)

target_link_libraries(ecal_benchmark_descgate
  PRIVATE
    benchmark::benchmark
)

target_compile_features(ecal_benchmark_descgate PRIVATE cxx_std_14)

add_executable(ecal_benchmark_serialization
  benchmark_serialization.cpp
)

target_link_libraries(ecal_benchmark_serialization
  PRIVATE
    benchmark::benchmark
    ecal_core_serialization
    generate_serialization_test_data
)

target_compile_features(ecal_benchmark_serialization PRIVATE cxx_std_14)
