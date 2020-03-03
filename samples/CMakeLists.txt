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
add_subdirectory(c/minimal/minimal_rec/src)
add_subdirectory(c/minimal/minimal_rec_cb/src)
add_subdirectory(c/minimal/minimal_snd/src)
add_subdirectory(c/services/service_client/src)
add_subdirectory(c/services/service_server/src)

# --------------------------------------------------------
# cpp samples
# --------------------------------------------------------
if(HAS_CAPNPROTO)
add_subdirectory(cpp/capnp/addressbook_snd/src)
add_subdirectory(cpp/capnp/addressbook_rec/src)
add_subdirectory(cpp/capnp/addressbook_rec_cb/src)
add_subdirectory(cpp/capnp/addressbook_rec_dynamic/src)
endif(HAS_CAPNPROTO)
add_subdirectory(cpp/counter/counter_rec_cb/src)
add_subdirectory(cpp/counter/counter_snd/src)
if(HAS_HDF5)
add_subdirectory(cpp/measurement/measurement_read/src)
add_subdirectory(cpp/measurement/measurement_write/src)
endif(HAS_HDF5)
add_subdirectory(cpp/event/event_rec/src)
add_subdirectory(cpp/event/event_snd/src)
add_subdirectory(cpp/latency/latency_rec/src)
add_subdirectory(cpp/latency/latency_rec_cb/src)
add_subdirectory(cpp/latency/latency_snd/src)
add_subdirectory(cpp/minimal/minimal_rec/src)
add_subdirectory(cpp/minimal/minimal_snd/src)
add_subdirectory(cpp/misc/process/src)
add_subdirectory(cpp/misc/proto_dyn/src)
add_subdirectory(cpp/misc/proto_dyn_json/src)
add_subdirectory(cpp/misc/time/src)
add_subdirectory(cpp/misc/timer/src)
add_subdirectory(cpp/monitoring/monitoring_rec/src)
add_subdirectory(cpp/monitoring/monitoring_reg/src)
add_subdirectory(cpp/multiple/multiple_rec_cb/src)
add_subdirectory(cpp/multiple/multiple_snd/src)
add_subdirectory(cpp/performance/dynsize_snd/src)
add_subdirectory(cpp/performance/performance_rec/src)
add_subdirectory(cpp/performance/performance_rec_cb/src)
add_subdirectory(cpp/performance/performance_snd/src)
add_subdirectory(cpp/performance/pubsub_throughput/src)
add_subdirectory(cpp/person/person_rec/src)
add_subdirectory(cpp/person/person_rec_events/src)
add_subdirectory(cpp/person/person_snd/src)
add_subdirectory(cpp/person/person_snd_events/src)
add_subdirectory(cpp/person/person_snd_inproc/src)
add_subdirectory(cpp/person/person_snd_multicast/src)
add_subdirectory(cpp/pingpong/ping/src)
add_subdirectory(cpp/pingpong/pong/src)
add_subdirectory(cpp/services/ecalplayer_client/src)
if(HAS_QT5)
  add_subdirectory(cpp/services/ecalplayer_gui_client)
endif(HAS_QT5)
add_subdirectory(cpp/services/math_client/src)
add_subdirectory(cpp/services/math_server/src)
add_subdirectory(cpp/services/ping_client/src)
add_subdirectory(cpp/services/ping_server/src)

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
  #add_subdirectory(csharp/person/protobuf)
  #add_subdirectory(csharp/person/person_rec)
  #add_subdirectory(csharp/person/person_rec_cb)
  #add_subdirectory(csharp/person/person_snd)
  endif()
endif()
