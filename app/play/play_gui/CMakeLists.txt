# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2025 Continental Corporation
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

project(play_gui)

# Legacy Qt5 (pre 5.15) support as suggested by teh Qt Documentation:
# https://doc.qt.io/qt-6/cmake-qt5-and-qt6-compatibility.html#supporting-older-qt-5-versions
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Concurrent)

if (WIN32 AND (${QT_VERSION_MAJOR} EQUAL 5))
    # For Qt5 we find the WinExtras component.
    # It enables those taskbar buttons on Windows 7 and up.
    # Unfortunatelly, this functionality has been removed with Qt 6.0 and up.
    # (2024-01-17, Current Qt Version: 6.6)
    #
    # Maybe we can re-enable the functionality with the following external lib:
    # https://github.com/oblivioncth/Qx
    #
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS WinExtras)
endif()

find_package(tclap REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF) # Reason for being turned off: AutoUIC will prevent VS from detecting changes in .ui files
set(CMAKE_AUTORCC OFF) # Reason for being turned off: AutoRCC will create an entirely new project in VS which clutters the solution appearance. Additionally, we cannot assign a source group to the generated .cpp files which will clutter the project.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(source_files
    src/ecal_play_gui.cpp
    src/ecal_play_gui.h
    src/ecal_play_service.cpp
    src/ecal_play_service.h
    src/help_window.cpp
    src/help_window.h
    src/main.cpp
    src/q_ecal_play.cpp
    src/q_ecal_play.h

    src/widgets/about_dialog/about_dialog.cpp
    src/widgets/about_dialog/about_dialog.h

    src/widgets/channel_widget/channel_widget.cpp
    src/widgets/channel_widget/channel_widget.h

    src/widgets/description_widget/description_widget.cpp
    src/widgets/description_widget/description_widget.h

    src/widgets/license_dialog/license_dialog.cpp
    src/widgets/license_dialog/license_dialog.h

    src/widgets/models/channel_tree_item.cpp
    src/widgets/models/channel_tree_item.h
    src/widgets/models/channel_tree_model.cpp
    src/widgets/models/channel_tree_model.h
    src/widgets/models/item_data_roles.h
    src/widgets/models/tree_item_type.h

    src/widgets/player_controls_widget/player_controls_widget.cpp
    src/widgets/player_controls_widget/player_controls_widget.h
    src/widgets/player_controls_widget/player_time_slider.cpp
    src/widgets/player_controls_widget/player_time_slider.h

    src/widgets/scenario_widget/edit_button_delegate.cpp
    src/widgets/scenario_widget/edit_button_delegate.h
    src/widgets/scenario_widget/scenario_model.cpp
    src/widgets/scenario_widget/scenario_model.h
    src/widgets/scenario_widget/scenario_widget.cpp
    src/widgets/scenario_widget/scenario_widget.h

    src/widgets/settings_widget/settings_widget.cpp
    src/widgets/settings_widget/settings_widget.h
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
    src/widgets/channel_widget/channel_widget.ui
    src/widgets/description_widget/description_widget.ui
    src/widgets/license_dialog/license_dialog.ui
    src/widgets/player_controls_widget/player_controls_widget.ui
    src/widgets/scenario_widget/scenario_widget.ui
    src/widgets/settings_widget/settings_widget.ui
)

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

create_targets_protobuf()
target_link_libraries (${PROJECT_NAME} PRIVATE
    tclap::tclap
    CustomTclap
    eCAL::play_core
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    CustomQt
    eCAL::core_pb
    eCAL::ecal-utils
)

if (WIN32 AND (${QT_VERSION_MAJOR} EQUAL 5))
    # For Qt5 we link against the WinExtras component.
    # It enables those taskbar buttons on Windows 7 and up.
    # Unfortunatelly, this functionality has been removed with Qt 6.0 and up.
    # (2024-01-17, Current Qt Version: 6.6)
    #
    # Maybe we can re-enable the functionality with the following external lib:
    # https://github.com/oblivioncth/Qx
    #
    target_link_libraries (${PROJECT_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::WinExtras
    )
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

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

target_link_options(${PROJECT_NAME} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/ignore:4098,4099>)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER app/play)

ecal_install_app(${PROJECT_NAME} START_MENU_NAME "eCAL Player")

# install files required for linux mimetypes and icon
if (UNIX)
    INSTALL (FILES "${CMAKE_CURRENT_LIST_DIR}/mimetype/ecal-play.xml"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/mime/packages/")

    INSTALL (FILES "${CMAKE_CURRENT_LIST_DIR}/mimetype/application-ecalmeas.svg"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/Humanity/scalable/mimetypes/")
endif()
