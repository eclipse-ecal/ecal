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
# c samples
# --------------------------------------------------------
# pubsub
add_subdirectory(c/pubsub/string/minimal_rec)
add_subdirectory(c/pubsub/string/minimal_rec_cb)
add_subdirectory(c/pubsub/string/minimal_snd)

# services
add_subdirectory(c/services/minimal_client_c)
add_subdirectory(c/services/minimal_server_c)

# --------------------------------------------------------
# cpp samples
# --------------------------------------------------------
# benchmarks
add_subdirectory(cpp/benchmarks/counter_rec_cb)
add_subdirectory(cpp/benchmarks/counter_snd)
add_subdirectory(cpp/benchmarks/datarate_rec)
add_subdirectory(cpp/benchmarks/datarate_snd)
add_subdirectory(cpp/benchmarks/dynsize_snd)
add_subdirectory(cpp/benchmarks/latency_rec)
add_subdirectory(cpp/benchmarks/latency_snd)
add_subdirectory(cpp/benchmarks/many_connections_rec)
add_subdirectory(cpp/benchmarks/many_connections_snd)
add_subdirectory(cpp/benchmarks/multiple_rec_cb)
add_subdirectory(cpp/benchmarks/multiple_snd)
add_subdirectory(cpp/benchmarks/performance_rec)
add_subdirectory(cpp/benchmarks/performance_rec_cb)
add_subdirectory(cpp/benchmarks/performance_snd)
add_subdirectory(cpp/benchmarks/pubsub_throughput)

# measurement
if(HAS_HDF5)
add_subdirectory(cpp/measurement/measurement_read)
add_subdirectory(cpp/measurement/measurement_write)
endif(HAS_HDF5)

# misc
add_subdirectory(cpp/misc/event_rec)
add_subdirectory(cpp/misc/event_snd)
add_subdirectory(cpp/misc/process)
add_subdirectory(cpp/misc/time)
add_subdirectory(cpp/misc/timer)

# monitoring
add_subdirectory(cpp/monitoring/monitoring_get_services)
add_subdirectory(cpp/monitoring/monitoring_get_topics)
add_subdirectory(cpp/monitoring/monitoring_performance)
add_subdirectory(cpp/monitoring/monitoring_rec)
add_subdirectory(cpp/monitoring/monitoring_reg)
add_subdirectory(cpp/orchestration/component1)
add_subdirectory(cpp/orchestration/component2)
add_subdirectory(cpp/orchestration/orchestrator)

# pubsub
add_subdirectory(cpp/pubsub/binary/binary_rec_cb)
add_subdirectory(cpp/pubsub/binary/binary_snd)
add_subdirectory(cpp/pubsub/binary/ping)
add_subdirectory(cpp/pubsub/binary/pong)
if(HAS_CAPNPROTO)
add_subdirectory(cpp/pubsub/capnp/addressbook_snd)
add_subdirectory(cpp/pubsub/capnp/addressbook_rec)
add_subdirectory(cpp/pubsub/capnp/addressbook_rec_cb)
add_subdirectory(cpp/pubsub/capnp/addressbook_rec_dynamic)
endif(HAS_CAPNPROTO)
if(HAS_FLATBUFFERS)
add_subdirectory(cpp/pubsub/flatbuffer/monster_rec)
add_subdirectory(cpp/pubsub/flatbuffer/monster_snd)
endif(HAS_FLATBUFFERS)
#add_subdirectory(cpp/pubsub/msgpack/address_rec)
#add_subdirectory(cpp/pubsub/msgpack/address_snd)
add_subdirectory(cpp/pubsub/protobuf/person_rec)
add_subdirectory(cpp/pubsub/protobuf/person_rec_events)
add_subdirectory(cpp/pubsub/protobuf/person_rec_lambda_in_class)
add_subdirectory(cpp/pubsub/protobuf/person_snd)
add_subdirectory(cpp/pubsub/protobuf/person_snd_dyn)
add_subdirectory(cpp/pubsub/protobuf/person_snd_events)
add_subdirectory(cpp/pubsub/protobuf/person_snd_inproc)
add_subdirectory(cpp/pubsub/protobuf/person_snd_multicast)
add_subdirectory(cpp/pubsub/protobuf/person_snd_tcp)
add_subdirectory(cpp/pubsub/protobuf/proto_dyn)
add_subdirectory(cpp/pubsub/protobuf/proto_dyn_json)
add_subdirectory(cpp/pubsub/string/minimal_rec)
add_subdirectory(cpp/pubsub/string/minimal_rec_cb)
add_subdirectory(cpp/pubsub/string/minimal_snd)

# services
add_subdirectory(cpp/services/ecalplayer_client)
if(HAS_QT5)
  add_subdirectory(cpp/services/ecalplayer_gui_client)
endif(HAS_QT5)
add_subdirectory(cpp/services/ecalsys_client)
add_subdirectory(cpp/services/latency_client)
add_subdirectory(cpp/services/latency_server)
add_subdirectory(cpp/services/math_client)
add_subdirectory(cpp/services/math_server)
add_subdirectory(cpp/services/minimal_client)
add_subdirectory(cpp/services/minimal_server)
add_subdirectory(cpp/services/ping_client)
add_subdirectory(cpp/services/ping_client_dyn)
add_subdirectory(cpp/services/ping_server)
add_subdirectory(cpp/services/player_stepper)
add_subdirectory(cpp/services/rec_client_service_cli)
if(HAS_QT5)
  add_subdirectory(cpp/services/rec_client_service_gui)
  add_subdirectory(cpp/services/rec_server_service_gui)
endif(HAS_QT5)

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
    add_subdirectory(python/pubsub/protobuf/person_rec_json)  
    add_subdirectory(python/pubsub/protobuf/person_snd)
    add_subdirectory(python/pubsub/string/minimal_rec)
    add_subdirectory(python/pubsub/string/minimal_rec_cb)
    add_subdirectory(python/pubsub/string/minimal_snd)

    # services
    add_subdirectory(python/services/minimal_service)
  endif(WIN32)
endif(BUILD_PY_BINDING)
