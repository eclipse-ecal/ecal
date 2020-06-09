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

project(addressbook_rec_cb)

ecal_find_package(CapnProto REQUIRED)
ecal_find_package(eCAL REQUIRED)

set(CAPNPC_IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
capnp_generate_cpp(CAPNP_SRCS CAPNP_HDRS src/addressbook.capnp)

ecal_add_sample(${PROJECT_NAME} src/addressbook_rec_cb.cpp ${CAPNP_SRCS} ${CAPNP_HDRS})

set_source_files_properties(${CAPNP_SRCS} PROPERTIES COMPILE_FLAGS $<$<CXX_COMPILER_ID:MSVC>:-W0>)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE CapnProto::capnp eCAL::core)
target_link_options(${PROJECT_NAME} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/ignore:4099>)

ecal_install_sample(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER samples/cpp/addressbook)