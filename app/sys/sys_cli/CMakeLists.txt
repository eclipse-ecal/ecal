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

project(sys)

find_package(tclap REQUIRED)
find_package(Protobuf REQUIRED)
find_package(termcolor REQUIRED)

set(source_files
  src/command_executor.cpp
  src/command_executor.h
  src/ecalsys_cli.cpp
  src/ecalsys_service.cpp
  src/ecalsys_service.h
  src/ecalsys_util.cpp
  src/ecalsys_util.h
  src/sys_error.h

  src/commands/command.cpp
  src/commands/command.h
  src/commands/exit.cpp
  src/commands/exit.h
  src/commands/helpers.cpp
  src/commands/helpers.h
  src/commands/list.cpp
  src/commands/list.h
  src/commands/load_config.cpp
  src/commands/load_config.h
  src/commands/restart_tasks.cpp
  src/commands/restart_tasks.h
  src/commands/sleep.cpp
  src/commands/sleep.h
  src/commands/start_tasks.cpp
  src/commands/start_tasks.h
  src/commands/stop_tasks.cpp
  src/commands/stop_tasks.h
  src/commands/update_from_cloud.cpp
  src/commands/update_from_cloud.h
)

if(WIN32)
  set(win32_source_files
    src/ecalsys.rc
  )
endif()

ecal_add_app_console(${PROJECT_NAME} ${source_files} ${win32_source_files})

target_include_directories(${PROJECT_NAME}
  PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE $<$<BOOL:${MSVC}>:PCRE_STATIC;_UNICODE>)

create_targets_protobuf()

target_link_libraries(${PROJECT_NAME}
  tclap::tclap
  termcolor::termcolor
  eCAL::sys_core
  protobuf::libprotobuf
  CustomTclap
)

# Create a source tree that mirrors the filesystem
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}"
    FILES
        ${source_files}
        ${win32_source_files}
)

ecal_install_app(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER app/sys)
