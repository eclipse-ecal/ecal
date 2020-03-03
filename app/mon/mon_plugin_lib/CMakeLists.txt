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

project(mon_plugin_lib)

set(ecalmonpluginlib_src
  src/dummy.cpp
)

set(ecalmonpluginlib_header
  include/ecal/mon/plugin_interface.h
  include/ecal/mon/plugin_widget_interface.h
  include/ecal/mon/plugin.h
)

ecal_add_library(${PROJECT_NAME} ${ecalmonpluginlib_src} ${ecalmonpluginlib_header})
add_library(eCAL::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ecal_install_library(${PROJECT_NAME})
install(DIRECTORY
  "include/" DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT sdk
   FILES_MATCHING PATTERN "*.h")

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER app/mon)