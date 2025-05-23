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

project(Eclipse.eCAL.Core)

find_package(eCAL REQUIRED)

set(ecal_clr_src
    AssemblyInfo.cpp
    include/config/clr_application.h
    include/config/clr_configuration.h
    include/config/clr_logging.h
    include/config/clr_publisher.h
    include/config/clr_registration.h
    include/config/clr_subscriber.h
    include/config/clr_time.h
    include/config/clr_transport_layer.h
    include/clr_common.h
    include/clr_config.h
    include/clr_core.h
    include/clr_init.h
    include/clr_log.h
    include/clr_monitoring.h
    include/clr_time.h
    include/clr_types.h
    include/pubsub/clr_publisher.h
    include/pubsub/clr_subscriber.h
    include/pubsub/clr_types.h
    include/service/clr_client.h
    include/service/clr_server.h
    include/service/clr_types.h
    src/clr_client.cpp
    src/clr_common.cpp
    src/clr_config.cpp
    src/clr_core.cpp
    src/clr_init.cpp
    src/clr_log.cpp
    src/clr_monitoring.cpp
    src/clr_publisher.cpp
    src/clr_server.cpp
    src/clr_subscriber.cpp
    src/clr_time.cpp

) 

add_library(${PROJECT_NAME} SHARED ${ecal_clr_src})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
)
target_link_libraries(${PROJECT_NAME} PRIVATE eCAL::core)
set_target_properties(${PROJECT_NAME} PROPERTIES
    COMMON_LANGUAGE_RUNTIME ""
)

set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

install(TARGETS ${PROJECT_NAME}
    # IMPORTANT: Add the library to the "export-set"
    EXPORT eCALTargets
    RUNTIME COMPONENT runtime
    LIBRARY COMPONENT sdk
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER lang/csharp)
