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

project(core_c VERSION ${eCAL_VERSION_STRING})


######################################
# c interface
######################################
set(ecal_c_src
    src/ecal_client_cimpl.cpp
    src/ecal_common_cimpl.cpp
    src/ecal_common_cimpl.h
    src/ecal_core_cimpl.cpp
    src/ecal_log_cimpl.cpp
    src/ecal_monitoring_cimpl.cpp
    src/ecal_process_cimpl.cpp
    src/ecal_publisher_cimpl.cpp
    src/ecal_server_cimpl.cpp
    src/ecal_subscriber_cimpl.cpp
    src/ecal_time_cimpl.cpp
    src/ecal_timer_cimpl.cpp
    src/ecal_util_cimpl.cpp    
)

######################################
# windows dll
######################################
if(WIN32)
  set(ecal_c_win_src
      src/win32/dll/dllmain.cpp
      src/win32/dll/ecal.rc
  )
endif()

######################################
# public header
######################################


set(ecal_header_cimpl
    include/ecal/cimpl/ecal_callback_cimpl.h
    include/ecal/cimpl/ecal_client_cimpl.h
    include/ecal/cimpl/ecal_core_cimpl.h
    include/ecal/cimpl/ecal_init_cimpl.h
    include/ecal/cimpl/ecal_log_cimpl.h
    include/ecal/cimpl/ecal_monitoring_cimpl.h
    include/ecal/cimpl/ecal_process_cimpl.h
    include/ecal/cimpl/ecal_publisher_cimpl.h
    include/ecal/cimpl/ecal_server_cimpl.h
    include/ecal/cimpl/ecal_service_info_cimpl.h
    include/ecal/cimpl/ecal_subscriber_cimpl.h
    include/ecal/cimpl/ecal_time_cimpl.h
    include/ecal/cimpl/ecal_timer_cimpl.h
    include/ecal/cimpl/ecal_util_cimpl.h
    include/ecal/ecalc.h
    include/ecal/ecalc_export.h
    include/ecal/ecalc_types.h
)


set(ecal_c_sources
    ${ecal_c_src}
    ${ecal_c_win_src}
    ${ecal_header_cimpl}
)

ecal_add_ecal_shared_library(${PROJECT_NAME}
    ${ecal_c_sources}
)

add_library(eCAL::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME} PRIVATE eCAL::core)

target_compile_definitions(${PROJECT_NAME}
  INTERFACE 
    ECAL_CORE_C_IMPORTS
  PRIVATE 
    ECAL_CORE_C_EXPORTS
    ECAL_NO_DEPRECATION_WARNINGS
)

set_target_properties(${PROJECT_NAME}
  PROPERTIES 
    CXX_VISIBILITY_PRESET hidden
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)


set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER lang/c)

ecal_install_ecal_shared_library(${PROJECT_NAME})

install(DIRECTORY
   "include/" DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT sdk
    FILES_MATCHING PATTERN "*.h")


if(NOT ${CMAKE_VERSION} VERSION_LESS "3.8.0") 
  source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES 
    ${ecal_c_sources}
  )
endif()