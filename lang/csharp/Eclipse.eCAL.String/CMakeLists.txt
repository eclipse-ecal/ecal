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

include(CSharpUtilities)

project(Eclipse.eCAL.String LANGUAGES CSharp)

set(core_cs_src
  StringPublisher.cs
  StringSubscriber.cs
)

add_library(${PROJECT_NAME} SHARED ${core_cs_src})

target_link_libraries(${PROJECT_NAME} PUBLIC Eclipse.eCAL.Core)

set_target_properties(${PROJECT_NAME} PROPERTIES
  VS_GLOBAL_ROOTNAMESPACE ${PROJECT_NAME}
)

install(TARGETS ${PROJECT_NAME}
    # IMPORTANT: Add the library to the "export-set"
    EXPORT eCALTargets
    RUNTIME COMPONENT runtime
    LIBRARY COMPONENT sdk
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER lang/csharp)
