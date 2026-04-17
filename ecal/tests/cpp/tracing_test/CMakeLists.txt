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

project(test_tracing)

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
find_package(nlohmann_json REQUIRED)

set(tracing_test_src
  src/tracing_test_helpers.h
  # Unit tests — individual classes in isolation
  src/unittest/tracing_types_test.cpp
  src/unittest/tracing_layer_type_test.cpp
  src/unittest/tracing_provider_test.cpp
  src/unittest/tracing_span_test.cpp
  src/unittest/tracing_writer_test.cpp
  src/unittest/tracing_edge_case_test.cpp
  # Integration tests — component interactions and concurrency
  src/integrationtest/tracing_integration_test.cpp
  src/integrationtest/tracing_thread_safety_test.cpp
  src/integrationtest/tracing_scale_test.cpp
  # System tests — full eCAL runtime, real pub/sub
  src/systemtest/tracing_pubsub_stress_test.cpp
)

ecal_add_gtest(${PROJECT_NAME} ${tracing_test_src})

target_include_directories(${PROJECT_NAME} PRIVATE 
  $<TARGET_PROPERTY:eCAL::core,INCLUDE_DIRECTORIES>
  ${CMAKE_CURRENT_LIST_DIR}/src
  ${ECAL_CORE_PROJECT_ROOT}/core/src
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE 
    ecal_core_private
    Threads::Threads
    nlohmann_json::nlohmann_json
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_compile_definitions(${PROJECT_NAME} PRIVATE ECAL_CORE_COMMAND_LINE)

ecal_install_gtest(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER tests/cpp/tracing)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES 
    ${tracing_test_src}
)
