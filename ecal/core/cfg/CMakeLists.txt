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

project(ecal_generate_config)

# Set the name of the YAML file to be generated
set(ECAL_YAML "ecal.yaml")
set(TIME_YAML "ecaltime.yaml")

add_executable(${PROJECT_NAME} 
  ${ECAL_CORE_PROJECT_ROOT}/core/cfg/generate_configuration_yaml.cpp
  ${ECAL_CORE_PROJECT_ROOT}/core/src/config/configuration_writer.cpp
  ${ECAL_CORE_PROJECT_ROOT}/core/src/config/default_configuration.cpp
  ${ECAL_CORE_PROJECT_ROOT}/core/src/config/ecal_path_processing.cpp
)

# Specify include directories for the executable
target_include_directories(${PROJECT_NAME} PRIVATE 
  ${ECAL_CORE_PROJECT_ROOT}/core/src
  ${ECAL_CORE_PROJECT_ROOT}/core/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  eCAL::core
  eCAL::ecal-utils
)

ecal_install_app(${PROJECT_NAME})