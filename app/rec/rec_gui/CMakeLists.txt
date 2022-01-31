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

set(PROJECT_NAME rec_gui)

find_package(Qt5 COMPONENTS
    Core
    Widgets
REQUIRED)

if(WIN32)
    find_package(Qt5 COMPONENTS
        WinExtras
    REQUIRED)
endif(WIN32)

find_package(simpleini REQUIRED)
find_package(tclap REQUIRED)
find_package(Protobuf REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF) # Reason for being turned off: AutoUIC will prevent VS from detecting changes in .ui files
set(CMAKE_AUTORCC OFF) # Reason for being turned off: AutoRCC will create an entirely new project in VS which clutters the solution appearance. Additionally, we cannot assign a source group to the generated .cpp files which will clutter the project.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(source_files
    src/ecalrec_gui.cpp
    src/ecalrec_gui.h
    src/help_window.cpp
    src/help_window.h

    src/main.cpp

    src/qecalrec.cpp
    src/qecalrec.h

    src/models/item_data_roles.h
    src/models/tree_item_types.h

    src/service/rec_server_service.h
    src/service/rec_server_service.cpp

    src/widgets/bytes_to_pretty_string_utils.h

    src/widgets/about_dialog/about_dialog.cpp
    src/widgets/about_dialog/about_dialog.h

    src/widgets/config_widget/config_widget.cpp
    src/widgets/config_widget/config_widget.h

    src/widgets/control_widget/control_widget.cpp
    src/widgets/control_widget/control_widget.h

    src/widgets/license_dialog/license_dialog.cpp
    src/widgets/license_dialog/license_dialog.h

    src/widgets/recordermanager_widget/host_filter_delegate.cpp
    src/widgets/recordermanager_widget/host_filter_delegate.h
    src/widgets/recordermanager_widget/host_picker.cpp
    src/widgets/recordermanager_widget/host_picker.h
    src/widgets/recordermanager_widget/recorder_model.cpp
    src/widgets/recordermanager_widget/recorder_model.h
    src/widgets/recordermanager_widget/recordermanager_add_dialog.h
    src/widgets/recordermanager_widget/recordermanager_add_dialog.cpp
    src/widgets/recordermanager_widget/recordermanager_widget.cpp
    src/widgets/recordermanager_widget/recordermanager_widget.h

    src/widgets/recording_history_widget/push_button_delegate.cpp
    src/widgets/recording_history_widget/push_button_delegate.h
    src/widgets/recording_history_widget/job_history_jobitem.cpp
    src/widgets/recording_history_widget/job_history_jobitem.h
    src/widgets/recording_history_widget/job_history_model.cpp
    src/widgets/recording_history_widget/job_history_model.h
    src/widgets/recording_history_widget/job_history_recorderitem.cpp
    src/widgets/recording_history_widget/job_history_recorderitem.h
    src/widgets/recording_history_widget/recording_history_widget.cpp
    src/widgets/recording_history_widget/recording_history_widget.h

    src/widgets/remote_measurement_debug_widget/remote_measurement_debug_widget.cpp
    src/widgets/remote_measurement_debug_widget/remote_measurement_debug_widget.h

    src/widgets/topic_widget/row_height_delegate.cpp
    src/widgets/topic_widget/row_height_delegate.h
    src/widgets/topic_widget/topiclist_dialog.cpp
    src/widgets/topic_widget/topiclist_dialog.h
    src/widgets/topic_widget/topic_widget.cpp
    src/widgets/topic_widget/topic_widget.h
    src/widgets/topic_widget/topic_list_model.cpp
    src/widgets/topic_widget/topic_list_model.h

    src/widgets/upload_settings_dialog/upload_settings_dialog.cpp
    src/widgets/upload_settings_dialog/upload_settings_dialog.h

    src/widgets/wait_for_shutdown_dialog/wait_for_shutdown_dialog.cpp
    src/widgets/wait_for_shutdown_dialog/wait_for_shutdown_dialog.h
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
    src/help_window.ui
    src/main_window.ui
    src/widgets/about_dialog/about_dialog.ui
    src/widgets/config_widget/config_widget.ui
    src/widgets/control_widget/control_widget.ui
    src/widgets/license_dialog/license_dialog.ui
    src/widgets/recordermanager_widget/host_picker.ui
    src/widgets/recordermanager_widget/recordermanager_widget.ui
    src/widgets/recordermanager_widget/recordermanager_add_dialog.ui
    src/widgets/recording_history_widget/recording_history_widget.ui
    src/widgets/remote_measurement_debug_widget/remote_measurement_debug_widget.ui
    src/widgets/topic_widget/topiclist_dialog.ui
    src/widgets/topic_widget/topic_widget.ui
    src/widgets/upload_settings_dialog/upload_settings_dialog.ui
    src/widgets/wait_for_shutdown_dialog/wait_for_shutdown_dialog.ui
)

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
    simpleini::simpleini
    tclap::tclap
    protobuf::libprotobuf
    eCAL::core
    eCAL::pb
    eCAL::rec_server_core
    Qt5::Widgets
    CustomQt
    CustomTclap
    eCAL::utils
    EcalParser
    QEcalParser
    ThreadingUtils
)
if (WIN32)
    target_link_libraries (${PROJECT_NAME}
        Qt5::WinExtras
    )
endif(WIN32)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif(WIN32)
if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/wd4127 /wd4714")
ENDIF(MSVC)

target_include_directories(${PROJECT_NAME} PRIVATE src)


if(WIN32)
    # Deploy Qt DLLs in the binary folder. This is necessary for starting the application from whithin the IDE without having to copy QtCore.dll, QtWidgets.dll etc. by hand each time
    qt_add_windeployqt_postbuild(--no-system-d3d-compiler --no-compiler-runtime --no-opengl-sw --pdb "$<TARGET_FILE:${PROJECT_NAME}>")
endif(WIN32)

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

target_link_options(${PROJECT_NAME} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/ignore:4098,4099>)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER app/rec)

ecal_install_app(${PROJECT_NAME} START_MENU_NAME "eCAL Recorder")

get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

# Running this with MSVC 2015 requires CMake 3.6+
if((MSVC_VERSION VERSION_EQUAL 1900 OR MSVC_VERSION VERSION_GREATER 1900)
        AND CMAKE_VERSION VERSION_LESS "3.6")
    message(WARNING "Deploying with MSVC 2015+ requires CMake 3.6+")
endif()

if(WIN32)
install(CODE
    "
    set(_file ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/ecal_rec_gui.exe)
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
