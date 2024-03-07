# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2019 Continental Corporation
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

project(test_measurement)

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

ecal_add_gtest(${PROJECT_NAME} reader_test.cpp)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    eCAL::measurement_hdf5
    Threads::Threads)

target_include_directories(${PROJECT_NAME} PRIVATE $<TARGET_PROPERTY:eCAL::measurement_hdf5,INCLUDE_DIRECTORIES>)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

ecal_install_gtest(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER testing/contrib/ecalhdf5)
