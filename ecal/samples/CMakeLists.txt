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
cmake_minimum_required(VERSION 3.10)

# --------------------------------------------------------
# cpp samples
# --------------------------------------------------------
# benchmarks
if(ECAL_CORE_PUBLISHER)
  add_subdirectory(cpp/benchmarks/counter_snd)
  add_subdirectory(cpp/benchmarks/dynsize_snd)
  add_subdirectory(cpp/benchmarks/many_connections_snd)
  add_subdirectory(cpp/benchmarks/multiple_snd)
  add_subdirectory(cpp/benchmarks/performance_snd)
  add_subdirectory(cpp/benchmarks/pubsub_throughput)
  if (ECAL_CORE_COMMAND_LINE)
    add_subdirectory(cpp/benchmarks/datarate_snd)
    add_subdirectory(cpp/benchmarks/latency_snd)
  endif()
endif()

if(ECAL_CORE_SUBSCRIBER)
  add_subdirectory(cpp/benchmarks/counter_rec)
  add_subdirectory(cpp/benchmarks/many_connections_rec)
  add_subdirectory(cpp/benchmarks/multiple_rec)
  add_subdirectory(cpp/benchmarks/performance_rec)
  if (ECAL_CORE_COMMAND_LINE)
    add_subdirectory(cpp/benchmarks/datarate_rec)
    add_subdirectory(cpp/benchmarks/latency_rec)
  endif()
endif()

if(ECAL_CORE_PUBLISHER AND ECAL_CORE_SUBSCRIBER)
  add_subdirectory(cpp/benchmarks/perftool)
  add_subdirectory(cpp/benchmarks/massive_pub_sub)
endif()

# misc
add_subdirectory(cpp/misc/process)
add_subdirectory(cpp/misc/time)
add_subdirectory(cpp/misc/config)
if(ECAL_CORE_PUBLISHER)
  add_subdirectory(cpp/misc/timer)
endif()

# monitoring
if(ECAL_CORE_MONITORING)
  add_subdirectory(cpp/monitoring/monitoring_get_services)
  add_subdirectory(cpp/monitoring/monitoring_get_topics)
  add_subdirectory(cpp/monitoring/monitoring_performance)
  if(ECAL_CORE_USE_PROTOBUF)
    add_subdirectory(cpp/monitoring/monitoring_rec)
  endif()
endif()

# orchestration
if(ECAL_CORE_PUBLISHER AND ECAL_CORE_SUBSCRIBER AND ECAL_CORE_SERVICE AND ECAL_CORE_USE_PROTOBUF)
  add_subdirectory(cpp/orchestration/component1)
  add_subdirectory(cpp/orchestration/component2)
  add_subdirectory(cpp/orchestration/orchestrator)
endif()

# pubsub
if(ECAL_CORE_PUBLISHER)
  add_subdirectory(cpp/pubsub/binary/binary_snd)
  add_subdirectory(cpp/pubsub/binary/binary_zero_copy_snd)
  add_subdirectory(cpp/pubsub/binary/ping)
endif()

if(ECAL_CORE_SUBSCRIBER)
  add_subdirectory(cpp/pubsub/binary/binary_rec)
  add_subdirectory(cpp/pubsub/binary/binary_zero_copy_rec)
  add_subdirectory(cpp/pubsub/binary/pong)
endif()

# services
if(ECAL_CORE_SERVICE)
  add_subdirectory(cpp/services/latency_client)
  add_subdirectory(cpp/services/latency_server)
  add_subdirectory(cpp/services/minimal_client)
  add_subdirectory(cpp/services/minimal_server)
endif()
