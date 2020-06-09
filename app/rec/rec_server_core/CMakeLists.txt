# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2018 Continental Corporation
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

ecal_find_package(Threads REQUIRED)
ecal_find_package(Protobuf REQUIRED)
ecal_find_package(spdlog REQUIRED)

set(PROJECT_NAME rec_server_core)

set(source_files
    include/rec_server_core/rec_server.h

    src/rec_server.cpp
    src/recorder_command.h
    src/recorder_settings.h

    src/recorder/abstract_recorder.h
    src/recorder/local_recorder.cpp
    src/recorder/local_recorder.h
    src/recorder/remote_recorder.cpp
    src/recorder/remote_recorder.h
) 

add_library (${PROJECT_NAME} ${source_files})
add_library (eCAL::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_include_directories(${PROJECT_NAME} PUBLIC  include)

create_targets_protobuf()

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    protobuf::libprotobuf
    eCAL::core
    eCAL::pb
    ThreadingUtils
    Threads::Threads
    eCAL::utils
    EcalParser
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    eCAL::rec_core
)

if(MSVC)
    msvc_source_tree(
        ${source_files}
    )
endif(MSVC)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER app/rec)
