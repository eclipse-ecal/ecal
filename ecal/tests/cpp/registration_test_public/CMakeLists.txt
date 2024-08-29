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

project(test_registration_public)

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

if(ECAL_CORE_PUBLISHER AND ECAL_CORE_SUBSCRIBER)
  set(registration_test_topics_src
    src/registration_getpublisherids.cpp
    src/registration_getsubscriberids.cpp
    src/registration_gettopics.cpp
  )
endif()

if(ECAL_CORE_SERVICE)
  set(registration_test_service_src
    src/registration_getclients.cpp
    src/registration_getservices.cpp
  )
endif()

set(registration_test_src
  ${registration_test_topics_src}
  ${registration_test_service_src}
)

ecal_add_gtest(${PROJECT_NAME} ${registration_test_src})

target_include_directories(${PROJECT_NAME} PRIVATE $<TARGET_PROPERTY:eCAL::core,INCLUDE_DIRECTORIES>)

target_link_libraries(${PROJECT_NAME}
  PRIVATE 
    eCAL::core
    Threads::Threads)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

ecal_install_gtest(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER tests/cpp/core)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES 
    ${${PROJECT_NAME}_src}
)
