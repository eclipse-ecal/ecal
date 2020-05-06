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

# version of the nuget Google.Protobuf package
set(google_protobuf_version 3.11.4)

# find nuget (install the latest CLI here: https://www.nuget.org/downloads).
find_program(NUGET nuget HINTS $ENV{NUGET_DIR})
if(NOT NUGET)
  message(FATAL_ERROR "CMake could not find the nuget command line tool. Please install it! (https://www.nuget.org/downloads)")
else()
  message(STATUS "Found nuget in ${NUGET}")
  # install google protobuf nuget package
  add_custom_command(TARGET ${PROJECT_NAME}
    PRE_BUILD
    COMMAND ${NUGET} install Google.Protobuf -Version ${google_protobuf_version}
  )
endif()

set(nuget_package_path
  "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}"
)
set(nuget_package_google
  "${nuget_package_path}/Google.Protobuf.${google_protobuf_version}/lib/net45/Google.Protobuf.dll"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
  VS_GLOBAL_ROOTNAMESPACE ${PROJECT_NAME}
  VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.6.1"
  VS_DOTNET_REFERENCE_Google.Protobuf ${nuget_package_google}
)

install(TARGETS ${PROJECT_NAME}
    # IMPORTANT: Add the library to the "export-set"
    EXPORT eCALTargets
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT runtime
    LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT sdk
    PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/ecal" COMPONENT sdk
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER lang/csharp)
