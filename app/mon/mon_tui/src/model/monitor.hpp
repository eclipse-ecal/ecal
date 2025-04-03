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

#include <functional>
#include <string>
#include <utility>
#include <vector>
#include <thread>
#include <map>

#include "model/data/host.hpp"
#include "model/data/process.hpp"
#include "model/data/service.hpp"
#include "model/data/topic.hpp"

#ifdef _MSC_VER
#pragma warning(push, 0) // disable proto warnings
#endif
#include "ecal/core/pb/monitoring.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

using ModelUpdateCallbackT = std::function<void()>;

class MonitorModel
{
  int update_time_ = 1000;
  bool is_polling;
  eCAL::pb::Monitoring mon_;

  std::vector<Host> hosts;
  std::vector<Process> processes;
  std::vector<Service> services;
  std::vector<Topic> topics;

  std::vector<ModelUpdateCallbackT> callbacks_;
  std::thread update_thread;
  mutable std::mutex mtx;
  mutable std::mutex callback_mtx;

  Process::Severity Severity(eCAL::pb::eProcessSeverity severity)
  {
    switch(severity)
    {
    case eCAL::pb::proc_sev_healthy:
      return Process::Severity::HEALTHY;
    case eCAL::pb::proc_sev_warning:
      return Process::Severity::WARNING;
    case eCAL::pb::proc_sev_critical:
      return Process::Severity::CRITICAL;
    case eCAL::pb::proc_sev_failed:
      return Process::Severity::FAILED;
    case eCAL::pb::proc_sev_unknown:
    default:
      return Process::Severity::UNKNOWN;
    }
  }

  Process::SeverityLevel SeverityLevel(eCAL::pb::eProcessSeverityLevel level)
  {
    switch(level)
    {
    case eCAL::pb::proc_sev_level1:
      return Process::SeverityLevel::LEVEL_1;
    case eCAL::pb::proc_sev_level2:
      return Process::SeverityLevel::LEVEL_2;
    case eCAL::pb::proc_sev_level3:
      return Process::SeverityLevel::LEVEL_3;
    case eCAL::pb::proc_sev_level4:
      return Process::SeverityLevel::LEVEL_4;
    case eCAL::pb::proc_sev_level5:
      return Process::SeverityLevel::LEVEL_5;
    case eCAL::pb::proc_sev_level_unknown:
    default:
      return Process::SeverityLevel::UNKNOWN;
    }
  }

  Process::TimeSyncState TimeSyncState(eCAL::pb::eTimeSyncState state)
  {
    switch(state)
    {
    case eCAL::pb::tsync_realtime:
      return Process::TimeSyncState::REALTIME;
    case eCAL::pb::tsync_replay:
      return Process::TimeSyncState::REALTIME;
    case eCAL::pb::tsync_none:
    default:
      return Process::TimeSyncState::NONE;
    }
  }

  Topic::Direction TopicDirection(const std::string &direction)
  {
    if(direction == "publisher") return Topic::Direction::PUBLISHER;

    return Topic::Direction::SUBSCRIBER;
  }

  Topic::TransportLayer TopicTransportLayer(eCAL::pb::eTransportLayerType layer)
  {
    switch(layer)
    {
      case eCAL::pb::tl_all:
        return Topic::TransportLayer::ALL;
      case eCAL::pb::tl_ecal_shm:
        return Topic::TransportLayer::SHM;
      case eCAL::pb::tl_ecal_tcp:
        return Topic::TransportLayer::TCP;
      case eCAL::pb::tl_ecal_udp_mc:
        return Topic::TransportLayer::UDP_MC;
      case eCAL::pb::tl_none:
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
    for(auto &p: *mon_.mutable_processes())
    {
      if(hosts_map.find(p.host_name()) == hosts_map.end())
      {
        auto &host = hosts.emplace_back();
        host.name = p.host_name();
        hosts_map[host.name] = &host;
      }
      auto &process = processes.emplace_back();
      process.process_id = p.process_id();
      process.name = std::move(*p.mutable_process_name());
      process.host_name = std::move(*p.mutable_host_name());
      process.unit_name = std::move(*p.mutable_unit_name());
      process.params = std::move(*p.mutable_process_parameter());
      process.severity = Severity(p.state().severity());
      process.severity_level = SeverityLevel(p.state().severity_level());
      process.state_info = std::move(*p.mutable_state()->mutable_info());
      process.time_sync_state = TimeSyncState(p.time_sync_state());
      process.time_sync_mod_name = std::move(*p.mutable_time_sync_module_name());
      process.component_init_info = std::move(*p.mutable_component_init_info());
      process.ecal_runtime_version = std::move(*p.mutable_ecal_runtime_version());
    }

    for (auto &t : *mon_.mutable_topics())
    {
      auto &name = t.host_name();
      auto &direction = t.direction();
      auto found = hosts_map.find(name);
      if(found != hosts_map.end())
      {
        auto host = found->second;
        if(direction == "publisher")
        {
          host->publisher_count++;
          host->data_sent_bytes += ((long long)t.topic_size() * (long long)t.data_frequency()) / 1000;
        }
        else if(direction == "subscriber")
        {
          host->subscriber_count++;
          host->data_received_bytes += ((long long)t.topic_size() * (long long)t.data_frequency()) / 1000;
        }
      }
      auto &topic = topics.emplace_back();
      topic.registration_clock = t.registration_clock();
      topic.host_name = std::move(*t.mutable_host_name());
      topic.process_id = t.process_id();
      topic.process_name = std::move(*t.mutable_process_name());
      topic.unit_name = std::move(*t.mutable_unit_name());
      topic.id = std::move(*t.mutable_topic_id());
      topic.name = std::move(*t.mutable_topic_name());
      topic.direction = TopicDirection(t.direction());
      topic.encoding = std::move(*t.mutable_datatype_information()->mutable_encoding());
      topic.type = std::move(*t.mutable_datatype_information()->mutable_name());
      topic.type_descriptor = std::move(*t.mutable_datatype_information()->mutable_descriptor_information());
      for(auto &tl: t.transport_layer())
      {
        if (tl.active())
        {
          topic.transport_layers.emplace_back(TopicTransportLayer(tl.type()));
        }
      }
      topic.size = t.topic_size();
      topic.local_connections_count = t.connections_local();
      topic.external_connections_count = t.connections_external();
      topic.message_drops = t.message_drops();
      topic.data_id = t.data_id();
      topic.data_clock = t.data_clock();
      topic.data_frequency = t.data_frequency();
    }

    for(auto &s: *mon_.mutable_services())
    {
      auto &service = services.emplace_back();
      service.id = s.service_id();
      service.name = std::move(*s.mutable_service_name());
      service.host_name = std::move(*s.mutable_host_name());
      service.process_name = std::move(*s.mutable_process_name());
      service.unit_name = std::move(*s.mutable_unit_name());
      service.registration_clock = s.registration_clock();
      service.tcp_port = s.tcp_port_v1();
      for(auto &m: *s.mutable_methods())
      {
        auto &method = service.methods.emplace_back();
        method.name = std::move(*m.mutable_method_name());
        method.request_type = std::move(*m.mutable_req_type());
        method.response_type = std::move(*m.mutable_resp_type());
        method.call_count = m.call_count();
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
    std::string raw_data;
    eCAL::Monitoring::GetMonitoring(raw_data);
    mon_.ParseFromString(raw_data);
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
