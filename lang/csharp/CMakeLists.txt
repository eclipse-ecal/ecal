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

# C# build, can be executed standalone against an eCAL installation
cmake_minimum_required(VERSION 3.15)

find_package(CMakeFunctions CONFIG REQUIRED)
git_revision_information()

project(ecal_csharp
  VERSION "${GIT_REVISION_MAJOR}.${GIT_REVISION_MINOR}.${GIT_REVISION_PATCH}"
)

include(GNUInstallDirs)

# Macro to determine the matching protobuf C# version compatible with protoc found.
# We should do this differently, but then we need to install protoc via nuget or similar...
# Because the Prototbuf Version used for C# bindings is independent from the one we get from the submodules / conan
macro(get_csharp_protobuf_version)
find_package(Protobuf REQUIRED)
if (${Protobuf_VERSION} VERSION_GREATER_EQUAL 4)
  if(NOT DEFINED Protobuf_VERSION_MINOR OR NOT DEFINED Protobuf_VERSION_PATCH)
    string(REPLACE "." ";" _ecal_protobuf_version "${Protobuf_VERSION}")
    # list(GET _ecal_protobuf_version 0 Protobuf_VERSION_MAJOR)
    list(GET _ecal_protobuf_version 1 Protobuf_VERSION_MINOR)
    list(GET _ecal_protobuf_version 2 Protobuf_VERSION_PATCH)
  endif()
  if(Protobuf_VERSION_MINOR STREQUAL "NOTFOUND" OR Protobuf_VERSION_PATCH STREQUAL "NOTFOUND")
    message(FATAL_ERROR "Unable to determine Protobuf version")
  endif()
set(ECAL_CSHARP_PROTOBUF_VERSION "3.${Protobuf_VERSION_MINOR}.${Protobuf_VERSION_PATCH}")
else ()
set(ECAL_CSHARP_PROTOBUF_VERSION "${Protobuf_VERSION}")
endif ()
endmacro()

option(ECAL_CSHARP_BUILD_SAMPLES "BUILD eCAL C# samples" ON)
option(ECAL_CSHARP_BUILD_TESTS   "BUILD eCAL C# unittests" ON)

get_csharp_protobuf_version()
message(STATUS "Using ECAL_CSHARP_PROTOBUF_VERSION: ${ECAL_CSHARP_PROTOBUF_VERSION}")

add_subdirectory(Eclipse.eCAL.Core)
add_subdirectory(Eclipse.eCAL.Protobuf)
add_subdirectory(Eclipse.eCAL.Protobuf.Datatypes)
add_subdirectory(Eclipse.eCAL.String)

if (ECAL_CSHARP_BUILD_TESTS)
add_subdirectory(Eclipse.eCAL.Core.Test)
add_subdirectory(Eclipse.eCAL.Protobuf.Test)
endif()

if (ECAL_CSHARP_BUILD_SAMPLES)
enable_testing()
add_subdirectory(Eclipse.eCAL.Core.Samples)
add_subdirectory(Eclipse.eCAL.Protobuf.Samples)
add_subdirectory(Eclipse.eCAL.String.Samples)
endif ()

if (ECAL_CSHARP_BUILD_SAMPLES OR ECAL_CSHARP_BUILD_TESTS)
#Contains the person.proto which are being compiled to cs files
add_subdirectory(Eclipse.eCAL.Protobuf.Samples.Datatypes)
endif()

if(WIN32)
  set(CPACK_GENERATOR "External")
  set(CPACK_EXTERNAL_ENABLE_STAGING ON)
endif()
SET(CPACK_OUTPUT_FILE_PREFIX _deploy) # WARN: Undocumented, should use CPACK_PACKAGE_DIRECTORY
include(CPack)
