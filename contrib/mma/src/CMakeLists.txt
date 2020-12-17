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

project(mma)

find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)

if(WIN32)
  set(mma_src
    mma_application.cpp
    mma.cpp
    logger.cpp
    zombie_instance_killer.cpp
    query_manager.cpp
    windows/disk.cpp
    windows/memory.cpp
    windows/mma_windows.cpp
    windows/network.cpp
    windows/processes.cpp
    windows/processor.cpp
    windows/ressource.cpp
  )
else()
  set(mma_src
    mma_application.cpp
    mma.cpp
    logger.cpp
    zombie_instance_killer.cpp
    query_manager.cpp
    linux/mma_linux.cpp
    linux/pipe_refresher.cpp
  )
endif()

if(WIN32)
  set(mma_win_src
      mma.rc
  )
endif()

ecal_add_app_console(${PROJECT_NAME} ${mma_src} ${mma_win_src})

target_include_directories(${PROJECT_NAME}
  PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)

create_targets_protobuf()

target_link_libraries(${PROJECT_NAME}
  protobuf::libprotobuf
  Threads::Threads
  eCAL::core eCAL::pb
  $<$<BOOL:${WIN32}>:Pdh>
  $<$<BOOL:${WIN32}>:wbemuuid.lib>)

ecal_install_app(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER contrib)
