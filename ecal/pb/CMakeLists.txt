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

# installing old, deprecated ecal core, app, time protobuf header for downward compatibility
# REMOVE ME IN ECAL6

project(pb)

set(ecal_pb_includes
  include/ecal/pb/ecal.pb.h
  include/ecal/pb/host.pb.h
  include/ecal/pb/layer.pb.h
  include/ecal/pb/monitoring.pb.h
  include/ecal/pb/process.pb.h
  include/ecal/pb/service.pb.h
  include/ecal/pb/sim_time.pb.h
  include/ecal/pb/topic.pb.h
  include/ecal/pb/mma/mma.pb.h
  include/ecal/pb/play/service.pb.h
  include/ecal/pb/play/state.pb.h
  include/ecal/pb/rec/client_service.pb.h
  include/ecal/pb/rec/client_state.pb.h
  include/ecal/pb/rec/server_config.pb.h
  include/ecal/pb/rec/server_service.pb.h
  include/ecal/pb/rec/server_state.pb.h
  include/ecal/pb/sys/client_service.pb.h
  include/ecal/pb/sys/service.pb.h
  include/ecal/pb/sys/state.pb.h
)

set(ecal_pb_src
    src/pb.cpp
)

ecal_add_static_library(${PROJECT_NAME}
    ${ecal_pb_includes}
    ${ecal_pb_src}
)

add_library(eCAL::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    eCAL::core_pb
)

if(BUILD_APPS)
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    eCAL::app_pb
)
endif()

if(BUILD_TIME)
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    eCAL::ecaltime_pb
)
endif()

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

install(DIRECTORY
   "include/" DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT sdk
    FILES_MATCHING PATTERN "*.pb.h")

ecal_install_library(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ecal/pb)
