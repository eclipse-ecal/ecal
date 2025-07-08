# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2024 Continental Corporation
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

find_package(eCAL REQUIRED)
find_package(GTest REQUIRED)


ecal_add_sample(fast_init_loop fast_init_loop.cpp)

target_link_libraries(fast_init_loop PRIVATE
  eCAL::core
  GTest::gtest
  GTest::gtest_main
)

target_compile_features(fast_init_loop PRIVATE cxx_std_14)

ecal_install_sample(fast_init_loop)

set_property(TARGET fast_init_loop PROPERTY FOLDER samples/cpp/benchmarks/fast_init_loop)
