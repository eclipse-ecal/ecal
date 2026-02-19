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

find_package(CapnProto REQUIRED)

##########################
# Capnproto base functionality (serialization ...)
##########################
add_library(capnproto_base INTERFACE)
add_library(eCAL::capnproto_base ALIAS capnproto_base)

target_link_libraries(capnproto_base 
  INTERFACE 
    CapnProto::capnp
    eCAL::message_base
)

target_sources(capnproto_base
  INTERFACE
    FILE_SET capnproto_base_headers
    TYPE HEADERS
    BASE_DIRS include
    FILES
      include/ecal/msg/capnproto/helper.h
      include/ecal/msg/capnproto/serializer.h
)

target_compile_features(capnproto_base INTERFACE cxx_std_17)

install(
  TARGETS capnproto_base
  EXPORT eCALCoreTargets  
  FILE_SET capnproto_base_headers COMPONENT sdk
)


##########################
# Capnproto core communication extension
##########################
add_library(capnproto_core INTERFACE)
add_library(eCAL::capnproto_core ALIAS capnproto_core)

target_link_libraries(capnproto_core 
  INTERFACE 
    eCAL::capnproto_base
    eCAL::message_core
)

target_sources(capnproto_core
  INTERFACE
    FILE_SET capnproto_core_headers
    TYPE HEADERS
    BASE_DIRS include
    FILES
      include/ecal/msg/capnproto/dynamic.h
      include/ecal/msg/capnproto/publisher.h
      include/ecal/msg/capnproto/subscriber.h
)

target_compile_features(capnproto_core INTERFACE cxx_std_17)

install(
  TARGETS capnproto_core
  EXPORT eCALCoreTargets  
  ARCHIVE       DESTINATION "${eCAL_install_archive_dir}" COMPONENT sdk
  LIBRARY       DESTINATION "${eCAL_install_lib_dir}"     COMPONENT sdk
  FILE_SET capnproto_core_headers COMPONENT sdk
)

##########################
# Capnproto measurement extension
##########################
if(ECAL_USE_HDF5)
  add_library(capnproto_measurement INTERFACE)
  add_library(eCAL::capnproto_measurement ALIAS capnproto_measurement)

  target_link_libraries(capnproto_measurement 
    INTERFACE 
      eCAL::capnproto_base
      eCAL::message_measurement
  )

  target_sources(capnproto_measurement
    INTERFACE
      FILE_SET capnproto_measurement_headers
      TYPE HEADERS
      BASE_DIRS include
      FILES
        include/ecal/msg/capnproto/imeasurement.h
        include/ecal/msg/capnproto/omeasurement.h
  )

  target_compile_features(capnproto_measurement INTERFACE cxx_std_14)

  install(
    TARGETS capnproto_measurement
    EXPORT eCALCoreTargets  
    ARCHIVE       DESTINATION "${eCAL_install_archive_dir}" COMPONENT sdk
    LIBRARY       DESTINATION "${eCAL_install_lib_dir}"     COMPONENT sdk
    FILE_SET capnproto_measurement_headers COMPONENT sdk
  )
endif()
