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

project(measurement_factory)

add_library(${PROJECT_NAME})
add_library(eCAL::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
  PRIVATE
    include/ecal/measurement/writer_factory.h
    src/writer_factory.cpp
    
)

target_include_directories(${PROJECT_NAME} PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    eCAL::measurement_base
  PRIVATE
    eCAL::hdf5
    eCAL::measurement_mcap
)

target_compile_definitions(${PROJECT_NAME} 
  PUBLIC
    $<$<BOOL:${HAS_HDF5}>:ECAL_MEASUREMENT_SUPPORTS_HDF5>
    $<$<BOOL:${HAS_MCAP}>:ECAL_MEASUREMENT_SUPPORTS_MCAP>
)

ecal_install_library(${PROJECT_NAME})

install(DIRECTORY
   "include/" DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT sdk
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER contrib/measurement)


