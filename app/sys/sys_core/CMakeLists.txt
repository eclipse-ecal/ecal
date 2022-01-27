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

project(sys_core)

find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(spdlog REQUIRED)
find_package(tinyxml2 REQUIRED)

set(ecalsyscore_src
  include/ecalsys/ecal_sys.h
  include/ecalsys/ecal_sys_logger.h
  include/ecalsys/esys_defs.h
  include/ecalsys/proto_helpers.h
  include/ecalsys/config/config_version.h
  include/ecalsys/task/ecal_sys_runner.h
  include/ecalsys/task/ecal_sys_task.h
  include/ecalsys/task/ecal_sys_task_helper.h
  include/ecalsys/task/task_group.h
  include/ecalsys/task/task_state.h
  include/threading/blocking_queue.h
  include/threading/interruptible_loop_thread.h
  include/threading/interruptible_thread.h
  include/threading/threadsafe_container.h

  src/config/config_manager.cpp
  src/config/config_manager.h
  src/config/esys_cfg.h
  src/config/esys_cfg_parser.cpp
  src/config/esys_cfg_parser.h

  src/connection/abstract_connection.h
  src/connection/connection_manager.cpp
  src/connection/connection_manager.h
  src/connection/local_connection.cpp
  src/connection/local_connection.h
  src/connection/remote_connection.cpp
  src/connection/remote_connection.h

  src/ecal_sys.cpp
  src/ecal_sys_monitor.cpp
  src/ecal_sys_monitor.h
  src/proto_helpers.cpp

  src/taskaction_threads/restart_task_list_thread.cpp
  src/taskaction_threads/restart_task_list_thread.h
  src/taskaction_threads/start_task_list_thread.cpp
  src/taskaction_threads/start_task_list_thread.h
  src/taskaction_threads/stop_task_list_thread.cpp
  src/taskaction_threads/stop_task_list_thread.h
  src/taskaction_threads/task_list_thread.h
  src/taskaction_threads/update_from_cloud_task_list_thread.cpp
  src/taskaction_threads/update_from_cloud_task_list_thread.h

  src/task/ecal_sys_runner.cpp
  src/task/ecal_sys_task.cpp
  src/task/ecal_sys_task_helper.cpp
  src/task/task_group.cpp
)

add_library(${PROJECT_NAME} ${ecalsyscore_src})
add_library(eCAL::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
  PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE $<$<BOOL:${MSVC}>:PCRE_STATIC;_UNICODE>)

create_targets_protobuf()

target_link_libraries(${PROJECT_NAME}
  Threads::Threads
  tinyxml2::tinyxml2
  spdlog::spdlog
  protobuf::libprotobuf
  eCAL::core
  eCAL::pb
  EcalParser
  eCAL::sys_client_core
)


# Create a source tree that mirrors the filesystem
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}"
    FILES
        ${ecalsyscore_src}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  FOLDER app/sys)
