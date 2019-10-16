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

find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(spdlog REQUIRED)

set(source_files
  include/ContinuityReport.h
  include/EcalPlay.h
  include/EcalPlayGlobals.h
  include/EcalPlayLogger.h
  include/EcalPlayScenario.h
  include/EcalPlayState.h

  src/EcalPlay.cpp
  src/EcalPlayCommand.h
  src/PlayThread.cpp
  src/PlayThread.h
  src/StatePublisherThread.cpp
  src/StatePublisherThread.h
  src/Stopwatch.cpp
  src/Stopwatch.h

  src/MeasurementContainer.cpp
  src/MeasurementContainer.h
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
  EcalUtils
)

if(MSVC)
  msvc_source_tree(
    ${source_files}
  )
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER app/play)
