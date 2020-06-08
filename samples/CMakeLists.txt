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

cmake_minimum_required (VERSION 3.5)
project(eCAL_samples VERSION "${ECAL_VERSION_STRING}")
set (CMAKE_CXX_STANDARD 14)

if(HAS_HDF5)
set(SAMPLE_PERSON_MEASUREMENT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/data/person)
endif(HAS_HDF5)

# --------------------------------------------------------
# c samples
# --------------------------------------------------------
add_subdirectory(c/minimal/minimal_rec)
add_subdirectory(c/minimal/minimal_rec_cb)
add_subdirectory(c/minimal/minimal_snd)
add_subdirectory(c/services/service_client)
add_subdirectory(c/services/service_server)

# --------------------------------------------------------
# cpp samples
# --------------------------------------------------------
if(HAS_CAPNPROTO)
add_subdirectory(cpp/capnp/addressbook_snd)
add_subdirectory(cpp/capnp/addressbook_rec)
add_subdirectory(cpp/capnp/addressbook_rec_cb)
add_subdirectory(cpp/capnp/addressbook_rec_dynamic)
endif(HAS_CAPNPROTO)
add_subdirectory(cpp/counter/counter_rec_cb)
add_subdirectory(cpp/counter/counter_snd)
if(HAS_HDF5)
add_subdirectory(cpp/measurement/measurement_read)
add_subdirectory(cpp/measurement/measurement_write)
endif(HAS_HDF5)
add_subdirectory(cpp/event/event_rec)
add_subdirectory(cpp/event/event_snd)
add_subdirectory(cpp/latency/latency_rec)
add_subdirectory(cpp/latency/latency_rec_cb)
add_subdirectory(cpp/latency/latency_snd)
add_subdirectory(cpp/minimal/minimal_rec)
add_subdirectory(cpp/minimal/minimal_snd)
add_subdirectory(cpp/misc/process)
add_subdirectory(cpp/misc/proto_dyn)
add_subdirectory(cpp/misc/proto_dyn_json)
add_subdirectory(cpp/misc/time)
add_subdirectory(cpp/misc/timer)
add_subdirectory(cpp/monitoring/monitoring_rec)
add_subdirectory(cpp/monitoring/monitoring_reg)
add_subdirectory(cpp/multiple/multiple_rec_cb)
add_subdirectory(cpp/multiple/multiple_snd)
add_subdirectory(cpp/performance/dynsize_snd)
add_subdirectory(cpp/performance/performance_rec)
add_subdirectory(cpp/performance/performance_rec_cb)
add_subdirectory(cpp/performance/performance_snd)
add_subdirectory(cpp/performance/pubsub_throughput)
add_subdirectory(cpp/person/person_rec)
add_subdirectory(cpp/person/person_rec_events)
add_subdirectory(cpp/person/person_snd)
add_subdirectory(cpp/person/person_snd_events)
add_subdirectory(cpp/person/person_snd_inproc)
add_subdirectory(cpp/person/person_snd_multicast)
add_subdirectory(cpp/pingpong/ping)
add_subdirectory(cpp/pingpong/pong)
add_subdirectory(cpp/services/ecalplayer_client)
if(HAS_QT5)
  add_subdirectory(cpp/services/ecalplayer_gui_client)
  add_subdirectory(cpp/services/rec_client_service_gui)
  add_subdirectory(cpp/services/rec_server_service_gui)
endif(HAS_QT5)
add_subdirectory(cpp/services/rec_client_service_cli)
add_subdirectory(cpp/services/math_client)
add_subdirectory(cpp/services/math_server)
add_subdirectory(cpp/services/ping_client)
add_subdirectory(cpp/services/ping_server)

# --------------------------------------------------------
# python samples
# --------------------------------------------------------
if(BUILD_PY_BINDING)
  if(WIN32)
    if(HAS_HDF5)
      add_subdirectory(python/ecalhdf5_rw)
    endif(HAS_HDF5)
    add_subdirectory(python/latency_rec)
    add_subdirectory(python/latency_rec_cb)
    add_subdirectory(python/latency_snd)
    if(HAS_HDF5)
      add_subdirectory(python/measurement_read)
    endif(HAS_HDF5)
    add_subdirectory(python/minimal_rec)
    add_subdirectory(python/minimal_rec_cb)
    add_subdirectory(python/minimal_service)
    add_subdirectory(python/minimal_snd)
    add_subdirectory(python/monitoring)
    add_subdirectory(python/person_rec)
    add_subdirectory(python/person_rec_cb)
    add_subdirectory(python/person_rec_json)  
    add_subdirectory(python/person_snd)
  endif(WIN32)
endif(BUILD_PY_BINDING)

# --------------------------------------------------------
# csharp samples
# --------------------------------------------------------
if(BUILD_CSHARP_BINDING)
  if(WIN32)
  # --------------------------------------------------------
  # samples csharp (not working curently, they should be create "AnyCPU" platform but they dont ..
  # --------------------------------------------------------
  add_subdirectory(csharp/minimal/minimal_rec)
  add_subdirectory(csharp/minimal/minimal_rec_cb)
  add_subdirectory(csharp/minimal/minimal_snd)
  add_subdirectory(csharp/person/protobuf)
  add_subdirectory(csharp/person/person_rec)
  add_subdirectory(csharp/person/person_rec_cb)
  add_subdirectory(csharp/person/person_rec_json_cb)
  add_subdirectory(csharp/person/person_snd)
  endif()
endif()
