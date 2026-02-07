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

project(ecal_config)

set(ecalconfig_src
  src/ecal_config.cpp
)

ecal_add_app_console(${PROJECT_NAME} ${ecalconfig_src})

target_include_directories(${PROJECT_NAME}
  PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE $<$<BOOL:${MSVC}>:PCRE_STATIC;_UNICODE>)

target_link_libraries(${PROJECT_NAME} PRIVATE eCAL::core)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

ecal_install_app(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER app/util)
