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

project(test_serialization)

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

add_library(generate_serialization_test_data OBJECT)
target_sources(generate_serialization_test_data
  PUBLIC
    src/common_generate.h
    src/logging_compare.h
    src/logging_generate.h
    src/monitoring_compare.h
    src/monitoring_generate.h
    src/payload_compare.h
    src/payload_generate.h
    src/registration_generate.h
    src/service_compare.h
    src/service_generate.h
  PRIVATE
    src/common_generate.cpp
    src/logging_compare.cpp
    src/logging_generate.cpp
    src/monitoring_compare.cpp
    src/monitoring_generate.cpp
    src/payload_compare.cpp
    src/payload_generate.cpp
    src/registration_generate.cpp
    src/service_compare.cpp
    src/service_generate.cpp
)

target_include_directories(generate_serialization_test_data
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    $<TARGET_PROPERTY:eCAL::core,INCLUDE_DIRECTORIES>
)

ecal_add_gtest(${PROJECT_NAME} ${unit_test_src})
target_sources(${PROJECT_NAME}
  PRIVATE
    src/logging_serialization_test.cpp
    src/monitoring_serialization_test.cpp
    src/parallel_serialization_test.cpp
    src/payload_serialization_test.cpp
    src/registration_serialization_test.cpp
    src/service_serialization_test.cpp
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Threads::Threads
    ecal_core_serialization
    generate_serialization_test_data
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

ecal_install_gtest(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER tests/cpp/core)