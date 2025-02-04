# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2024 Continental Corporation
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
project(Continental.eCAL.Protobuf.Test LANGUAGES CSharp)

add_library(${PROJECT_NAME} SHARED)

target_sources(${PROJECT_NAME}
  PRIVATE
    test.cs
)

target_link_libraries(${PROJECT_NAME} Continental.eCAL.Protobuf Continental.eCAL.Protobuf.Samples.Datatypes)

set_target_properties(${PROJECT_NAME} PROPERTIES
  VS_GLOBAL_ROOTNAMESPACE ${PROJECT_NAME}
  VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.1"
  # mark this is a unit testing project, see https://cmake.org/cmake/help/latest/prop_tgt/VS_GLOBAL_PROJECT_TYPES.html
  VS_GLOBAL_PROJECT_TYPES "{3AC096D0-A1C2-E12C-1390-A8335801FDAB};{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}"
)

set_property(TARGET ${PROJECT_NAME}  PROPERTY
  VS_PACKAGE_REFERENCES 
    "Google.Protobuf_${ECAL_CSHARP_PROTOBUF_VERSION};MSTest.TestFramework_2.2.10;MSTest.TestAdapter_2.2.10"
)

install(TARGETS ${PROJECT_NAME}
    # IMPORTANT: Add the library to the "export-set"
    EXPORT eCALTargets
    RUNTIME COMPONENT runtime
    LIBRARY COMPONENT sdk
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER tests/csharp)
