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

cmake_minimum_required(VERSION 3.13)

project(eCAL_Protobuf)

option(ECAL_PROTOBUF_BUILD_SAMPLES "BUILD eCAL Protobuf samples"   ON)
option(ECAL_PROTOBUF_BUILD_TESTS   "BUILD eCAL Protobuf unittests" ON)

# --------------------------------------------------------
# main extension functionality
# --------------------------------------------------------
add_subdirectory(protobuf)

# --------------------------------------------------------
# samples
# --------------------------------------------------------
if(ECAL_PROTOBUF_BUILD_SAMPLES)
  add_subdirectory(samples/pubsub/person_events_rec)
  add_subdirectory(samples/pubsub/person_rec)
  add_subdirectory(samples/pubsub/proto_dyn_json_rec)
  add_subdirectory(samples/pubsub/proto_dyn_rec)
  add_subdirectory(samples/pubsub/proto_dyn_snd)
  add_subdirectory(samples/pubsub/person_loopback)
  add_subdirectory(samples/pubsub/person_events_snd)
  add_subdirectory(samples/pubsub/person_snd)  
  add_subdirectory(samples/pubsub/person_snd_udp)
  add_subdirectory(samples/pubsub/person_snd_tcp)    
  add_subdirectory(samples/services/math_client)
  add_subdirectory(samples/services/math_server)
  add_subdirectory(samples/services/ping_client)
  add_subdirectory(samples/services/ping_client_dyn)
  add_subdirectory(samples/services/ping_server)  
endif()

# --------------------------------------------------------
# tests
# --------------------------------------------------------
if(ECAL_PROTOBUF_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests/clientserver_proto_test)
  add_subdirectory(tests/dynproto_test)
  add_subdirectory(tests/ecal_proto_test)
  add_subdirectory(tests/pubsub_proto_test)
endif()