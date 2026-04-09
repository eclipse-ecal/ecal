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

project(test_pubsub_proto)

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
find_package(Protobuf REQUIRED)

create_targets_protobuf()

set(${PROJECT_NAME}_src
  src/proto_dyn_subscriber_test.cpp
  src/proto_publisher_test.cpp
  src/proto_subscriber_test.cpp
)

set(pubsub_proto_test_proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/protobuf/animal.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/protobuf/house.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/protobuf/person.proto
)

ecal_add_gtest(${PROJECT_NAME} ${${PROJECT_NAME}_src})

PROTOBUF_TARGET_CPP(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/protobuf ${pubsub_proto_test_proto})

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    eCAL::protobuf_core
    Threads::Threads
)


target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

ecal_install_gtest(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER tests/cpp/pubsub)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES 
    ${${PROJECT_NAME}_src}
)
