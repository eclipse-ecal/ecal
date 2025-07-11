# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2023 Continental Corporation
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

cmake_minimum_required(VERSION 3.15)

project(perftool)

find_package(Threads REQUIRED)
find_package(eCAL REQUIRED)

set(source_files
  src/main.cpp
  src/publisher.cpp
  src/publisher.h
  src/publisher_statistics.h
  src/subscriber.cpp
  src/subscriber.h
  src/subscriber_statistics.h
)

#add_executable(${PROJECT_NAME} ${source_files})
ecal_add_sample(${PROJECT_NAME} ${source_files})

target_link_libraries(${PROJECT_NAME} PRIVATE
  eCAL::core
  Threads::Threads
)

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

source_group(TREE "${CMAKE_CURRENT_LIST_DIR}"
    FILES
        ${source_files}
)

ecal_install_sample(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER samples/cpp/benchmarks/perftool)
