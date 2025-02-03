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

include(CSharpUtilities)

project(Continental.eCAL.Protobuf LANGUAGES CSharp)

set(core_cs_src
  ProtoHelper.cs
  ProtoSubscriber.cs
  ProtoPublisher.cs
)

add_library(${PROJECT_NAME} SHARED ${core_cs_src})

target_link_libraries(${PROJECT_NAME} PUBLIC Continental.eCAL.Core)

set_target_properties(${PROJECT_NAME} PROPERTIES
  VS_GLOBAL_ROOTNAMESPACE ${PROJECT_NAME}
  VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.1"
)

if (ECAL_CSHARP_PROTOBUF_VERSION VERSION_GREATER "3.16.0")
target_compile_definitions(${PROJECT_NAME} PRIVATE "ProtobufReflectionSupport")
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY
  VS_PACKAGE_REFERENCES "Google.Protobuf_${ECAL_CSHARP_PROTOBUF_VERSION}"
)

install(TARGETS ${PROJECT_NAME}
    # IMPORTANT: Add the library to the "export-set"
    EXPORT eCALTargets
    RUNTIME COMPONENT runtime
    LIBRARY COMPONENT sdk
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER lang/csharp)
