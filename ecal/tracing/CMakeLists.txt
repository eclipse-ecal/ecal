# ========================= eCAL LICENSE =================================
#
# Copyright 2026 AUMOVIO and subsidiaries. All rights reserved.
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

project(tracing)

find_package(nlohmann_json REQUIRED)

add_library(${PROJECT_NAME})
add_library(eCAL::tracing ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
  PUBLIC
    include/ecal/tracing/types.h
    include/ecal/tracing/writer.h
    include/ecal/tracing/writer_jsonl.h
  PRIVATE
    src/writer_jsonl.cpp
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    nlohmann_json::nlohmann_json
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ecal/tracing)

if(ECAL_BUILD_TESTS)
  add_subdirectory(tests)
endif()