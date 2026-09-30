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

cmake_minimum_required(VERSION 3.15)

project(blob)

find_package(eCAL REQUIRED)

if(ECAL_CORE_PUBLISHER)
	ecal_add_sample(blob_send blob_send.cpp)
	target_link_libraries(blob_send PRIVATE eCAL::core)
	target_compile_features(blob_send PRIVATE cxx_std_14)
	ecal_install_sample(blob_send)
	set_property(TARGET blob_send PROPERTY FOLDER samples/cpp/pubsub/blob)
endif()

if(ECAL_CORE_SUBSCRIBER)
	ecal_add_sample(blob_receive blob_receive.cpp)
	target_link_libraries(blob_receive PRIVATE eCAL::core)
	target_compile_features(blob_receive PRIVATE cxx_std_14)
	ecal_install_sample(blob_receive)
	set_property(TARGET blob_receive PROPERTY FOLDER samples/cpp/pubsub/blob)
endif()