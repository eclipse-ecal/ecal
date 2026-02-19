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

find_package(FlatBuffers REQUIRED)


##########################
# flatbuffers base functionality (serialization ...)
##########################
add_library(flatbuffers_base INTERFACE)
add_library(eCAL::flatbuffers_base ALIAS flatbuffers_base)

target_link_libraries(flatbuffers_base 
  INTERFACE 
    flatbuffers::flatbuffers
    eCAL::message_base
)

target_sources(flatbuffers_base
  INTERFACE
    FILE_SET flatbuffers_base_headers
    TYPE HEADERS
    BASE_DIRS include
    FILES
      include/ecal/msg/flatbuffers/serializer.h
)

target_compile_features(flatbuffers_base INTERFACE cxx_std_17)

install(
  TARGETS flatbuffers_base
  EXPORT eCALCoreTargets  
  FILE_SET flatbuffers_base_headers COMPONENT sdk
)

##########################
# FlatBuffers core communication extension
##########################
add_library(flatbuffers_core INTERFACE)
add_library(eCAL::flatbuffers_core ALIAS flatbuffers_core)

target_link_libraries(flatbuffers_core 
  INTERFACE 
    eCAL::flatbuffers_base
    eCAL::message_core
)

target_sources(flatbuffers_core
  INTERFACE
    FILE_SET flatbuffers_core_header
    TYPE HEADERS
    BASE_DIRS include
    FILES
      include/ecal/msg/flatbuffers/publisher.h
      include/ecal/msg/flatbuffers/subscriber.h  
)

target_compile_features(flatbuffers_core INTERFACE cxx_std_17)

install(
  TARGETS flatbuffers_core
  EXPORT eCALCoreTargets  
  ARCHIVE       DESTINATION "${eCAL_install_archive_dir}" COMPONENT sdk
  LIBRARY       DESTINATION "${eCAL_install_lib_dir}"     COMPONENT sdk
  FILE_SET flatbuffers_core_header COMPONENT sdk
)

##########################
# flatbuffers measurement extension
##########################
if(ECAL_USE_HDF5)
  add_library(flatbuffers_measurement INTERFACE)
  add_library(eCAL::flatbuffers_measurement ALIAS flatbuffers_measurement)

  target_link_libraries(flatbuffers_measurement 
    INTERFACE 
      eCAL::flatbuffers_base
      eCAL::message_measurement
  )

  target_sources(flatbuffers_measurement
    INTERFACE
      FILE_SET flatbuffers_measurement_headers
      TYPE HEADERS
      BASE_DIRS include
      FILES
        include/ecal/msg/flatbuffers/imeasurement.h
        include/ecal/msg/flatbuffers/omeasurement.h
  )

  target_compile_features(flatbuffers_measurement INTERFACE cxx_std_14)

  install(
    TARGETS flatbuffers_measurement
    EXPORT eCALCoreTargets  
    ARCHIVE       DESTINATION "${eCAL_install_archive_dir}" COMPONENT sdk
    LIBRARY       DESTINATION "${eCAL_install_lib_dir}"     COMPONENT sdk
    FILE_SET flatbuffers_measurement_headers COMPONENT sdk
  )
endif()
