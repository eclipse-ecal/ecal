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

ecal_find_package(Threads REQUIRED)
ecal_find_package(Protobuf REQUIRED)
ecal_find_package(spdlog REQUIRED)

set(PROJECT_NAME rec_core)

set(source_files
    include/rec_core/ecal_rec.h
    include/rec_core/ecal_rec_defs.h
    include/rec_core/ecal_rec_logger.h
    include/rec_core/job_config.h
    include/rec_core/recorder_state.h
    include/rec_core/record_mode.h
    include/rec_core/topic_info.h

    src/ecal_rec.cpp
    src/frame.h
    src/garbage_collector_trigger_thread.cpp
    src/garbage_collector_trigger_thread.h
    src/hdf5_writer_thread.cpp
    src/hdf5_writer_thread.h
    src/job_config.cpp
    src/monitoring_thread.cpp
    src/monitoring_thread.h
    src/recorder.cpp
    src/recorder.h
) 

add_library (${PROJECT_NAME} ${source_files})
add_library (eCAL::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_include_directories(${PROJECT_NAME} PUBLIC  include)

create_targets_protobuf()

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    spdlog::spdlog
)

target_link_libraries(${PROJECT_NAME}
 PUBLIC
    protobuf::libprotobuf
    eCAL::core
    eCAL::pb
  PRIVATE
    eCAL::hdf5
    ThreadingUtils
    Threads::Threads
    eCAL::utils
    EcalParser
)

if(MSVC)
    msvc_source_tree(
        ${source_files}
    )
endif(MSVC)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER app/rec)
