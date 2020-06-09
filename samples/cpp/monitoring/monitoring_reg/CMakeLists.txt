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

project(monitoring_reg)

ecal_find_package(eCAL REQUIRED)

set(monitoring_reg_src
    src/monitoring_reg.cpp
)

ecal_add_sample(${PROJECT_NAME} ${monitoring_reg_src})

target_link_libraries(${PROJECT_NAME} eCAL::core)

target_link_libraries(${PROJECT_NAME} eCAL::pb)

ecal_install_sample(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER samples/cpp/monitoring)
