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

project(measurement)

###################################
# Base Measurement common headers #
###################################
ecal_add_interface_library(measurement_base)
add_library(eCAL::measurement_base ALIAS measurement_base)

target_sources(measurement_base
  INTERFACE
    FILE_SET measurement_base_headers
    TYPE HEADERS
    BASE_DIRS include
    FILES
      include/ecal/measurement/base/reader.h
      include/ecal/measurement/base/types.h
      include/ecal/measurement/base/writer.h
)

target_compile_features(measurement_base INTERFACE cxx_std_14)

install(
  TARGETS       measurement_base
  EXPORT        eCALCoreTargets  
  ARCHIVE       DESTINATION "${eCAL_install_archive_dir}" COMPONENT sdk
  LIBRARY       DESTINATION "${eCAL_install_lib_dir}"     COMPONENT sdk
  FILE_SET      measurement_base_headers COMPONENT sdk
)


###################################
# Measurement library             #
###################################
ecal_add_interface_library(measurement)
add_library(eCAL::measurement ALIAS measurement)

target_sources(measurement
  INTERFACE
    FILE_SET measurement_headers
    TYPE HEADERS
    BASE_DIRS include
    FILES
      include/ecal/measurement/measurement.h
      include/ecal/measurement/imeasurement.h
      include/ecal/measurement/omeasurement.h      
          
)

target_compile_features(measurement INTERFACE cxx_std_14)

install(
  TARGETS       measurement
  EXPORT        eCALCoreTargets  
  ARCHIVE       DESTINATION "${eCAL_install_archive_dir}" COMPONENT sdk
  LIBRARY       DESTINATION "${eCAL_install_lib_dir}"     COMPONENT sdk
  FILE_SET      measurement_headers COMPONENT sdk
)

target_link_libraries(measurement
  INTERFACE
    eCAL::measurement_hdf5
)


