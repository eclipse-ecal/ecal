# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2020 Continental Corporation
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

project(rec)

find_package(tclap REQUIRED)
find_package(Protobuf REQUIRED)
find_package(termcolor REQUIRED)

set(source_files
  src/bytes_to_pretty_string_utils.h
  src/command_executor.cpp
  src/command_executor.h
  src/ecal_rec_server_cli.cpp
  src/rec_server_service.cpp
  src/rec_server_service.h
  
  src/commands/activate.cpp
  src/commands/activate.h
  src/commands/command.cpp
  src/commands/command.h
  src/commands/comment.cpp
  src/commands/comment.h
  src/commands/deactivate.cpp
  src/commands/deactivate.h
  src/commands/delete.cpp
  src/commands/delete.h
  src/commands/exit.cpp
  src/commands/exit.h
  src/commands/get_config.cpp
  src/commands/get_config.h
  src/commands/load_config.cpp
  src/commands/load_config.h
  src/commands/record.cpp
  src/commands/record.h
  src/commands/save_config.cpp
  src/commands/save_config.h
  src/commands/save_pre_buffer.cpp
  src/commands/save_pre_buffer.h
  src/commands/set_config.cpp
  src/commands/set_config.h
  src/commands/sleep.cpp
  src/commands/sleep.h
  src/commands/status.cpp
  src/commands/status.h
  src/commands/stop_recording.cpp
  src/commands/stop_recording.h
  src/commands/table_printer.cpp
  src/commands/table_printer.h
  src/commands/upload.cpp
  src/commands/upload.h
)

if(WIN32)
    set(win_src
        resources/resource.h
        resources/win32_resources.rc
    )
endif(WIN32)

ecal_add_app_console(${PROJECT_NAME} ${source_files} ${win32_source_files})

target_include_directories(${PROJECT_NAME}
  PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE $<$<BOOL:${MSVC}>:PCRE_STATIC;_UNICODE>)

create_targets_protobuf()

target_link_libraries(${PROJECT_NAME}
  tclap::tclap
  termcolor::termcolor
  eCAL::rec_server_core
  eCAL::core
  eCAL::utils
  CustomTclap
  ThreadingUtils
)

# Create a source tree that mirrors the filesystem
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}"
    FILES
        ${source_files}
        ${win32_source_files}
)


ecal_install_app(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER app/rec)
