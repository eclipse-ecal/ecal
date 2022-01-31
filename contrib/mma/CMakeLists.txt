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

set(mma_src
  include/interruptable_timer.h
  include/logger.h
  include/mma.h
  include/mma_defs.h
  include/mma_impl.h
  include/query_manager.h
  include/zombie_instance_killer.h

  src/logger.cpp
  src/mma.cpp
  src/mma_application.cpp
  src/query_manager.cpp
  src/zombie_instance_killer.cpp
)
if(WIN32)
  list(APPEND mma_src
    include/windows/disk.h
    include/windows/memory.h
    include/windows/mma_windows.h
    include/windows/network.h
    include/windows/process.h
    include/windows/processes.h
    include/windows/processor.h
    include/windows/ressource.h
    
    src/windows/disk.cpp
    src/windows/memory.cpp
    src/windows/mma_windows.cpp
    src/windows/network.cpp
    src/windows/process.cpp
    src/windows/processes.cpp
    src/windows/processor.cpp
    src/windows/ressource.cpp
  )
else()
  list(APPEND mma_src
    include/linux/mma_linux.h
    include/linux/pipe_refresher.h
    include/linux/ressource.h
    
    src/linux/mma_linux.cpp
    src/linux/pipe_refresher.cpp
  )
endif()

if(WIN32)
  set(mma_win_src
      src/mma.rc
  )
endif()

ecal_add_app_console(${PROJECT_NAME} ${mma_src} ${mma_win_src})

target_include_directories(${PROJECT_NAME}
  PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

create_targets_protobuf()

target_link_libraries(${PROJECT_NAME}
  protobuf::libprotobuf
  Threads::Threads
  eCAL::core eCAL::pb
  $<$<BOOL:${WIN32}>:Pdh>
  $<$<BOOL:${WIN32}>:wbemuuid.lib>)

ecal_install_app(${PROJECT_NAME})

source_group(TREE "${CMAKE_CURRENT_LIST_DIR}"
    FILES
        ${mma_src}
        ${mma_win_src}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER contrib)
