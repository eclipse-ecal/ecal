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

# We need this for protoc. It's a bit stupid, because we don't really need a C++ package here
# let's check if we can do something with nuget instead maybe

find_package(Protobuf REQUIRED)
include(protobuf_csharp_helper.cmake)

project(person_protobufs_cs LANGUAGES CSharp)

add_library(person_protobufs_cs SHARED)

PROTOBUF_TARGET_CSHARP(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_SOURCE_DIR}/animal.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/house.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/person.proto)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_ROOTNAMESPACE ${PROJECT_NAME}
    VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.6.1"
    VS_DOTNET_REFERENCES "System"
)


set_property(TARGET ${PROJECT_NAME}  PROPERTY
  VS_PACKAGE_REFERENCES "Google.Protobuf_${ECAL_CSHARP_PROTOBUF_VERSION}"
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER samples/csharp/pubsub/protobuf)