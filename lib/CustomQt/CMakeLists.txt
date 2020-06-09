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

cmake_minimum_required(VERSION 3.0.2)

project(CustomQt)

ecal_find_package(Qt5 COMPONENTS
    Core
    Widgets
REQUIRED)

if(POLICY CMP0071)
  cmake_policy(SET CMP0071 OLD)
endif()


set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set (custom_qt_includes
  include/CustomQt/QAbstractTreeItem.h
  include/CustomQt/QAbstractTreeModel.h
  include/CustomQt/QAdvancedCheckBox.h
  include/CustomQt/QAdvancedTreeView.h
  include/CustomQt/QCheckboxHeaderView.h
  include/CustomQt/QClickableStatusBar.h
  include/CustomQt/QCustomFileSystemModel.h
  include/CustomQt/QFilterLineEdit.h
  include/CustomQt/QListMenuToolButton.h
  include/CustomQt/QMulticolumnSortFilterProxyModel.h
  include/CustomQt/QPathLineEdit.h
  include/CustomQt/QSizeHintFrame.h
  include/CustomQt/QStableSortFilterProxyModel.h
  include/CustomQt/QStandardTreeItem.h
  include/CustomQt/QStandardTreeModel.h
  include/CustomQt/QToolTipMenu.h
)

set(custom_qt_src
  src/QAbstractTreeItem.cpp
  src/QAbstractTreeModel.cpp
  src/QAdvancedTreeView.cpp
  src/QCheckboxHeaderView.cpp
  src/QClickableStatusBar.cpp
  src/QCustomFileSystemModel.cpp
  src/QFilterLineEdit.cpp
  src/QListMenuToolButton.cpp
  src/QMulticolumnSortFilterProxyModel.cpp
  src/QPathLineEdit.cpp
  src/QSizeHintFrame.cpp
  src/QStableSortFilterProxyModel.cpp
  src/QStandardTreeItem.cpp
  src/QStandardTreeModel.cpp
  src/QToolTipMenu.cpp
)

add_library (${PROJECT_NAME}
  ${custom_qt_includes}
  ${custom_qt_src}
)

target_include_directories(${PROJECT_NAME} 
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
  PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
)

if(MSVC)
  # Create a source tree that mirrors the filesystem
  msvc_source_tree(${custom_qt_includes} ${custom_qt_src})

  # Disable Compiler warning "Conditional expression is constant"
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/wd4127")

  source_group(autogen FILES 
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_automoc.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_autogen/mocs_compilation.cpp
  )
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER lib)
