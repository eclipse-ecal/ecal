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

project(_ecal_core_py)

ecal_find_package(Python COMPONENTS Development Interpreter)

set(ecal_lang_py_src
    src/ecal_wrap.cxx
)

ecal_add_python_module(${PROJECT_NAME} SOURCES ${ecal_lang_py_src} PYTHON_CODE ${CMAKE_CURRENT_SOURCE_DIR}/ecal)

target_link_libraries(${PROJECT_NAME} PRIVATE Python::Python)
target_link_libraries(${PROJECT_NAME} PRIVATE eCAL::core)
target_link_libraries(${PROJECT_NAME} PRIVATE eCAL::pb)

ecal_find_package(Protobuf REQUIRED)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER lang/python/core)

if(ECAL_INCLUDE_PY_SAMPLES)
  if(WIN32)

    include_external_msproject(ecal_core_py ${CMAKE_CURRENT_SOURCE_DIR}/ecal_core_py.pyproj)
    set_property(TARGET ecal_core_py PROPERTY FOLDER lang/python/core)

  endif()
endif()
