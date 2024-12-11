# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2024 Continental Corporation
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
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/pb.h
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/pb_common.c
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/pb_common.h
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/pb_decode.c
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/pb_decode.h
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/pb_encode.c
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/pb_encode.h
)

set(nanopb_generated_src
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/ecal/core/pb/datatype.npb.c
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/ecal/core/pb/datatype.npb.h
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/ecal/core/pb/ecal.npb.c
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/ecal/core/pb/ecal.npb.h
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/ecal/core/pb/host.npb.c
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/ecal/core/pb/host.npb.h
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/ecal/core/pb/layer.npb.c
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/ecal/core/pb/layer.npb.h
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/ecal/core/pb/logging.npb.c
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/ecal/core/pb/logging.npb.h
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/ecal/core/pb/monitoring.npb.c
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/ecal/core/pb/monitoring.npb.h
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/ecal/core/pb/process.npb.c
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/ecal/core/pb/process.npb.h
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/ecal/core/pb/service.npb.c
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/ecal/core/pb/service.npb.h
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/ecal/core/pb/topic.npb.c
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/nanopb/ecal/core/pb/topic.npb.h
)

set(ecal_serialize_src
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/ecal_serialize_common.cpp
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/ecal_serialize_common.h
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/ecal_serialize_logging.cpp
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/ecal_serialize_logging.h
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/ecal_serialize_monitoring.cpp
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/ecal_serialize_monitoring.h
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/ecal_serialize_sample_payload.cpp
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/ecal_serialize_sample_payload.h
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/ecal_serialize_sample_registration.cpp
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/ecal_serialize_sample_registration.h
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/ecal_serialize_service.cpp
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/ecal_serialize_service.h
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/ecal_struct_sample_common.h
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/ecal_struct_sample_payload.h
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/ecal_struct_sample_registration.h
  ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/ecal_struct_service.h
)

set(unit_test_src
  src/common_generate.cpp
  src/common_generate.h
  src/logging_compare.cpp
  src/logging_compare.h
  src/logging_generate.cpp
  src/logging_generate.h
  src/logging_serialization_test.cpp
  src/monitoring_compare.cpp
  src/monitoring_compare.h
  src/monitoring_generate.cpp
  src/monitoring_generate.h
  src/monitoring_serialization_test.cpp
  src/parallel_serialization_test.cpp
  src/payload_compare.cpp
  src/payload_compare.h
  src/payload_generate.cpp
  src/payload_generate.h
  src/payload_serialization_test.cpp
  src/registration_generate.cpp
  src/registration_generate.h
  src/registration_serialization_test.cpp
  src/service_compare.cpp
  src/service_compare.h
  src/service_generate.cpp
  src/service_generate.h
  src/service_serialization_test.cpp
)

source_group("nanopb/lib"       FILES ${nanopb_lib_src})
source_group("nanopb/generated" FILES ${nanopb_generated_src})
source_group("ecal_serialize"   FILES ${ecal_serialize_src})

ecal_add_gtest(${PROJECT_NAME} ${nanopb_lib_src} ${nanopb_generated_src} ${ecal_serialize_src} ${unit_test_src})

target_include_directories(${PROJECT_NAME}
  PRIVATE 
    $<TARGET_PROPERTY:eCAL::core,INCLUDE_DIRECTORIES>
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
