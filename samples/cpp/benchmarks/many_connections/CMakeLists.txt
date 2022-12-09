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

cmake_minimum_required(VERSION 3.10)

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

project(many_connections)

find_package(eCAL REQUIRED)

ecal_add_sample(many_connections_snd
  many_connections_snd.cpp
)

ecal_add_sample(many_connections_rec
  many_connections_rec.cpp
)

target_link_libraries(many_connections_snd PRIVATE eCAL::core)
target_link_libraries(many_connections_rec PRIVATE eCAL::core)

target_compile_features(many_connections_snd PRIVATE cxx_std_14)
target_compile_features(many_connections_rec PRIVATE cxx_std_14)

ecal_install_sample(many_connections_snd)
ecal_install_sample(many_connections_rec)

set_property(TARGET many_connections_snd PROPERTY FOLDER samples/cpp/benchmarks)
set_property(TARGET many_connections_rec PROPERTY FOLDER samples/cpp/benchmarks)
