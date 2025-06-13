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
project(Eclipse.eCAL.Core.Test LANGUAGES CSharp)

add_library(${PROJECT_NAME} SHARED)

target_sources(${PROJECT_NAME}
  PRIVATE
    PubSubReceiveTest.cs
    PubSubEventHandlingTest.cs
    ConfigTest.cs
)

target_link_libraries(${PROJECT_NAME} Eclipse.eCAL.Core)

set_target_properties(${PROJECT_NAME} PROPERTIES
  VS_GLOBAL_ROOTNAMESPACE ${PROJECT_NAME}
  # mark this is a unit testing project, see https://cmake.org/cmake/help/latest/prop_tgt/VS_GLOBAL_PROJECT_TYPES.html
  VS_GLOBAL_PROJECT_TYPES "{3AC096D0-A1C2-E12C-1390-A8335801FDAB};{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}"
)

set_property(TARGET ${PROJECT_NAME}  PROPERTY
  VS_PACKAGE_REFERENCES 
    "MSTest.TestFramework_2.2.10;MSTest.TestAdapter_2.2.10"
)

install(TARGETS ${PROJECT_NAME}
    # IMPORTANT: Add the library to the "export-set"
    EXPORT eCALTargets
    RUNTIME COMPONENT runtime
    LIBRARY COMPONENT sdk
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER tests/csharp)

macro(ecal_add_test_executable sample_name)
  ecal_add_sample(${sample_name} ${sample_name}.cs)
  target_link_libraries(${sample_name} PRIVATE Eclipse.eCAL.Core)
  set_property(TARGET ${sample_name} PROPERTY FOLDER tests/csharp)
  add_dependencies(${sample_name} Eclipse.eCAL.Core.Test)
endmacro()

ecal_add_test_executable(testexec_csharp)

