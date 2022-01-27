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

find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(spdlog REQUIRED)

set(PROJECT_NAME sys_client_core)

set(source_files
  include/sys_client_core/ecal_sys_client.h
  include/sys_client_core/ecal_sys_client_defs.h
  include/sys_client_core/proto_helpers.h
  include/sys_client_core/runner.h
  include/sys_client_core/task.h
  
  src/ecal_sys_client.cpp
  src/proto_helpers.cpp
  src/task.cpp
) 

add_library (${PROJECT_NAME} ${source_files})
add_library (eCAL::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_include_directories(${PROJECT_NAME} PUBLIC  include)

create_targets_protobuf()

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    protobuf::libprotobuf
    eCAL::core
    eCAL::pb
    eCAL::utils
    spdlog::spdlog
  PRIVATE
    Threads::Threads  
    EcalParser
)

source_group(TREE "${CMAKE_CURRENT_LIST_DIR}"
    FILES
        ${source_files}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER app/sys)

