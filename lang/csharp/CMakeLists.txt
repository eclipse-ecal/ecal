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
project(ecal_csharp)

include(GNUInstallDirs)

# Macro to determine the matching protobuf C# version compatible with protoc found.
# We should do this differently, but then we need to install protoc via nuget or similar...
# Because the Prototbuf Version used for C# bindings is independent from the one we get from the submodules / conan
macro(get_csharp_protobuf_version)
find_package(Protobuf REQUIRED)
if (${Protobuf_VERSION_MAJOR} GREATER 3)
set(ECAL_CSHARP_PROTOBUF_VERSION "3.${Protobuf_VERSION_MAJOR}.${Protobuf_VERSION_MINOR}")
else ()
set(ECAL_CSHARP_PROTOBUF_VERSION "${Protobuf_VERSION}")
endif ()
endmacro()

option(ECAL_CSHARP_BUILD_SAMPLES "BUILD eCAL C# samples" ON)
option(ECAL_CSHARP_BUILD_TESTS   "BUILD eCAL C# unittests" ON)

get_csharp_protobuf_version()
message(STATUS "Using ECAL_CSHARP_PROTOBUF_VERSION: ${ECAL_CSHARP_PROTOBUF_VERSION}")

add_subdirectory(Continental.eCAL.Core)
add_subdirectory(Continental.eCAL.Protobuf)
if (ECAL_CSHARP_BUILD_TESTS)
add_subdirectory(Continental.eCAL.Core.Test)
add_subdirectory(Continental.eCAL.Protobuf.Test)
endif()

if (ECAL_CSHARP_BUILD_SAMPLES)
add_subdirectory(Continental.eCAL.Core.Samples)
add_subdirectory(Continental.eCAL.Protobuf.Samples)
endif ()

if (ECAL_CSHARP_BUILD_SAMPLES OR ECAL_CSHARP_BUILD_TESTS)
#Contains the person.proto which are being compiled to cs files
add_subdirectory(Continental.eCAL.Protobuf.Samples.Datatypes)
endif()
