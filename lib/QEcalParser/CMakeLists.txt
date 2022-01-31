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

project(QEcalParser)

find_package(Qt5 COMPONENTS
    Core
    Widgets
REQUIRED)


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF) # Reason for being turned off: AutoUIC will prevent VS from detecting changes in .ui files
set(CMAKE_AUTORCC OFF) # Reason for being turned off: AutoRCC will create an entirely new project in VS which clutters the solution appearance. Additionally, we cannot assign a source group to the generated .cpp files which will clutter the project.
set(CMAKE_INCLUDE_CURRENT_DIR ON)


set (qecalparser_includes
    include/QEcalParser/ecal_parser_editor/ecal_parser_editor_dialog.h
    include/QEcalParser/ecal_parser_editor/ecal_parser_editor_widget.h
    include/QEcalParser/ecal_parser_lineedit/ecal_parser_lineedit.h
    include/QEcalParser/ecal_parser_textedit/ecal_parser_textedit.h
)

set(qecalparser_src
    src/ecal_parser_editor/ecal_parser_editor_dialog.cpp
    src/ecal_parser_editor/ecal_parser_editor_widget.cpp
    src/ecal_parser_editor/function_list_model.cpp
    src/ecal_parser_editor/function_list_model.h
    src/ecal_parser_lineedit/ecal_parser_lineedit.cpp
    src/ecal_parser_textedit/ecal_parser_textedit.cpp
)

set(qecalparser_ui
    src/ecal_parser_editor/ecal_parser_editor_dialog.ui
    src/ecal_parser_editor/ecal_parser_editor_widget.ui
)

set(qecalparser_resources
    resources/qecalparser.qrc
)

# compile qt resource files and ui files
qt5_add_resources(autogen_resources ${qecalparser_resources})
qt5_wrap_ui      (autogen_ui        ${qecalparser_ui})

add_library (${PROJECT_NAME}
    ${qecalparser_includes}
    ${qecalparser_src}
    ${qecalparser_ui}
    ${qecalparser_resources}

    ${autogen_resources}
    ${autogen_ui}
)

target_include_directories(${PROJECT_NAME} 
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
  PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    Qt5::Core
    Qt5::Widgets
  PRIVATE
    CustomQt
    EcalParser
)

if(MSVC)
    # Disable Compiler warning "Conditional expression is constant"
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/wd4127 /wd4714")
endif()

# Create a source tree that mirrors the filesystem
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}"
    FILES
        ${qecalparser_includes}
        ${qecalparser_src}
        ${qecalparser_ui}
        ${qecalparser_resources}
)

source_group(autogen FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_automoc.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_autogen/mocs_compilation.cpp
    ${autogen_resources}
    ${autogen_ui}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER lib)
