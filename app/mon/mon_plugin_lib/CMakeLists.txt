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

cmake_minimum_required(VERSION 3.14)

project(mon_plugin_lib)

# Legacy Qt5 (pre 5.15) support as suggested by teh Qt Documentation:
# https://doc.qt.io/qt-6/cmake-qt5-and-qt6-compatibility.html#supporting-older-qt-5-versions
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

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

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14) 

ecal_install_library(${PROJECT_NAME})
install(DIRECTORY
  "include/" DESTINATION "${eCAL_install_include_dir}" COMPONENT sdk
   FILES_MATCHING PATTERN "*.h")

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER app/mon)
