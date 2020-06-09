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

project(test_pubsub)

ecal_find_package(Threads REQUIRED)
ecal_find_package(GTest REQUIRED)

set(pubsub_test_src
  src/pubsub_test.cpp
)

ecal_add_gtest(${PROJECT_NAME}_cpp ${pubsub_test_src})
target_link_libraries(${PROJECT_NAME}_cpp
  PRIVATE
    eCAL::core
    Threads::Threads)
ecal_install_gtest(${PROJECT_NAME}_cpp)
set_property(TARGET ${PROJECT_NAME}_cpp PROPERTY FOLDER testing/ecal/pubsub)

if(WIN32)
  ecal_add_gtest(${PROJECT_NAME}_c   ${pubsub_test_src})
  target_link_libraries(${PROJECT_NAME}_c
    PRIVATE
      eCAL::core_c
      Threads::Threads)
  ecal_install_gtest(${PROJECT_NAME}_c)
  set_property(TARGET ${PROJECT_NAME}_c PROPERTY FOLDER testing/ecal/pubsub)
endif()
