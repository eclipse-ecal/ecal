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
cmake_minimum_required(VERSION 3.10)

project(eCAL_samples VERSION "${ECAL_VERSION_STRING}")

if(HAS_HDF5)
set(SAMPLE_PERSON_MEASUREMENT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/data/person)
endif(HAS_HDF5)

# --------------------------------------------------------
# cpp samples
# --------------------------------------------------------
# measurement
if(HAS_HDF5)
add_subdirectory(cpp/measurement/benchmark)
add_subdirectory(cpp/measurement/measurement_read)
add_subdirectory(cpp/measurement/measurement_write)
endif(HAS_HDF5)

# services
add_subdirectory(cpp/services/ecalplayer_client)
if(HAS_QT)
  add_subdirectory(cpp/services/ecalplayer_gui_client)
endif(HAS_QT)
add_subdirectory(cpp/services/ecalsys_client)
add_subdirectory(cpp/services/player_stepper)
add_subdirectory(cpp/services/rec_client_service_cli)
if(HAS_QT)
  add_subdirectory(cpp/services/rec_client_service_gui)
  add_subdirectory(cpp/services/rec_server_service_gui)
endif(HAS_QT)

# --------------------------------------------------------
# python samples
# --------------------------------------------------------
if(BUILD_PY_BINDING)
  if(WIN32)
    # benchmarks
    add_subdirectory(python/benchmarks/latency_rec)
    add_subdirectory(python/benchmarks/latency_rec_cb)
    add_subdirectory(python/benchmarks/latency_snd)

    # measurement
    if(HAS_HDF5)
      add_subdirectory(python/measurement/ecalhdf5_rw)
      add_subdirectory(python/measurement/ecalhdf5_protobuf_rw)
      add_subdirectory(python/measurement/measurement_read)
    endif(HAS_HDF5)

    # monitoring
    add_subdirectory(python/monitoring/monitoring)

    # pubsub
    add_subdirectory(python/pubsub/protobuf/person_rec)
    add_subdirectory(python/pubsub/protobuf/person_rec_cb)
    add_subdirectory(python/pubsub/protobuf/person_snd)
    add_subdirectory(python/pubsub/string/minimal_rec)
    add_subdirectory(python/pubsub/string/minimal_rec_cb)
    add_subdirectory(python/pubsub/string/minimal_snd)

    # services
    add_subdirectory(python/services/minimal_service)
  endif(WIN32)
endif(BUILD_PY_BINDING)
