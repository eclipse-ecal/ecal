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

project(pb)

find_package(Protobuf REQUIRED)

set(ProtoFiles
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/pb/ecal.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/pb/host.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/pb/layer.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/pb/mma/mma.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/pb/monitoring.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/pb/play/service.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/pb/play/state.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/pb/process.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/pb/rec/client_service.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/pb/rec/client_state.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/pb/rec/server_config.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/pb/rec/server_service.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/pb/rec/server_state.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/pb/service.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/pb/sim_time.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/pb/sys/service.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/pb/sys/client_service.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/pb/sys/state.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ecal/pb/topic.proto
)

ecal_add_static_library(${PROJECT_NAME} src/ecalpb.cpp)
add_library(eCAL::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

protobuf_target_cpp(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src INSTALL_FOLDER include ${ProtoFiles})

target_compile_options(${PROJECT_NAME}
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/wd4505 /wd4592 /wd4189>
    $<$<CXX_COMPILER_ID:GNU>:-Wno-unused-parameter>)

set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${PROJECT_NAME} protobuf::libprotobuf)

ecal_install_static_library(${PROJECT_NAME})

if(BUILD_PY_BINDING)
  protobuf_generate_python_ext(python_sources ${PYTHON_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src ${ProtoFiles})
  target_sources(${PROJECT_NAME} PRIVATE ${python_sources})
  set_source_files_properties(${python_sources} PROPERTIES HEADER_FILE_ONLY TRUE)
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ecal/pb)
