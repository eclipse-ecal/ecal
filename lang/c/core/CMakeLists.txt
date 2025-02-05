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
    src/client.cpp
    src/common.cpp
    src/common.h
    src/core.cpp
    src/log.cpp
    src/monitoring.cpp
    src/process.cpp
    src/publisher.cpp
    src/server.cpp
    src/subscriber.cpp
    src/time.cpp
    src/timer.cpp
    src/util.cpp    
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
    include/ecal_c/pubsub/publisher.h
    include/ecal_c/pubsub/subscriber.h
    include/ecal_c/service/client.h
    include/ecal_c/service/server.h
    include/ecal_c/service/service_info.h
    include/ecal_c/callback.h
    include/ecal_c/core.h
    include/ecal_c/ecal.h
    include/ecal_c/export.h
    include/ecal_c/init.h
    include/ecal_c/log.h
    include/ecal_c/monitoring.h
    include/ecal_c/process.h
    include/ecal_c/time.h
    include/ecal_c/timer.h
    include/ecal_c/tlayer.h
    include/ecal_c/types.h
    include/ecal_c/util.h
)

set(ecal_c_sources
    ${ecal_c_src}
    ${ecal_c_win_src}
    ${ecal_header_cimpl}
)

ecal_add_shared_library(${PROJECT_NAME}
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

ecal_install_shared_library(${PROJECT_NAME})

install(DIRECTORY
   "include/" DESTINATION "${eCAL_install_include_dir}" COMPONENT sdk
    FILES_MATCHING PATTERN "*.h")


if(NOT ${CMAKE_VERSION} VERSION_LESS "3.8.0") 
  source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES 
    ${ecal_c_sources}
  )
endif()