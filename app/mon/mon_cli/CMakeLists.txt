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

project(mon_cli)

find_package(Protobuf REQUIRED)
find_package(tclap REQUIRED)

set(ecalmon_cli_src
  src/ecal_mon_cli.cpp
)

if(WIN32)
  set(ecalmon_cli_win_src
    src/ecal_mon_cli.rc
  )
endif()

ecal_add_app_console(${PROJECT_NAME} ${ecalmon_cli_src} ${ecalmon_cli_win_src})

target_include_directories(${PROJECT_NAME}
  PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE $<$<BOOL:${MSVC}>:PCRE_STATIC;_UNICODE>)

create_targets_protobuf()

target_link_libraries(${PROJECT_NAME} PRIVATE
  tclap::tclap
  eCAL::protobuf_core
  eCAL::string_core
  eCAL::core_pb)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

ecal_install_app(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER app/mon)
