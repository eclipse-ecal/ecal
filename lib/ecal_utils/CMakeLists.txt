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
project(utils)

ecal_find_package(Threads REQUIRED)

set (ecal_utils_includes
  include/ecal_utils/filesystem.h
  include/ecal_utils/ecal_utils.h
  include/ecal_utils/string.h
)

set(ecal_utils_src
    src/filesystem.cpp
)

ecal_add_library (${PROJECT_NAME}
    ${ecal_utils_includes}
    ${ecal_utils_src}
)

add_library(eCAL::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE src/)
target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

if(MSVC)
  # Create a source tree that mirrors the filesystem
  msvc_source_tree(${ecal_utils_includes} ${ecal_utils_src})
endif()

ecal_install_library(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER lib)
