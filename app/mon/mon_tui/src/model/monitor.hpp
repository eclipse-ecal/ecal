/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include <chrono>

#include <ecal/ecal.h>
#include <ecal/monitoring.h>
#include <ecal/process_severity.h>

#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "model/data/host.hpp"
#include "model/data/process.hpp"
#include "model/data/service.hpp"
#include "model/data/topic.hpp"

using ModelUpdateCallbackT = std::function<void()>;

class MonitorModel
{
  int update_time_ = 1000;
  bool is_polling;
  eCAL::Monitoring::SMonitoring mon_;

  std::vector<Host> hosts;
  std::vector<Process> processes;
  std::vector<Service> services;
  std::vector<Topic> topics;

  std::vector<ModelUpdateCallbackT> callbacks_;
  std::thread update_thread;
  mutable std::mutex mtx;
  mutable std::mutex callback_mtx;

  Process::Severity Severity(int32_t severity)
  {
    switch(static_cast<eCAL::Process::eSeverity>(severity))
    {
    case eCAL::Process::eSeverity::healthy:
      return Process::Severity::HEALTHY;
    case eCAL::Process::eSeverity::warning:
      return Process::Severity::WARNING;
    case eCAL::Process::eSeverity::critical:
      return Process::Severity::CRITICAL;
    case eCAL::Process::eSeverity::failed:
      return Process::Severity::FAILED;
    case eCAL::Process::eSeverity::unknown:
    default:
      return Process::Severity::UNKNOWN;
    }
  }

  Process::SeverityLevel SeverityLevel(int32_t level)
  {
    switch(static_cast<eCAL::Process::eSeverityLevel>(level))
    {
    case eCAL::Process::eSeverityLevel::level1:
      return Process::SeverityLevel::LEVEL_1;
    case eCAL::Process::eSeverityLevel::level2:
      return Process::SeverityLevel::LEVEL_2;
    case eCAL::Process::eSeverityLevel::level3:
      return Process::SeverityLevel::LEVEL_3;
    case eCAL::Process::eSeverityLevel::level4:
      return Process::SeverityLevel::LEVEL_4;
    case eCAL::Process::eSeverityLevel::level5:
      return Process::SeverityLevel::LEVEL_5;
    default:
      return Process::SeverityLevel::UNKNOWN;
    }
  }

  Process::TimeSyncState TimeSyncState(int32_t state)
  {
    switch(state)
    {
    case 1:
      return Process::TimeSyncState::REALTIME;
    case 2:
      return Process::TimeSyncState::REPLAY;
    case 0:
    default:
      return Process::TimeSyncState::NONE;
    }
  }

  Topic::TransportLayer TopicTransportLayer(eCAL::Monitoring::eTransportLayerType layer)
  {
    switch(layer)
    {
      case eCAL::Monitoring::eTransportLayerType::shm:
        return Topic::TransportLayer::SHM;
      case eCAL::Monitoring::eTransportLayerType::tcp:
        return Topic::TransportLayer::TCP;
      case eCAL::Monitoring::eTransportLayerType::udp_mc:
        return Topic::TransportLayer::UDP_MC;
      case eCAL::Monitoring::eTransportLayerType::none:
      default:
        return Topic::TransportLayer::NONE;
    }
  }

  void ProcessData()
  {
    std::lock_guard<std::mutex> lock{mtx};

    hosts.clear();
    processes.clear();
    services.clear();
    topics.clear();

    std::unordered_map<std::string, Host*> hosts_map;
    for(const auto &p : mon_.processes)
    {
      if(hosts_map.find(p.host_name) == hosts_map.end())
      {
        auto &host = hosts.emplace_back();
        host.name = p.host_name;
        hosts_map[host.name] = &host;
      }
      auto &process = processes.emplace_back();
      process.process_id = p.process_id;
      process.name = p.process_name;
      process.host_name = p.host_name;
      process.unit_name = p.unit_name;
      process.params = p.process_parameter;
      process.severity = Severity(p.state_severity);
      process.severity_level = SeverityLevel(p.state_severity_level);
      process.state_info = p.state_info;
      process.time_sync_state = TimeSyncState(p.time_sync_state);
      process.time_sync_mod_name = p.time_sync_module_name;
      process.component_init_info = p.component_init_info;
      process.ecal_runtime_version = p.ecal_runtime_version;
    }

    auto process_topic_vector = [&](const std::vector<eCAL::Monitoring::STopic>& source_topics, Topic::Direction direction)
    {
      for (const auto& t : source_topics)
      {
        auto found = hosts_map.find(t.host_name);
        if(found != hosts_map.end())
        {
          auto* host = found->second;
          if(direction == Topic::Direction::PUBLISHER)
          {
            host->publisher_count++;
            host->data_sent_bytes += ((long long)t.topic_size * (long long)t.data_frequency) / 1000;
          }
          else
          {
            host->subscriber_count++;
            host->data_received_bytes += ((long long)t.topic_size * (long long)t.data_frequency) / 1000;
          }
        }

        auto &topic = topics.emplace_back();
        topic.registration_clock = t.registration_clock;
        topic.host_name = t.host_name;
        topic.process_id = t.process_id;
        topic.process_name = t.process_name;
        topic.unit_name = t.unit_name;
        topic.id = std::to_string(t.topic_id);
        topic.name = t.topic_name;
        topic.direction = direction;
        topic.encoding = t.datatype_information.encoding;
        topic.type = t.datatype_information.name;
        topic.type_descriptor = t.datatype_information.descriptor;
        for(const auto &tl : t.transport_layer)
        {
          if (tl.active)
          {
            topic.transport_layers.emplace_back(TopicTransportLayer(tl.type));
          }
        }
        topic.size = t.topic_size;
        topic.local_connections_count = t.connections_local;
        topic.external_connections_count = t.connections_external;
        topic.message_drops = t.message_drops;
        topic.data_id = t.data_id;
        topic.data_clock = t.data_clock;
        topic.data_frequency_mhz = t.data_frequency;
      }
    };

    process_topic_vector(mon_.publishers, Topic::Direction::PUBLISHER);
    process_topic_vector(mon_.subscribers, Topic::Direction::SUBSCRIBER);

    for(const auto &s : mon_.servers)
    {
      auto &service = services.emplace_back();
      service.id = std::to_string(s.service_id);
      service.name = s.service_name;
      service.host_name = s.host_name;
      service.process_name = s.process_name;
      service.unit_name = s.unit_name;
      service.registration_clock = s.registration_clock;
      service.tcp_port = static_cast<int>(s.tcp_port_v1);
      for(const auto &m : s.methods)
      {
        auto &method = service.methods.emplace_back();
        method.name = m.method_name;
        method.request_type = m.request_datatype_information.name;
        method.response_type = m.response_datatype_information.name;
        method.call_count = m.call_count;
      }
    }
  }

  void NotifyUpdate()
  {
    std::lock_guard<std::mutex> lock{callback_mtx};
    for(auto &callback: callbacks_)
    {
      callback();
    }
  }

  void Update()
  {
    eCAL::Monitoring::GetMonitoring(mon_);
    ProcessData();
    NotifyUpdate();
  }

  void PollData()
  {
    while(is_polling)
    {
      Update();
      eCAL::Process::SleepMS(update_time_);
    }
  }

public:
  MonitorModel() : is_polling{true},
    update_thread{std::bind(&MonitorModel::PollData, this)} { }

  std::vector<Host> Hosts()
  {
    std::lock_guard<std::mutex> lock{mtx};
    return hosts;
  }

  std::vector<Process> Processes()
  {
    std::lock_guard<std::mutex> lock{mtx};
    return processes;
  }

  std::vector<Service> Services()
  {
    std::lock_guard<std::mutex> lock{mtx};
    return services;
  }

  std::vector<Topic> Topics()
  {
    std::lock_guard<std::mutex> lock{mtx};
    return topics;
  }

  void AddModelUpdateCallback(ModelUpdateCallbackT callback)
  {
    std::lock_guard<std::mutex> lock{callback_mtx};
    callbacks_.push_back(callback);
  }

  ~MonitorModel()
  {
    is_polling = false;
    update_thread.join();
  }
};
