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

ecal_find_package(eCAL REQUIRED)

ecal_find_package(Protobuf REQUIRED)
create_targets_protobuf()

set(PROJECT_NAME measurement_read)

set(measurement_read_src
    src/measurement_read.cpp
)

set(measurement_proto
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protobuf/animal.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protobuf/house.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protobuf/person.proto    
)
ecal_add_sample(${PROJECT_NAME} ${measurement_read_src})
PROTOBUF_TARGET_CPP(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/protobuf ${measurement_proto})
#target_include_directories(${PROJECT_NAME} PRIVATE .)
target_compile_definitions(${PROJECT_NAME} PRIVATE MEASUREMENT_PATH="${SAMPLE_PERSON_MEASUREMENT_PATH}")

target_link_libraries(${PROJECT_NAME} eCAL::hdf5 eCAL::proto)

target_link_libraries(${PROJECT_NAME} protobuf::libprotobuf)

ecal_install_sample(${PROJECT_NAME})

if(MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY LINK_FLAGS "/ignore:4098,4099")
endif(MSVC)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER samples/cpp/ecalhdf5)
