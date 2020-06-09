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

project(ecaltime-simtime)

ecal_find_package(Protobuf REQUIRED)

set(ecal_time_simtime_src
  src/ecal_time_simtime.cpp
  src/ecaltime.cpp
) 

set(ecal_time_simtime_header
  ../include/dynamic_sleeper.h
  ../include/ecaltime.h
  src/ecal_time_simtime.h
)

include_directories(${PROJECT_NAME}
  ../include
)

if(WIN32)
  set(ecal_time_simtime_dll_src
    src/dllmain.cpp
  )
endif()

ecal_add_time_plugin(${PROJECT_NAME} SHARED ${ecal_time_simtime_src} ${ecal_time_simtime_header} ${ecal_time_simtime_dll_src})

target_compile_definitions(${PROJECT_NAME} PRIVATE ECAL_TIME_PLUGIN_API_EXPORT)

target_link_libraries(${PROJECT_NAME} PRIVATE eCAL::core)
target_link_libraries(${PROJECT_NAME} PRIVATE eCAL::pb)

target_include_directories(${PROJECT_NAME} PRIVATE .)
target_include_directories(${PROJECT_NAME} PRIVATE  ../include)

ecal_install_time_plugin(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER contrib/ecaltime)
set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>/${ECAL_TIME_PLUGIN_DIR})
