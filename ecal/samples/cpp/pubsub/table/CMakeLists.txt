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

project(table)

find_package(eCAL REQUIRED)

if(ECAL_CORE_PUBLISHER)
	ecal_add_sample(table_send table_send.cpp)
	target_compile_features(table_send PRIVATE cxx_std_14)
	target_link_libraries(table_send PRIVATE eCAL::core)
	ecal_install_sample(table_send)
	set_property(TARGET table_send PROPERTY FOLDER samples/cpp/pubsub/table)
endif()

if(ECAL_CORE_SUBSCRIBER)
	ecal_add_sample(table_receive table_receive.cpp)
	target_compile_features(table_receive PRIVATE cxx_std_14)
	target_link_libraries(table_receive PRIVATE eCAL::core)
	ecal_install_sample(table_receive)
	set_property(TARGET table_receive PROPERTY FOLDER samples/cpp/pubsub/table)
endif()