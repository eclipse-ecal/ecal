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

# Allow the install command to use generator expressions
if(POLICY CMP0087)
    cmake_policy(SET CMP0087 NEW)
endif()

project(mon_gui)

# Legacy Qt5 (pre 5.15) support as suggested by teh Qt Documentation:
# https://doc.qt.io/qt-6/cmake-qt5-and-qt6-compatibility.html#supporting-older-qt-5-versions
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

find_package(Protobuf REQUIRED)

if(ECAL_USE_NPCAP)
  find_package(udpcap REQUIRED)
endif(ECAL_USE_NPCAP)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF) # Reason for being turned off: AutoUIC will prevent VS from detecting changes in .ui files
set(CMAKE_AUTORCC OFF) # Reason for being turned off: AutoRCC will create an entirely new project in VS which clutters the solution appearance. Additionally, we cannot assign a source group to the generated .cpp files which will clutter the project.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(source_files
    src/ecalmon.cpp
    src/ecalmon.h
    src/ecalmon_globals.h
    src/main.cpp
    src/util.h

    src/plugin/plugin_manager.cpp
    src/plugin/plugin_manager.h
    src/plugin/plugin_wrapper.cpp
    src/plugin/plugin_wrapper.h

    src/widgets/about_dialog/about_dialog.cpp
    src/widgets/about_dialog/about_dialog.h

    src/widgets/column_selector_dialog/column_picker_delegate.cpp
    src/widgets/column_selector_dialog/column_picker_delegate.h
    src/widgets/column_selector_dialog/column_selector_dialog.cpp
    src/widgets/column_selector_dialog/column_selector_dialog.h

    src/widgets/ecalmon_tree_widget/ecalmon_tree_widget.cpp
    src/widgets/ecalmon_tree_widget/ecalmon_tree_widget.h
    src/widgets/ecalmon_tree_widget/host_widget.cpp
    src/widgets/ecalmon_tree_widget/host_widget.h
    src/widgets/ecalmon_tree_widget/process_widget.cpp
    src/widgets/ecalmon_tree_widget/service_widget.cpp
    src/widgets/ecalmon_tree_widget/service_widget.h
    src/widgets/ecalmon_tree_widget/topic_widget.cpp
    src/widgets/ecalmon_tree_widget/topic_widget.h

    src/widgets/license_dialog/license_dialog.cpp
    src/widgets/license_dialog/license_dialog.h

    src/widgets/log_widget/log_widget.cpp
    src/widgets/log_widget/log_widget.h

    src/widgets/models/column_tree_item.cpp
    src/widgets/models/column_tree_item.h
    src/widgets/models/column_tree_model.cpp
    src/widgets/models/column_tree_model.h
    src/widgets/models/group_tree_item.cpp
    src/widgets/models/group_tree_item.h
    src/widgets/models/group_tree_model.cpp
    src/widgets/models/group_tree_model.h
    src/widgets/models/host_tree_item.cpp
    src/widgets/models/host_tree_item.h
    src/widgets/models/host_tree_model.cpp
    src/widgets/models/host_tree_model.h
    src/widgets/models/item_data_roles.h
    src/widgets/models/item_view_model.cpp
    src/widgets/models/item_view_model.h
    src/widgets/models/log_model.cpp
    src/widgets/models/log_model.h
    src/widgets/models/log_sort_filter_proxy_model.cpp
    src/widgets/models/log_sort_filter_proxy_model.h
    src/widgets/models/plugin_table_model.cpp
    src/widgets/models/plugin_table_model.h
    src/widgets/models/process_tree_item.cpp
    src/widgets/models/process_tree_item.h
    src/widgets/models/process_tree_model.cpp
    src/widgets/models/process_tree_model.h
    src/widgets/models/service_tree_item.h
    src/widgets/models/service_tree_model.cpp
    src/widgets/models/service_tree_model.h
    src/widgets/models/topic_sort_filter_proxy_model.cpp
    src/widgets/models/topic_sort_filter_proxy_model.h
    src/widgets/models/topic_tree_item.cpp
    src/widgets/models/topic_tree_item.h
    src/widgets/models/topic_tree_model.cpp
    src/widgets/models/topic_tree_model.h
    src/widgets/models/tree_item_type.h

    src/widgets/plugin_settings_dialog/plugin_settings_dialog.cpp
    src/widgets/plugin_settings_dialog/plugin_settings_dialog.h

    src/widgets/raw_monitoring_data_widget/protobuf_highlighter.cpp
    src/widgets/raw_monitoring_data_widget/protobuf_highlighter.h
    src/widgets/raw_monitoring_data_widget/raw_monitoring_data_widget.cpp
    src/widgets/raw_monitoring_data_widget/raw_monitoring_data_widget.h
    src/widgets/raw_monitoring_data_widget/search_lineedit.cpp
    src/widgets/raw_monitoring_data_widget/search_lineedit.h

    src/widgets/visualisation_widget/visualisation_widget.cpp
    src/widgets/visualisation_widget/visualisation_widget.h
    src/widgets/visualisation_widget/visualisation_window.cpp
    src/widgets/visualisation_widget/visualisation_window.h
    src/widgets/visualisation_widget/visualisation_dock_widget.cpp
    src/widgets/visualisation_widget/visualisation_dock_widget.h

    src/widgets/system_information_widget/system_information_widget.cpp
    src/widgets/system_information_widget/system_information_widget.h

    src/widgets/tree_item_view_widget/tree_item_view_widget.cpp
    src/widgets/tree_item_view_widget/tree_item_view_widget.h
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
    src/main_window.ui
    src/widgets/about_dialog/about_dialog.ui
    src/widgets/column_selector_dialog/column_selector_dialog.ui
    src/widgets/ecalmon_tree_widget/ecalmon_tree_widget.ui
    src/widgets/license_dialog/license_dialog.ui
    src/widgets/log_widget/log_widget.ui
    src/widgets/plugin_settings_dialog/plugin_settings_dialog.ui
    src/widgets/raw_monitoring_data_widget/raw_monitoring_data_widget.ui
    src/widgets/system_information_widget/system_information_widget.ui
    src/widgets/tree_item_view_widget/tree_item_view_widget.ui
    src/widgets/visualisation_widget/visualisation_widget.ui
    src/widgets/visualisation_widget/visualisation_dock_widget.ui
)

if(ECAL_USE_NPCAP)
    set(source_files
        ${source_files}
        src/widgets/npcap_status_dialog/npcap_status_dialog.cpp
        src/widgets/npcap_status_dialog/npcap_status_dialog.h
    )
    set(ui_files
        ${ui_files}
        src/widgets/npcap_status_dialog/npcap_status_dialog.ui
    )
endif(ECAL_USE_NPCAP)

# compile qt resource files and ui files
if (${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_resources(autogen_resources ${qt_resource_files})
    qt_wrap_ui      (autogen_ui        ${ui_files})
else()
    qt5_add_resources(autogen_resources ${qt_resource_files})
    qt5_wrap_ui      (autogen_ui        ${ui_files})
endif()

# Add all files. The resource files and ui files are not necessary, but we want them to show up in the IDE
ecal_add_app_qt(${PROJECT_NAME} 
    ${source_files}
    ${qt_resource_files}
    ${win32_resource_files}
    ${ui_files}

    ${autogen_resources}
    ${autogen_ui}
)

target_link_libraries (${PROJECT_NAME} PRIVATE
    protobuf::libprotobuf
    eCAL::core
    eCAL::core_pb
    eCAL::mon_plugin_lib
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    CustomQt
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14) 

if(ECAL_USE_NPCAP)
  add_definitions(-DECAL_USE_NPCAP)
    target_link_libraries(${PROJECT_NAME} PRIVATE
    udpcap::udpcap
  )
endif(ECAL_USE_NPCAP)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()
if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/wd4127 /wd4714")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_include_directories(${PROJECT_NAME} PRIVATE $<TARGET_PROPERTY:eCAL::core,INCLUDE_DIRECTORIES>)


if ((WIN32 OR APPLE) AND (${QT_VERSION_MAJOR} GREATER_EQUAL 6))

    # Generate a script that will deploy all necessary Qt DLLs to the binary folder
    # https://doc.qt.io/qt-6/qt-deploy-runtime-dependencies.html
    # Available for Qt 6.3 and up (=> Not for Qt5!)
    # Executing it requires CMake 3.14 and up, due to policy https://cmake.org/cmake/help/latest/policy/CMP0087.html
    qt_generate_deploy_app_script(
        TARGET ${PROJECT_NAME}
        OUTPUT_SCRIPT qt_deploy_script
        NO_COMPILER_RUNTIME
        NO_UNSUPPORTED_PLATFORM_ERROR
    )

    # Add a postbuild script that will also execute the created script via cmake -P
    # This is necessary to make the application startable / debuggable from the build directory.
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -DQT_DEPLOY_PREFIX=$<TARGET_FILE_DIR:${PROJECT_NAME}> -DQT_DEPLOY_BIN_DIR=. -P ${qt_deploy_script}
    )

    # Use the script for deploying the qt dlls in the install dir
    install(SCRIPT ${qt_deploy_script})

elseif(WIN32)

    # For Qt5 we use our legacy script.
    # Deploy Qt DLLs in the binary folder. This is necessary for starting the application from whithin the IDE without having to copy QtCore.dll, QtWidgets.dll etc. by hand each time
    qt_add_windeployqt_postbuild(--no-system-d3d-compiler --no-compiler-runtime --no-opengl-sw --pdb "$<TARGET_FILE:${PROJECT_NAME}>")

    get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
    install(CODE
        "
        set(_file ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/ecal_mon_gui.exe)
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
endif()

# Create a source tree that mirrors the filesystem
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}"
    FILES
        ${source_files}
        ${win32_resource_files}
        ${ui_files}
)

source_group(resources FILES
    ${qt_resource_files}
)

# Also create a group for autogenerated files. The autogenerated ui files are not necessary as they are only header files. We add them anyhow, just for completeness.
source_group( autogen FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_automoc.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_autogen/mocs_compilation.cpp
    ${autogen_ui}
    ${autogen_resources}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER app/mon)

ecal_install_app(${PROJECT_NAME} START_MENU_NAME "eCAL Monitor")