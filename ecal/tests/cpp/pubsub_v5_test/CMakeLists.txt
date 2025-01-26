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

project(test_pubsub_v5)

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

if(ECAL_CORE_TRANSPORT_SHM)
  set(pubsub_test_src_shm
    src/pubsub_connection_test.cpp
  )
endif()

set(pubsub_test_src
  src/pubsub_callback_topicid.cpp
  src/pubsub_receive_test.cpp
  ${pubsub_test_src_shm}
)

ecal_add_gtest(${PROJECT_NAME} ${pubsub_test_src})

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    eCAL::core
    Threads::Threads)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

ecal_install_gtest(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER tests/cpp/pubsub)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES 
    ${${PROJECT_NAME}_src}
)
