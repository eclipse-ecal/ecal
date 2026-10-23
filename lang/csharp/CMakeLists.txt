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
# Needs CMAKE_FIND_PACKAGE_PREFER_CONFIG=ON: Module mode defines a subset of the protobuf:: targets, which conflicts
# with the targets of the (newer) protobuf Config files loaded later on by eCALConfig.cmake.
find_package(Protobuf REQUIRED)
if (${Protobuf_VERSION} VERSION_GREATER_EQUAL 4)
  string(REPLACE "." ";" _ecal_protobuf_version "${Protobuf_VERSION}")
  list(GET _ecal_protobuf_version 0 _ecal_protobuf_version_first)
  if(_ecal_protobuf_version_first GREATER_EQUAL 10)
    # Config files of recent protobuf versions omit the leading runtime major version (e.g. 36.2.0 for v36.2)
    list(GET _ecal_protobuf_version 0 _ecal_protobuf_version_minor)
    list(GET _ecal_protobuf_version 1 _ecal_protobuf_version_patch)
  else()
    list(GET _ecal_protobuf_version 1 _ecal_protobuf_version_minor)
    list(GET _ecal_protobuf_version 2 _ecal_protobuf_version_patch)
  endif()
  set(Protobuf_VERSION_MINOR "${_ecal_protobuf_version_minor}")
  set(Protobuf_VERSION_PATCH "${_ecal_protobuf_version_patch}")
  if(Protobuf_VERSION_MINOR STREQUAL "NOTFOUND" OR Protobuf_VERSION_PATCH STREQUAL "NOTFOUND")
    message(FATAL_ERROR "Unable to determine Protobuf version")
  endif()
set(ECAL_CSHARP_PROTOBUF_VERSION "3.${Protobuf_VERSION_MINOR}.${Protobuf_VERSION_PATCH}")
else ()
set(ECAL_CSHARP_PROTOBUF_VERSION "${Protobuf_VERSION}")
endif ()
endmacro()

set(CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.1" CACHE STRING "Specify the .NET framework version to use.")

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
