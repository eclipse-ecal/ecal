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

project(play_core)

ecal_find_package(Threads REQUIRED)
ecal_find_package(Protobuf REQUIRED)
ecal_find_package(spdlog REQUIRED)

set(source_files
  include/continuity_report.h
  include/ecal_play.h
  include/ecal_play_globals.h
  include/ecal_play_logger.h
  include/ecal_play_scenario.h
  include/ecal_play_state.h

  src/ecal_play.cpp
  src/ecal_play_command.h
  src/play_thread.cpp
  src/play_thread.h
  src/state_publisher_thread.cpp
  src/state_publisher_thread.h
  src/stop_watch.cpp
  src/stop_watch.h

  src/measurement_container.cpp
  src/measurement_container.h
) 

add_library(${PROJECT_NAME} ${source_files})
add_library(eCAL::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_include_directories(${PROJECT_NAME} PUBLIC  include)

create_targets_protobuf()

target_link_libraries(${PROJECT_NAME} PUBLIC
  Threads::Threads
  spdlog::spdlog
  protobuf::libprotobuf
  eCAL::core
  eCAL::pb
  eCAL::hdf5
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  ThreadingUtils
  eCAL::utils
)

if(MSVC)
  msvc_source_tree(
    ${source_files}
  )
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER app/play)
