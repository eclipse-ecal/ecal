# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2025 Continental Corporation
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

######
# build
######
add_library(python_top_level INTERFACE)

#configure_file("${CMAKE_CURRENT_LIST_DIR}/_version.py.in"
#               "${CMAKE_CURRENT_BINARY_DIR}/ecal/_version.py"
#                @ONLY)

set(python_top_level_files
  ecal/__init__.py
 # ${CMAKE_CURRENT_BINARY_DIR}/ecal/_version.py
)

target_sources(python_top_level
  INTERFACE    
    FILE_SET python_top_level_files
    TYPE HEADERS
    BASE_DIRS 
      ./ecal
  #    ${CMAKE_CURRENT_BINARY_DIR}/ecal
    FILES
      ${python_top_level_files}
)

#should we install the filesets or this this done via setup.toml
install(TARGETS python_top_level
  FILE_SET python_top_level_files DESTINATION ecal COMPONENT python EXCLUDE_FROM_ALL
)

ecal_python_set_output_directory(${PROJECT_NAME})