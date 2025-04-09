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

project(mon_tui)

find_package(Protobuf REQUIRED)
find_package(tclap REQUIRED)
find_package(ftxui REQUIRED)

set(ecalmon_tui_src
  src/main.cpp

  src/args.hpp
  src/ecal_mon_tui_defs.h
  src/resource.h
  src/model/log.hpp
  src/model/monitor.hpp
  src/model/data/host.hpp
  src/model/data/process.hpp
  src/model/data/service.hpp
  src/model/data/topic.hpp
  src/tui/command_manager.hpp
  src/tui/event_loop.hpp
  src/tui/keybinds_manager.hpp
  src/tui/notification_manager.hpp
  src/tui/style_sheet.hpp
  src/tui/tui.hpp
  src/tui/view/command_line.hpp
  src/tui/view/factory.hpp
  src/tui/view/help.hpp
  src/tui/view/hosts.hpp
  src/tui/view/logs.hpp
  src/tui/view/log_details.hpp
  src/tui/view/processes.hpp
  src/tui/view/process_details.hpp
  src/tui/view/services.hpp
  src/tui/view/service_details.hpp
  src/tui/view/shell.hpp
  src/tui/view/system_information.hpp
  src/tui/view/table.hpp
  src/tui/view/topics.hpp
  src/tui/view/topic_details.hpp
  src/tui/view/view.hpp
  src/tui/view/component/bytes.hpp
  src/tui/view/component/data_table.hpp
  src/tui/view/component/decorator.hpp
  src/tui/view/component/focus_manager.hpp
  src/tui/view/component/non_wrapping_parameter.hpp
  src/tui/view/component/scroller.hpp
  src/tui/view/component/tree.hpp
  src/tui/view/message_visualization/factory.hpp
  src/tui/view/message_visualization/messaage_vizualization.hpp
  src/tui/view/message_visualization/proto.hpp
  src/tui/view/message_visualization/proto_tree.hpp
  src/tui/view/message_visualization/raw.hpp
  src/tui/view/message_visualization/string.hpp
  src/tui/viewmodel/command_line.hpp
  src/tui/viewmodel/factory.hpp
  src/tui/viewmodel/help.hpp
  src/tui/viewmodel/hosts.hpp
  src/tui/viewmodel/logs.hpp
  src/tui/viewmodel/processes.hpp
  src/tui/viewmodel/services.hpp
  src/tui/viewmodel/shell.hpp
  src/tui/viewmodel/system_information.hpp
  src/tui/viewmodel/table.hpp
  src/tui/viewmodel/topics.hpp
  src/tui/viewmodel/viewmodel.hpp
  src/tui/viewmodel/message_visualization/message_visualization.hpp
  src/tui/viewmodel/message_visualization/proto.hpp
  src/tui/viewmodel/message_visualization/raw.hpp
  src/tui/viewmodel/message_visualization/string.hpp
  src/utils/container.hpp
  src/utils/string.hpp
)

source_group(
    TREE
        ${CMAKE_CURRENT_LIST_DIR}
    FILES
        ${ecalmon_tui_src}
)

if(WIN32)
  set(ecalmon_tui_win_src
    src/ecal_mon_tui.rc
  )
endif()

ecal_add_app_console(${PROJECT_NAME} ${ecalmon_tui_src} ${ecalmon_tui_win_src})

target_include_directories(${PROJECT_NAME}
  PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE 
    $<$<BOOL:${MSVC}>:PCRE_STATIC;_UNICODE>
    FTXUI_VERSION_MAJOR=${ftxui_VERSION_MAJOR}
  )

create_targets_protobuf()

target_link_libraries(${PROJECT_NAME} PRIVATE
  protobuf::libprotobuf
  tclap::tclap
  eCAL::protobuf_core
  eCAL::string_core
  eCAL::core_pb
  ftxui::screen
  ftxui::dom
  ftxui::component)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

ecal_install_app(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER app/mon)
