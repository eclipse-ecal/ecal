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

find_package(Threads REQUIRED)

set (ecal_utils_includes
  include/ecal_utils/command_line.h
  include/ecal_utils/filesystem.h
  include/ecal_utils/ecal_utils.h
  include/ecal_utils/str_convert.h
  include/ecal_utils/string.h
  include/ecal_utils/win_cp_changer.h
  include/ecal_utils/portable_endian.h
)

set(ecal_utils_src
    src/command_line.cpp
    src/filesystem.cpp
    src/str_convert.cpp
    src/win_cp_changer.cpp
)

ecal_add_static_library(${PROJECT_NAME}
    ${ecal_utils_includes}
    ${ecal_utils_src}
)

add_library(eCAL::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE src/)
target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# Create a source tree that mirrors the filesystem
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}"
    FILES
        ${ecal_utils_includes}
        ${ecal_utils_src}
)

ecal_install_library(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER lib)
