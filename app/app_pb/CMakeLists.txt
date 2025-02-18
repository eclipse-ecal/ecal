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

project(app_pb)

find_package(Protobuf REQUIRED)

set(ProtoFiles
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/app/pb/mma/mma.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/app/pb/play/service.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/app/pb/play/state.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/app/pb/rec/client_service.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/app/pb/rec/client_state.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/app/pb/rec/server_config.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/app/pb/rec/server_service.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/app/pb/rec/server_state.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/app/pb/sys/service.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/app/pb/sys/client_service.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/app/pb/sys/state.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/app/pb/sys/process.proto
)

# Compile statically on Windows and shared for all other systems. 
# 
# We compile shared on Linux etc., as we must only load the proto descriptors 
# once; otherwise, protobuf would throw an exception on runtime. A shared object 
# achieves that, as it is only loaded into memory once, even when being linked 
# against from different libraries.
# We don't have to do that on Windows, as we compile against protobuf statically 
# and therefore each .dll has it's own descriptor pool. Having a static lib here 
# also has the advantage that we need to export the symbols, which is default 
# disabled on Windows.
# 
# TODO: Having code like that probably isn't the best solution ever. We should 
# maybe always link against protobuf statically. Currently the reason why we 
# don't do that is, that the default Ubuntu libprotobuf.a doesn't contain 
# position independent code and can therefore not be statically compiled into a 
# shared object library.
if(WIN32)
  ecal_add_static_library(${PROJECT_NAME} src/app_pb.cpp)
else()
  ecal_add_shared_library(${PROJECT_NAME} src/app_pb.cpp)
endif()

add_library(eCAL::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

protobuf_target_cpp(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src
  INSTALL_FOLDER "${eCAL_install_include_dir}"
  ${ProtoFiles}
)

target_compile_options(${PROJECT_NAME}
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/wd4505 /wd4592 /wd4189>
    $<$<CXX_COMPILER_ID:GNU>:-Wno-unused-parameter>)

set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${PROJECT_NAME} PUBLIC protobuf::libprotobuf)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14) 

ecal_install_library(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER app/app_pb)
