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

cmake_minimum_required(VERSION 3.15)

project(mirror)

find_package(eCAL REQUIRED)

ecal_add_sample(mirror_client_cpp mirror_client.cpp)
target_link_libraries(mirror_client_cpp PRIVATE eCAL::core)
target_compile_features(mirror_client_cpp PRIVATE cxx_std_14)
ecal_install_sample(mirror_client_cpp)
set_property(TARGET mirror_client_cpp PROPERTY FOLDER samples/cpp/services/binary)

# The target name mirror_server_cpp is used by HDF5.
ecal_add_sample(mirror_server_cpp mirror_server.cpp)
target_link_libraries(mirror_server_cpp PRIVATE eCAL::core)
target_compile_features(mirror_server_cpp PRIVATE cxx_std_14)
ecal_install_sample(mirror_server_cpp)
set_property(TARGET mirror_server_cpp PROPERTY FOLDER samples/cpp/services/binary)