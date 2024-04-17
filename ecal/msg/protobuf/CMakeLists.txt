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

find_package(Protobuf REQUIRED)

project(proto)

ecal_add_library(proto)
add_library(eCAL::proto ALIAS proto)

target_sources(proto
  PUBLIC
    FILE_SET proto_headers
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

target_link_libraries(proto PUBLIC protobuf::libprotobuf)

target_compile_features(proto PUBLIC cxx_std_14)

install(
  TARGETS proto
  EXPORT eCALCoreTargets  
  ARCHIVE       DESTINATION "${eCAL_install_archive_dir}" COMPONENT sdk
  LIBRARY       DESTINATION "${eCAL_install_lib_dir}"     COMPONENT sdk
  FILE_SET proto_headers COMPONENT sdk
)

set_property(TARGET proto PROPERTY FOLDER core)
