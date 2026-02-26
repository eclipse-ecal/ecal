# ========================= eCAL LICENSE =================================
#
# 
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

find_package(GTest REQUIRED)

add_library(test-dynamic-library SHARED)
target_sources(test-dynamic-library 
 PRIVATE
   dynamic_library.h
   dynamic_library.cpp
)
target_include_directories(test-dynamic-library PRIVATE .)

set(ecal_utils_test_src
  test_dynamic_library.cpp
)  

ecal_add_gtest(ecal-utils-test ${ecal_utils_test_src})

target_link_libraries(ecal-utils-test
  PRIVATE
    ecal-utils
    Threads::Threads)

target_compile_definitions(ecal-utils-test
  PRIVATE
    TEST_DYNLIB_PATH="$<TARGET_FILE:test-dynamic-library>"
)

ecal_install_gtest(ecal-utils-test)

set_property(TARGET ecal-utils-test PROPERTY FOLDER tests/cpp/lib/ecal-utils)
set_property(TARGET test-dynamic-library PROPERTY FOLDER tests/cpp/lib/ecal-utils)
