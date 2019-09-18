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

# --------------------------------------------------------
# samples
# --------------------------------------------------------
add_subdirectory(c/minimal/minimal_rec/src)
add_subdirectory(c/minimal/minimal_rec_cb/src)
add_subdirectory(c/minimal/minimal_snd/src)
add_subdirectory(c/services/service_client/src)
add_subdirectory(c/services/service_server/src)
if (HAS_CAPNPROTO)
add_subdirectory(cpp/capnp/addressbook_snd/src)
add_subdirectory(cpp/capnp/addressbook_rec/src)
add_subdirectory(cpp/capnp/addressbook_rec_cb/src)
add_subdirectory(cpp/capnp/addressbook_rec_dynamic/src)
endif (HAS_CAPNPROTO)
add_subdirectory(cpp/counter/counter_rec_cb/src)
add_subdirectory(cpp/counter/counter_snd/src)
add_subdirectory(cpp/event/event_rec/src)
add_subdirectory(cpp/event/event_snd/src)
add_subdirectory(cpp/latency/latency_rec/src)
add_subdirectory(cpp/latency/latency_snd/src)
add_subdirectory(cpp/minimal/minimal_rec/src)
add_subdirectory(cpp/minimal/minimal_snd/src)
add_subdirectory(cpp/misc/process/src)
add_subdirectory(cpp/misc/proto_dyn/src)
add_subdirectory(cpp/misc/proto_dyn_json/src)
add_subdirectory(cpp/misc/time/src)
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
add_subdirectory(cpp/person/person_snd_lcm/src)
add_subdirectory(cpp/person/person_snd_multicast/src)
if (ECAL_LAYER_FASTRTPS)
add_subdirectory(cpp/person/person_snd_rtps/src)
endif (ECAL_LAYER_FASTRTPS)
add_subdirectory(cpp/person/person_snd_unicast/src)
add_subdirectory(cpp/pingpong/ping/src)
add_subdirectory(cpp/pingpong/pong/src)
add_subdirectory(cpp/services/math_client/src)
add_subdirectory(cpp/services/math_server/src)
add_subdirectory(cpp/services/ping_client/src)
add_subdirectory(cpp/services/ping_server/src)
