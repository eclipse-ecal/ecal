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

project(test_config)

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
find_package(tclap REQUIRED)
find_package(yaml-cpp REQUIRED)

set(cmd_parser_src
  ${ECAL_CORE_PROJECT_ROOT}/core/src/config/ecal_cmd_parser.cpp
  ${ECAL_CORE_PROJECT_ROOT}/core/src/util/advanced_tclap_output.cpp
  ${ECAL_PROJECT_ROOT}/lib/ecal_utils/src/filesystem.cpp
  ${ECAL_PROJECT_ROOT}/lib/ecal_utils/src/str_convert.cpp 
)

set(config_reader_src
  ${CMAKE_SOURCE_DIR}/ecal/core/src/config/configuration_reader.h
)

set(config_test_src
  src/config_test.cpp
  ${cmd_parser_src}
  ${config_reader_src}
)

ecal_add_gtest(${PROJECT_NAME} ${config_test_src})

target_include_directories(${PROJECT_NAME} PRIVATE 
  $<TARGET_PROPERTY:eCAL::core,INCLUDE_DIRECTORIES>
  ${CMAKE_CURRENT_LIST_DIR}/src
  ${ECAL_CORE_PROJECT_ROOT}/core/src/config
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE 
    eCAL::core 
    Threads::Threads
    tclap::tclap
    yaml-cpp::yaml-cpp
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

target_compile_definitions(${PROJECT_NAME} PRIVATE ECAL_CORE_COMMAND_LINE)

ecal_install_gtest(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER tests/cpp/config)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES 
    ${${PROJECT_NAME}_src}
)
