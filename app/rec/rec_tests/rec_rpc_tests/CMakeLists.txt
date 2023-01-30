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

project(rec_rpc_tests)

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

set(source_files
  src/rec_cli_test.cpp
  src/external_ecal_rec.cpp
  src/external_ecal_rec.h
)

source_group(
    TREE
        ${CMAKE_CURRENT_LIST_DIR}
    FILES
        ${source_files}
)

ecal_add_gtest(${PROJECT_NAME} ${source_files})

target_compile_definitions(${PROJECT_NAME}
    PRIVATE ECAL_REC_CLI_PATH="$<TARGET_FILE:rec>"
    PRIVATE ECAL_REC_GUI_PATH="$<TARGET_FILE:rec_gui>"
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    eCAL::core
    Threads::Threads
    eCAL::rec_server_core
)

# Depend on the Rec CLI to make sure it is already built
add_dependencies(${PROJECT_NAME}
  rec
  rec_gui
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER app/rec/rec_tests/)
