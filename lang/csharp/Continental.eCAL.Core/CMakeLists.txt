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

project(Continental.eCAL.Core)

find_package(eCAL REQUIRED)

set(ecal_clr_src
    AssemblyInfo.cpp
    ecal_clr.h
    ecal_clr.cpp
) 

add_library(${PROJECT_NAME} SHARED ${ecal_clr_src})

target_include_directories(${PROJECT_NAME} PRIVATE .)
target_link_libraries(${PROJECT_NAME} PRIVATE eCAL::core)
set_target_properties(${PROJECT_NAME} PROPERTIES
    COMMON_LANGUAGE_RUNTIME ""
)

set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

install(TARGETS ${PROJECT_NAME}
    # IMPORTANT: Add the library to the "export-set"
    EXPORT eCALTargets
    RUNTIME COMPONENT runtime
    LIBRARY COMPONENT sdk
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER lang/csharp)
