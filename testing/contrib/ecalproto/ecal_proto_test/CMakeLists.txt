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

project(test_ecal_proto)

ecal_find_package(Threads REQUIRED)
ecal_find_package(GTest REQUIRED)
ecal_find_package(Protobuf REQUIRED)

create_targets_protobuf()

set(ecal_proto_test_src
  src/test_filters.cpp
)

set(${PROJECT_NAME}_proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/protobuf/animal.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/protobuf/house.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/protobuf/person.proto
)

ecal_add_gtest(${PROJECT_NAME} ${ecal_proto_test_src})
PROTOBUF_TARGET_CPP(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/protobuf ${${PROJECT_NAME}_proto})

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Threads::Threads
    eCAL::proto
    protobuf::libprotobuf)

ecal_install_gtest(${PROJECT_NAME} ${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER testing/contrib/ecalproto)
