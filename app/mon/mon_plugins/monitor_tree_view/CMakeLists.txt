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

project(MonitorTreeView)

ecal_find_package(Qt5 
  COMPONENTS Core Widgets
  REQUIRED
)

set(source_files
  src/item_data_roles.h
  src/monitor_tree_model.cpp
  src/monitor_tree_model.h
  src/tree_item_type.h
  src/monitor_tree_item.cpp
  src/monitor_tree_item.h
  src/util.h
) 

set(CMAKE_AUTOMOC ON)

add_library(${PROJECT_NAME} ${source_files})
add_library(eCAL::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC src)

target_link_libraries(${PROJECT_NAME}
  Qt5::Widgets CustomQt
)

if(MSVC)
  msvc_source_tree(
   ${source_files}
  )
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER app/mon/plugins)
