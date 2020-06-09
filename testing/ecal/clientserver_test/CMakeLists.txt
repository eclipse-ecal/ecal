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

project(test_clientserver)

ecal_find_package(Threads REQUIRED)
ecal_find_package(GTest REQUIRED)
ecal_find_package(Protobuf REQUIRED)

create_targets_protobuf()

set(clientserver_test_src
  src/clientserver_test.cpp
)

set(clientserver_test_proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/protobuf/math.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/protobuf/ping.proto
)

ecal_add_static_library(clientserver_proto src/dummy.cpp)
PROTOBUF_TARGET_CPP(clientserver_proto ${CMAKE_CURRENT_SOURCE_DIR}/src/protobuf ${clientserver_test_proto})
target_link_libraries(clientserver_proto PRIVATE protobuf::libprotobuf)
set_property(TARGET clientserver_proto PROPERTY FOLDER testing/ecal/service)

ecal_add_gtest(${PROJECT_NAME}_cpp ${clientserver_test_src})
target_link_libraries(${PROJECT_NAME}_cpp
  PRIVATE eCAL::core clientserver_proto Threads::Threads)

  ecal_install_gtest(${PROJECT_NAME}_cpp)
set_property(TARGET ${PROJECT_NAME}_cpp PROPERTY FOLDER testing/ecal/service)

if(WIN32)
  ecal_add_gtest(${PROJECT_NAME}_c   ${clientserver_test_src})
  target_link_libraries(${PROJECT_NAME}_c PRIVATE
    eCAL::core_c clientserver_proto Threads::Threads)
  ecal_install_gtest(${PROJECT_NAME}_c)
  set_property(TARGET ${PROJECT_NAME}_c   PROPERTY FOLDER testing/ecal/service)
endif()
