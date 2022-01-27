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

project(CustomTclap)

find_package(tclap REQUIRED)

set(include_files
  include/custom_tclap/advanced_tclap_output.h
  include/custom_tclap/fuzzy_duo_value_arg_unsigned_longlong_string.h
  include/custom_tclap/fuzzy_value_switch_arg_bool.h
  include/custom_tclap/fuzzy_value_switch_arg_double.h
  include/custom_tclap/fuzzy_value_switch_arg_unsigned_longlong.h
)


set(src_files
  src/advanced_tclap_output.cpp
  src/fuzzy_duo_value_arg_unsigned_longlong_string.cpp
  src/fuzzy_value_switch_arg_bool.cpp
  src/fuzzy_value_switch_arg_double.cpp
  src/fuzzy_value_switch_arg_unsigned_longlong.cpp
)

ecal_add_static_library(${PROJECT_NAME} 
  ${include_files}
  ${src_files}
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC 
	    tclap::tclap
)

target_include_directories(${PROJECT_NAME}
  PUBLIC include/
  PRIVATE src/)

# Create a source tree that mirrors the filesystem
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}"
    FILES
        ${include_files}
        ${src_files}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER lib/)
