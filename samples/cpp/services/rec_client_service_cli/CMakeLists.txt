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

project(rec_client_service_cli)

find_package(eCAL REQUIRED)

set(ecalrecorder_client_src
  src/ecalrecorder_client.cpp
)

ecal_add_sample(${PROJECT_NAME} ${ecalrecorder_client_src})

target_include_directories(${PROJECT_NAME} PRIVATE .)

target_link_libraries(${PROJECT_NAME}
  eCAL::core
  eCAL::pb)

ecal_install_sample(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER samples/cpp/services)
