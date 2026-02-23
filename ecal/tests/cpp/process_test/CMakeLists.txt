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

project(test_process)

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

ecal_add_gtest(test_process
  src/process_test.cpp
)

target_link_libraries(test_process
  PRIVATE
    eCAL::core
    eCAL::ecal-utils
    Threads::Threads)

target_compile_features(test_process PRIVATE cxx_std_14)

target_compile_definitions(test_process PRIVATE
  ECAL_PROCESS_TEST_EXECUTABLE_NAME="$<TARGET_FILE:test_process>"
)

ecal_install_gtest(test_process)

set_property(TARGET test_process PROPERTY FOLDER tests/cpp/core/public)
