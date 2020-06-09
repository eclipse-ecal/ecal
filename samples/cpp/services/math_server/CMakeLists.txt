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

project(math_server)

ecal_find_package(eCAL REQUIRED)
ecal_find_package(Protobuf REQUIRED)

set(math_server_src
    src/math_server.cpp
)

set(math_server_proto
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protobuf/math.proto
)

ecal_add_sample(${PROJECT_NAME} ${math_server_src})
PROTOBUF_TARGET_CPP(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/protobuf ${math_server_proto})

target_link_libraries(${PROJECT_NAME}
    eCAL::core
    protobuf::libprotobuf
)

ecal_install_sample(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER samples/cpp/services)
