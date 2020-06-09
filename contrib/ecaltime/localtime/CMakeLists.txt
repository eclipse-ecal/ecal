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

project(ecaltime-localtime)

ecal_find_package(Threads REQUIRED)

set(ecal_time_localtime_src
  src/ecaltime.cpp
) 

if(WIN32)
  set(ecal_time_localtime_dll_src
    src/dllmain.cpp
  )
endif()

set(ecal_time_localtime_header
  ../include/ecaltime.h
)

ecal_add_time_plugin(${PROJECT_NAME} SHARED ${ecal_time_localtime_src} ${ecal_time_localtime_header} ${ecal_time_localtime_dll_src})

target_compile_definitions(${PROJECT_NAME} PRIVATE ECAL_TIME_PLUGIN_API_EXPORT)

target_include_directories(${PROJECT_NAME} 
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)

target_link_libraries(${PROJECT_NAME}
  Threads::Threads
  $<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>>:rt>
  )

ecal_install_time_plugin(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
  FOLDER contrib/ecaltime
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>/${ECAL_TIME_PLUGIN_DIR})
