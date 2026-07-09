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

project(test_contrib_tracing)

find_package(GTest REQUIRED)
find_package(nlohmann_json REQUIRED)

set(tracing_test_src
  src/tracing_test_helpers.h
  src/tracing_writer_test.cpp
)

ecal_add_gtest(${PROJECT_NAME} ${tracing_test_src})

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    eCAL::tracing
    nlohmann_json::nlohmann_json
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

ecal_install_gtest(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER tests/contrib/tracing)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES
  ${tracing_test_src}
)