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

find_package(Qt5 COMPONENTS
    Core
    Widgets
REQUIRED)

find_package(Protobuf REQUIRED)
find_package(simpleini REQUIRED)

if(POLICY CMP0071)
  cmake_policy(SET CMP0071 OLD)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF) # Reason for being turned off: AutoUIC will prevent VS from detecting changes in .ui files
set(CMAKE_AUTORCC OFF) # Reason for being turned off: AutoRCC will create an entirely new project in VS which clutters the solution appearance. Additionally, we cannot assign a source group to the generated .cpp files which will clutter the project.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(source_files
    src/convert_utf.cpp
    src/convert_utf.h
    src/Ecalmon.cpp
    src/Ecalmon.h
    src/EcalmonGlobals.h
    src/main.cpp
    src/Util.h
    
    src/PluginLoader.cpp
    src/PluginLoader.h
    
    src/Widgets/AboutDialog/AboutDialog.cpp
    src/Widgets/AboutDialog/AboutDialog.h

    src/Widgets/ColumnSelectorDialog/ColumnPickerDelegate.cpp
    src/Widgets/ColumnSelectorDialog/ColumnPickerDelegate.h
    src/Widgets/ColumnSelectorDialog/ColumnSelectorDialog.cpp
    src/Widgets/ColumnSelectorDialog/ColumnSelectorDialog.h

    src/Widgets/EcalmonTreeWidget/EcalmonTreeWidget.cpp
    src/Widgets/EcalmonTreeWidget/EcalmonTreeWidget.h
    src/Widgets/EcalmonTreeWidget/HostWidget.cpp
    src/Widgets/EcalmonTreeWidget/HostWidget.h
    src/Widgets/EcalmonTreeWidget/ProcessWidget.cpp
    src/Widgets/EcalmonTreeWidget/ProcessWidget.h
    src/Widgets/EcalmonTreeWidget/ServiceWidget.cpp
    src/Widgets/EcalmonTreeWidget/ServiceWidget.h
    src/Widgets/EcalmonTreeWidget/TopicWidget.cpp
    src/Widgets/EcalmonTreeWidget/TopicWidget.h

    src/Widgets/LicenseDialog/LicenseDialog.cpp
    src/Widgets/LicenseDialog/LicenseDialog.h

    src/Widgets/LogWidget/LogWidget.cpp
    src/Widgets/LogWidget/LogWidget.h

    src/Widgets/Models/ColumnTreeItem.cpp
    src/Widgets/Models/ColumnTreeItem.h
    src/Widgets/Models/ColumnTreeModel.cpp
    src/Widgets/Models/ColumnTreeModel.h
    src/Widgets/Models/GroupTreeItem.cpp
    src/Widgets/Models/GroupTreeItem.h
    src/Widgets/Models/GroupTreeModel.cpp
    src/Widgets/Models/GroupTreeModel.h
    src/Widgets/Models/HostTreeItem.cpp
    src/Widgets/Models/HostTreeItem.h
    src/Widgets/Models/HostTreeModel.cpp
    src/Widgets/Models/HostTreeModel.h
    src/Widgets/Models/ItemDataRoles.h
    src/Widgets/Models/ItemViewModel.cpp
    src/Widgets/Models/ItemViewModel.h
    src/Widgets/Models/LogModel.cpp
    src/Widgets/Models/LogModel.h
    src/Widgets/Models/LogSortFilterProxyModel.cpp
    src/Widgets/Models/LogSortFilterProxyModel.h
    src/Widgets/Models/PluginTableModel.cpp
    src/Widgets/Models/PluginTableModel.h
    src/Widgets/Models/ProcessTreeItem.cpp
    src/Widgets/Models/ProcessTreeItem.h
    src/Widgets/Models/ProcessTreeModel.cpp
    src/Widgets/Models/ProcessTreeModel.h
    src/Widgets/Models/ProtobufTreeItem.cpp
    src/Widgets/Models/ProtobufTreeItem.h
    src/Widgets/Models/ProtobufTreeModel.cpp
    src/Widgets/Models/ProtobufTreeModel.h
    src/Widgets/Models/ServiceTreeItem.cpp
    src/Widgets/Models/ServiceTreeItem.h
    src/Widgets/Models/ServiceTreeModel.cpp
    src/Widgets/Models/ServiceTreeModel.h
    src/Widgets/Models/TopicSortFilterProxyModel.cpp
    src/Widgets/Models/TopicSortFilterProxyModel.h
    src/Widgets/Models/TopicTreeItem.cpp
    src/Widgets/Models/TopicTreeItem.h
    src/Widgets/Models/TopicTreeModel.cpp
    src/Widgets/Models/TopicTreeModel.h
    src/Widgets/Models/TreeItemType.h
    
    src/Widgets/PluginSettingsDialog/PluginSettingsDialog.cpp
    src/Widgets/PluginSettingsDialog/PluginSettingsDialog.h

    src/Widgets/VisualisationWidget/VisualisationWidget.cpp
    src/Widgets/VisualisationWidget/VisualisationWidget.h
    src/Widgets/VisualisationWidget/VisualisationWindow.cpp
    src/Widgets/VisualisationWidget/VisualisationWindow.h
    
    src/Widgets/SystemInformationWidget/SystemInformationWidget.cpp
    src/Widgets/SystemInformationWidget/SystemInformationWidget.h

    src/Widgets/TreeItemViewWidget/TreeItemViewWidget.cpp
    src/Widgets/TreeItemViewWidget/TreeItemViewWidget.h
)

set(qt_resource_files
    ../../iconset/ecalicons.qrc
    resources/resources.qrc
)
if(WIN32)
    set(win32_resource_files
        resources/resource.h
        resources/win32_resources.rc
    )
ENDIF(WIN32)

set(ui_files
    src/MainWindow.ui
    src/Widgets/AboutDialog/AboutDialog.ui
    src/Widgets/ColumnSelectorDialog/ColumnSelectorDialog.ui
    src/Widgets/EcalmonTreeWidget/EcalmonTreeWidget.ui
    src/Widgets/LicenseDialog/LicenseDialog.ui
    src/Widgets/LogWidget/LogWidget.ui
    src/Widgets/PluginSettingsDialog/PluginSettingsDialog.ui
    src/Widgets/SystemInformationWidget/SystemInformationWidget.ui
    src/Widgets/TreeItemViewWidget/TreeItemViewWidget.ui
    src/Widgets/VisualisationWidget/VisualisationWidget.ui
)

if(ECAL_NPCAP_SUPPORT)
    set(source_files
        ${source_files}
        src/Widgets/NpcapStatusDialog/NpcapStatusDialog.cpp
        src/Widgets/NpcapStatusDialog/NpcapStatusDialog.h
    )
    set(ui_files
        ${ui_files}
        src/Widgets/NpcapStatusDialog/NpcapStatusDialog.ui
    )
endif(ECAL_NPCAP_SUPPORT)

# compile qt resource files and ui files
qt5_add_resources(autogen_resources ${qt_resource_files})
qt5_wrap_ui      (autogen_ui        ${ui_files})

# Add all files. The resource files and ui files are not necessary, but we want them to show up in the IDE
ecal_add_app_qt(${PROJECT_NAME} 
    ${source_files}
    ${qt_resource_files}
    ${win32_resource_files}
    ${ui_files}

    ${autogen_resources}
    ${autogen_ui}
)

create_targets_protobuf()
target_link_libraries (${PROJECT_NAME}
    protobuf::libprotobuf
    simpleini::simpleini
    eCAL::core
    eCAL::pb
    eCAL::mon_plugin_lib
    Qt5::Widgets
    CustomQt
)

if(ECAL_NPCAP_SUPPORT)
  add_definitions(-DECAL_NPCAP_SUPPORT)
  target_link_libraries(${PROJECT_NAME}
    udpcap
  )
endif(ECAL_NPCAP_SUPPORT)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()
if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/wd4127")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_include_directories(${PROJECT_NAME} PRIVATE $<TARGET_PROPERTY:eCAL::core,INCLUDE_DIRECTORIES>)


if(WIN32)
    # Deploy Qt DLLs in the binary folder. This is necessary for starting the application from whithin the IDE without having to copy QtCore.dll, QtWidgets.dll etc. by hand each time
    qt_add_windeployqt_postbuild(--no-system-d3d-compiler --no-compiler-runtime --no-opengl-sw --pdb "$<TARGET_FILE:${PROJECT_NAME}>")

    # Create a source tree that mirrors the filesystem
    msvc_source_tree(
        ${source_files}
        ${qt_resource_files}
        ${win32_resource_files}
        ${ui_files}
    )

    # Also create a group for autogenerated files. The autogenerated ui files are not necessary as they are only header files. We add them anyhow, just for completeness.
    source_group( autogen FILES 
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_automoc.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_autogen/mocs_compilation.cpp
        ${autogen_ui}
        ${autogen_resources}
    )
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER app/mon)

ecal_install_app(${PROJECT_NAME})

#file(GENERATE OUTPUT "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}_path"
#CONTENT "$<TARGET_FILE:${PROJECT_NAME}>"
#)
get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

# Running this with MSVC 2015 requires CMake 3.6+
if((MSVC_VERSION VERSION_EQUAL 1900 OR MSVC_VERSION VERSION_GREATER 1900)
        AND CMAKE_VERSION VERSION_LESS "3.6")
    message(WARNING "Deploying with MSVC 2015+ requires CMake 3.6+")
endif()

install(CODE
    "
    set(_file ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/ecal_mon.exe)
    execute_process(
            COMMAND \"${CMAKE_COMMAND}\" -E
                env PATH=\"${_qt_bin_dir}\" \"${WINDEPLOYQT_EXECUTABLE}\"
                    --dry-run
                    --no-compiler-runtime
                    --no-angle
                    --no-opengl-sw
                    --list mapping
                    \${_file}
            OUTPUT_VARIABLE _output
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    separate_arguments(_files WINDOWS_COMMAND \${_output})
    while(_files)
            list(GET _files 0 _src)
            list(GET _files 1 _dest)
            execute_process(
                COMMAND \"${CMAKE_COMMAND}\" -E
                    copy \${_src} \"\${CMAKE_INSTALL_PREFIX}/bin/\${_dest}\"
            )
            list(REMOVE_AT _files 0 1)
    endwhile()
    "
)
