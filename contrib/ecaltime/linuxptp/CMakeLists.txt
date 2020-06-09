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

project(ecaltime-linuxptp)

ecal_find_package(simpleini REQUIRED)

set(ecal_time_linuxptp_src
  src/ecal_time_linuxptp.cpp
  src/ecaltime.cpp
  src/convert_utf.cpp
) 

set(ecal_time_linuxptp_header
  ../include/ecaltime.h
  src/ecal_time_linuxptp.h
  src/config/config.h
)

if(UNIX)
  set_source_files_properties(convert_utf.cpp PROPERTIES COMPILE_FLAGS -Wno-implicit-fallthrough)
endif()

ecal_add_time_plugin(${PROJECT_NAME} SHARED ${ecal_time_linuxptp_src} ${ecal_time_linuxptp_header})

target_link_libraries(${PROJECT_NAME} eCAL::core simpleini::simpleini)

target_include_directories(${PROJECT_NAME} 
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)

ecal_install_time_plugin(${PROJECT_NAME})

set(ECALTIME_CONFIG_FILES
  ${ECALTIME_CONFIG_FILES}
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config/ecaltime.ini
  PARENT_SCOPE)

set_target_properties(${PROJECT_NAME} PROPERTIES
  FOLDER contrib/ecaltime
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>/${ECAL_TIME_PLUGIN_DIR})
