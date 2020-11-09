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

find_package(pybind11 CONFIG REQUIRED)

project(_ecal_sys_py)

set(${PROJECT_NAME}_src
  src/module.cpp
)

ecal_add_pybind11_module(${PROJECT_NAME}
  SOURCES ${${PROJECT_NAME}_src}
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  eCAL::sys_core
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER lang/python/ecal_sys_py)
