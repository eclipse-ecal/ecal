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
cmake_minimum_required(VERSION 3.23)

add_library(types INTERFACE)
add_library(eCAL::types ALIAS types)

target_include_directories(types 
  INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:${eCAL_install_include_dir}>
)

target_sources(types
  INTERFACE
    FILE_SET ecal_types_headers
    TYPE HEADERS
    BASE_DIRS include
    FILES
      ${CMAKE_CURRENT_LIST_DIR}/include/ecal/cimpl/ecal_callback_cimpl.h
      ${CMAKE_CURRENT_LIST_DIR}/include/ecal/cimpl/ecal_service_info_cimpl.h
      ${CMAKE_CURRENT_LIST_DIR}/include/ecal/ecal_callback.h
      ${CMAKE_CURRENT_LIST_DIR}/include/ecal/ecal_eventhandle.h
      ${CMAKE_CURRENT_LIST_DIR}/include/ecal/ecal_log_level.h
      ${CMAKE_CURRENT_LIST_DIR}/include/ecal/ecal_process_mode.h
      ${CMAKE_CURRENT_LIST_DIR}/include/ecal/ecal_process_severity.h
      ${CMAKE_CURRENT_LIST_DIR}/include/ecal/ecal_service_info.h
      ${CMAKE_CURRENT_LIST_DIR}/include/ecal/ecal_tlayer.h
      ${CMAKE_CURRENT_LIST_DIR}/include/ecal/types/topic_information.h
)

install(TARGETS types
  # IMPORTANT: Add the library to the "export-set"
  EXPORT eCALCoreTargets
  ARCHIVE                     DESTINATION "${eCAL_install_archive_dir}" COMPONENT sdk  
  FILE_SET ecal_types_headers DESTINATION "${eCAL_install_include_dir}" COMPONENT sdk
)