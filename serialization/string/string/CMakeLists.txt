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


##########################
# String base functionality
##########################

# std::string core communication extension
##########################
add_library(string_base INTERFACE)
add_library(eCAL::string_base ALIAS string_base)

target_sources(string_base
  INTERFACE
    FILE_SET string_base_headers
    TYPE HEADERS
    BASE_DIRS include
    FILES
      include/ecal/msg/string/message.h   
)

target_compile_features(string_base INTERFACE cxx_std_14)

install(
  TARGETS string_base
  EXPORT eCALCoreTargets  
  ARCHIVE       DESTINATION "${eCAL_install_archive_dir}" COMPONENT sdk
  LIBRARY       DESTINATION "${eCAL_install_lib_dir}"     COMPONENT sdk
  FILE_SET string_base_headers COMPONENT sdk
)

##########################
# std::string core communication extension
##########################
add_library(string_core INTERFACE)
add_library(eCAL::string_core ALIAS string_core)

target_link_libraries(string_core 
  INTERFACE 
    eCAL::message_core
    eCAL::string_base
)

target_sources(string_core
  INTERFACE
    FILE_SET string_core_headers
    TYPE HEADERS
    BASE_DIRS include
    FILES
      include/ecal/msg/string/publisher.h
      include/ecal/msg/string/subscriber.h      
)

target_compile_features(string_core INTERFACE cxx_std_14)

install(
  TARGETS string_core
  EXPORT eCALCoreTargets  
  ARCHIVE       DESTINATION "${eCAL_install_archive_dir}" COMPONENT sdk
  LIBRARY       DESTINATION "${eCAL_install_lib_dir}"     COMPONENT sdk
  FILE_SET string_core_headers COMPONENT sdk
)