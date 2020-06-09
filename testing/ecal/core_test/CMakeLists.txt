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

project(test_core)

ecal_find_package(Threads REQUIRED)
ecal_find_package(GTest REQUIRED)

set(core_test_src
  src/core_test.cpp
)

ecal_add_gtest(${PROJECT_NAME}_cpp ${core_test_src})
target_include_directories(${PROJECT_NAME}_cpp PRIVATE $<TARGET_PROPERTY:eCAL::core,INCLUDE_DIRECTORIES>)
target_link_libraries(${PROJECT_NAME}_cpp
  PRIVATE eCAL::core Threads::Threads)
ecal_install_gtest(${PROJECT_NAME}_cpp)
set_property(TARGET ${PROJECT_NAME}_cpp PROPERTY FOLDER testing/ecal/core)

if(WIN32)
  ecal_add_gtest(${PROJECT_NAME}_c   ${core_test_src})
  target_include_directories(${PROJECT_NAME}_c   PRIVATE $<TARGET_PROPERTY:eCAL::core,INCLUDE_DIRECTORIES>)
  target_link_libraries(${PROJECT_NAME}_c PRIVATE eCAL::core_c Threads::Threads)
  ecal_install_gtest(${PROJECT_NAME}_c)
  set_property(TARGET ${PROJECT_NAME}_c   PROPERTY FOLDER testing/ecal/core)
endif()
