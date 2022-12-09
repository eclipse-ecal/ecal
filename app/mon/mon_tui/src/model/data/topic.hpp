/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/
#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>

struct Topic
{
  enum class QOSReliability
  {
    BEST_EFFORT, RELIABLE
  };

  enum class QOSHistoryKind
  {
    KEEP_LAST, KEEP_ALL
  };

  enum class TransportLayer
  {
    NONE, UDP_MC, SHM, TCP, INPROC, ALL
  };

  enum class Direction
  {
    PUBLISHER, SUBSCRIBER
  };

  int32_t registration_clock;
  int32_t host_id;
  std::string host_name;
  int32_t pid;
  std::string process_name;
  std::string unit_name;
  std::string id;
  std::string name;
  Direction direction;
  std::string type;
  std::string type_descriptor;
  QOSHistoryKind history_kind;
  int32_t history_depth;
  QOSReliability reliability;
  std::vector<TransportLayer> transport_layers;
  int32_t size;
  int32_t local_connections_count;
  int32_t external_connections_count;
  int32_t message_drops;
  int64_t data_id;
  int64_t data_clock;
  int32_t data_frequency;
  std::map<std::string, std::string> attributes;
};
