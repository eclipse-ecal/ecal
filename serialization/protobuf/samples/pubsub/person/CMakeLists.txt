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

project(person)

find_package(eCAL REQUIRED)
find_package(Protobuf REQUIRED)

set(person_proto
    ${CMAKE_CURRENT_SOURCE_DIR}/animal.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/house.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/person.proto
)

ecal_add_sample(person_send person_send.cpp)
PROTOBUF_TARGET_CPP(person_send ${CMAKE_CURRENT_SOURCE_DIR} ${person_proto})
target_link_libraries(person_send PRIVATE eCAL::protobuf_core)
target_compile_features(person_send PRIVATE cxx_std_14)
ecal_install_sample(person_send)
set_property(TARGET person_send PROPERTY FOLDER samples/cpp/pubsub/protobuf)

ecal_add_sample(person_receive person_receive.cpp)
PROTOBUF_TARGET_CPP(person_receive ${CMAKE_CURRENT_SOURCE_DIR} ${person_proto})
target_link_libraries(person_receive PRIVATE eCAL::protobuf_core)
target_compile_features(person_receive PRIVATE cxx_std_17)
ecal_install_sample(person_receive)
set_property(TARGET person_receive PROPERTY FOLDER samples/cpp/pubsub/protobuf)

ecal_add_sample(person_events_snd person_events_snd.cpp)
PROTOBUF_TARGET_CPP(person_events_snd ${CMAKE_CURRENT_SOURCE_DIR} ${person_proto})
target_link_libraries(person_events_snd PRIVATE eCAL::protobuf_core)
target_compile_features(person_events_snd PRIVATE cxx_std_14)
ecal_install_sample(person_events_snd)
set_property(TARGET person_events_snd PROPERTY FOLDER samples/cpp/pubsub/protobuf)

ecal_add_sample(person_events_rec person_events_rec.cpp)
PROTOBUF_TARGET_CPP(person_events_rec ${CMAKE_CURRENT_SOURCE_DIR} ${person_proto})
target_link_libraries(person_events_rec PRIVATE eCAL::protobuf_core)
target_compile_features(person_events_rec PRIVATE cxx_std_14)
ecal_install_sample(person_events_rec)
set_property(TARGET person_events_rec PROPERTY FOLDER samples/cpp/pubsub/protobuf)