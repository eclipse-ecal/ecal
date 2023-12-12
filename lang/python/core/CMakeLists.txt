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

find_package(Python3 REQUIRED COMPONENTS Development.Module)
find_package(Protobuf REQUIRED)

set(ecal_lang_py_src
    src/ecal_wrap.cxx
)

Python3_add_library(${PROJECT_NAME} MODULE ${ecal_lang_py_src})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        eCAL::core
        eCAL::core_pb
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

install(TARGETS ${PROJECT_NAME}
        EXCLUDE_FROM_ALL
        COMPONENT python_modules
        DESTINATION ${PY_BUILD_CMAKE_MODULE_NAME})

install(PROGRAMS $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
        EXCLUDE_FROM_ALL
        COMPONENT python_modules
        DESTINATION ${PY_BUILD_CMAKE_MODULE_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER lang/python/core)

