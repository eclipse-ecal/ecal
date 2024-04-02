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

project(test_serialization)

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

set(nanopb_lib_src
  ../../../core/src/serialization/nanopb/nanopb/pb.h
  ../../../core/src/serialization/nanopb/nanopb/pb_common.c
  ../../../core/src/serialization/nanopb/nanopb/pb_common.h
  ../../../core/src/serialization/nanopb/nanopb/pb_decode.c
  ../../../core/src/serialization/nanopb/nanopb/pb_decode.h
  ../../../core/src/serialization/nanopb/nanopb/pb_encode.c
  ../../../core/src/serialization/nanopb/nanopb/pb_encode.h
)

set(nanopb_generated_src
  ../../../core/src/serialization/nanopb/ecal.pb.c
  ../../../core/src/serialization/nanopb/ecal.pb.h
  ../../../core/src/serialization/nanopb/host.pb.c
  ../../../core/src/serialization/nanopb/host.pb.h
  ../../../core/src/serialization/nanopb/layer.pb.c
  ../../../core/src/serialization/nanopb/layer.pb.h
  ../../../core/src/serialization/nanopb/logging.pb.c
  ../../../core/src/serialization/nanopb/logging.pb.h
  ../../../core/src/serialization/nanopb/monitoring.pb.c
  ../../../core/src/serialization/nanopb/monitoring.pb.h
  ../../../core/src/serialization/nanopb/process.pb.c
  ../../../core/src/serialization/nanopb/process.pb.h
  ../../../core/src/serialization/nanopb/service.pb.c
  ../../../core/src/serialization/nanopb/service.pb.h
  ../../../core/src/serialization/nanopb/topic.pb.c
  ../../../core/src/serialization/nanopb/topic.pb.h
)

set(ecal_serialize_src
  ../../../core/src/serialization/ecal_serialize_common.cpp
  ../../../core/src/serialization/ecal_serialize_common.h
  ../../../core/src/serialization/ecal_serialize_logging.cpp
  ../../../core/src/serialization/ecal_serialize_logging.h
  ../../../core/src/serialization/ecal_serialize_monitoring.cpp
  ../../../core/src/serialization/ecal_serialize_monitoring.h
  ../../../core/src/serialization/ecal_serialize_sample_payload.cpp
  ../../../core/src/serialization/ecal_serialize_sample_payload.h
  ../../../core/src/serialization/ecal_serialize_sample_registration.cpp
  ../../../core/src/serialization/ecal_serialize_sample_registration.h
  ../../../core/src/serialization/ecal_serialize_service.cpp
  ../../../core/src/serialization/ecal_serialize_service.h
  ../../../core/src/serialization/ecal_struct_sample_common.h
  ../../../core/src/serialization/ecal_struct_sample_payload.h
  ../../../core/src/serialization/ecal_struct_sample_registration.h
  ../../../core/src/serialization/ecal_struct_service.h
)

set(unit_test_src
  src/common_generate.cpp
  src/logging_compare.cpp
  src/logging_generate.cpp
  src/logging_serialization_test.cpp
  src/monitoring_compare.cpp
  src/monitoring_generate.cpp
  src/monitoring_serialization_test.cpp
  src/payload_compare.cpp
  src/payload_generate.cpp
  src/payload_serialization_test.cpp
  src/registration_compare.cpp
  src/registration_generate.cpp
  src/registration_serialization_test.cpp
  src/service_compare.cpp
  src/service_generate.cpp
  src/service_serialization_test.cpp
)

source_group("nanopb/lib"       FILES ${nanopb_lib_src})
source_group("nanopb/generated" FILES ${nanopb_generated_src})
source_group("ecal_serialize"   FILES ${ecal_serialize_src})

ecal_add_gtest(${PROJECT_NAME} ${nanopb_lib_src} ${nanopb_generated_src} ${ecal_serialize_src} ${unit_test_src})

target_include_directories(${PROJECT_NAME}
  PRIVATE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../core/src/serialization/nanopb/nanopb>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../core/src/serialization/nanopb>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../core/include>
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Threads::Threads)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

ecal_install_gtest(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER tests/cpp/core)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES 
    ${${PROJECT_NAME}_src}
)
