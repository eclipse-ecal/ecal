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

##########################
# Protobuf base functionality
##########################
find_package(Protobuf REQUIRED)
ecal_add_library(protobuf_base)
add_library(eCAL::protobuf_base ALIAS protobuf_base)

target_sources(protobuf_base
  PUBLIC
    FILE_SET protobuf_base_headers
    TYPE HEADERS
    BASE_DIRS include
    FILES
      include/ecal/msg/protobuf/ecal_proto_decoder.h
      include/ecal/msg/protobuf/ecal_proto_dyn.h
      include/ecal/msg/protobuf/ecal_proto_hlp.h
      include/ecal/msg/protobuf/ecal_proto_maximum_array_dimensions.h
      include/ecal/msg/protobuf/ecal_proto_message_filter.h
      include/ecal/msg/protobuf/ecal_proto_visitor.h
  PRIVATE
    src/ecal_proto_decoder.cpp
    src/ecal_proto_dyn.cpp
    src/ecal_proto_maximum_array_dimensions.cpp
    src/ecal_proto_message_filter.cpp
    src/ecal_proto_visitor.cpp
)

target_link_libraries(protobuf_base PUBLIC protobuf::libprotobuf)

target_compile_features(protobuf_base PUBLIC cxx_std_14)

install(
  TARGETS protobuf_base
  EXPORT eCALCoreTargets  
  ARCHIVE       DESTINATION "${eCAL_install_archive_dir}" COMPONENT sdk
  LIBRARY       DESTINATION "${eCAL_install_lib_dir}"     COMPONENT sdk
  FILE_SET protobuf_base_headers COMPONENT sdk
)

set_property(TARGET protobuf_base PROPERTY FOLDER core)

##########################
# Protobuf core communication extension
##########################
add_library(protobuf_core INTERFACE)
add_library(eCAL::protobuf_core ALIAS protobuf_core)

target_link_libraries(protobuf_core 
  INTERFACE 
    eCAL::protobuf_base
    eCAL::message_core
)

target_sources(protobuf_core
  INTERFACE
    FILE_SET proto_headers
    TYPE HEADERS
    BASE_DIRS include
    FILES
      include/ecal/msg/protobuf/client.h
      include/ecal/msg/protobuf/dynamic_json_subscriber.h
      include/ecal/msg/protobuf/dynamic_publisher.h
      include/ecal/msg/protobuf/dynamic_subscriber.h
      include/ecal/msg/protobuf/publisher.h
      include/ecal/msg/protobuf/server.h
      include/ecal/msg/protobuf/subscriber.h
)

target_compile_features(protobuf_core INTERFACE cxx_std_14)

install(
  TARGETS protobuf_core
  EXPORT eCALCoreTargets  
  ARCHIVE       DESTINATION "${eCAL_install_archive_dir}" COMPONENT sdk
  LIBRARY       DESTINATION "${eCAL_install_lib_dir}"     COMPONENT sdk
  FILE_SET proto_headers COMPONENT sdk
)
