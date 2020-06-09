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

ecal_find_package(Protobuf REQUIRED)

project(proto)

set(ecal_protobuf_src
    src/ecal_proto_decoder.cpp
    src/ecal_proto_dyn.cpp
    src/ecal_proto_maximum_array_dimensions.cpp
    src/ecal_proto_message_filter.cpp
    src/ecal_proto_visitor.cpp
)

set(ecal_protobuf_header
    include/ecal/protobuf/ecal_proto_decoder.h
    include/ecal/protobuf/ecal_proto_dyn.h
    include/ecal/protobuf/ecal_proto_hlp.h
    include/ecal/protobuf/ecal_proto_maximum_array_dimensions.h
    include/ecal/protobuf/ecal_proto_message_filter.h
    include/ecal/protobuf/ecal_proto_visitor.h
)

ecal_add_library(${PROJECT_NAME} ${ecal_protobuf_src} ${ecal_protobuf_header})
add_library(eCAL::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME} PUBLIC protobuf::libprotobuf)

ecal_install_library(${PROJECT_NAME})

install(
  FILES ${ecal_protobuf_header}
  DESTINATION "${INSTALL_INCLUDE_DIR}/ecal/protobuf"
  COMPONENT sdk
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER contrib)
