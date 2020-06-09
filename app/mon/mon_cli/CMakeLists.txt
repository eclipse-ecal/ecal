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

project(mon)

ecal_find_package(Protobuf REQUIRED)
ecal_find_package(tclap REQUIRED)

set(ecalmon_src
  src/ecal_mon.cpp
)

if(WIN32)
  set(ecalmon_win_src
    src/ecal_mon.rc
  )
endif()

ecal_add_app_console(${PROJECT_NAME} ${ecalmon_src} ${ecalmon_win_src})

target_include_directories(${PROJECT_NAME}
  PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE $<$<BOOL:${MSVC}>:PCRE_STATIC;_UNICODE>)

create_targets_protobuf()

target_link_libraries(${PROJECT_NAME}
  protobuf::libprotobuf
  tclap::tclap
  eCAL::core
  eCAL::pb)

ecal_install_app(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER app/mon)
