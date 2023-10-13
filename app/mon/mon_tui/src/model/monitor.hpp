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

#include <chrono>

#include <ecal/ecal.h>
#include <ecal/ecal_monitoring.h>

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

  Process::TimeSyncState TimeSyncState(eCAL::pb::eTSyncState state)
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

  Topic::QOSHistoryKind TopicHistoryKind(eCAL::pb::QOS_eQOSPolicy_HistoryKind kind)
  {
    switch(kind)
    {
      case eCAL::pb::QOS_eQOSPolicy_HistoryKind_keep_all_history_qos:
        return Topic::QOSHistoryKind::KEEP_ALL;
      case eCAL::pb::QOS_eQOSPolicy_HistoryKind_keep_last_history_qos:
      default:
        return Topic::QOSHistoryKind::KEEP_LAST;
    }
  }

  Topic::QOSReliability TopicReliability(eCAL::pb::QOS_eQOSPolicy_Reliability reliability)
  {
    switch(reliability)
    {
      case eCAL::pb::QOS_eQOSPolicy_Reliability_reliable_reliability_qos:
        return Topic::QOSReliability::RELIABLE;
      case eCAL::pb::QOS_eQOSPolicy_Reliability_best_effort_reliability_qos:
      default:
        return Topic::QOSReliability::BEST_EFFORT;
    }
  }

  Topic::TransportLayer TopicTransportLayer(eCAL::pb::eTLayerType layer)
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
      case eCAL::pb::tl_inproc:
        return Topic::TransportLayer::INPROC;
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
      if(hosts_map.find(p.hname()) == hosts_map.end())
      {
        auto &host = hosts.emplace_back();
        host.name = p.hname();
        hosts_map[host.name] = &host;
      }
      auto &process = processes.emplace_back();
      process.pid = p.pid();
      process.name = std::move(*p.mutable_pname());
      process.host_name = std::move(*p.mutable_hname());
      process.unit_name = std::move(*p.mutable_uname());
      process.params = std::move(*p.mutable_pparam());
      process.memory_usage = p.pmemory();
      process.cpu_usage = p.pcpu();;
      process.user_time = p.usrptime();
      process.data_sent_bytes = p.datawrite();
      process.data_recieved_bytes = p.dataread();
      process.severity = Severity(p.state().severity());
      process.severity_level = SeverityLevel(p.state().severity_level());
      process.state_info = std::move(*p.mutable_state()->mutable_info());
      process.time_sync_state = TimeSyncState(p.tsync_state());
      process.time_sync_mod_name = std::move(*p.mutable_tsync_mod_name());
      process.component_init_info = std::move(*p.mutable_component_init_info());
      process.ecal_runtime_version = std::move(*p.mutable_ecal_runtime_version());
    }

    for (auto &t : *mon_.mutable_topics())
    {
      auto &name = t.hname();
      auto &direction = t.direction();
      auto found = hosts_map.find(name);
      if(found != hosts_map.end())
      {
        auto host = found->second;
        if(direction == "publisher")
        {
          host->publisher_count++;
          host->data_sent_bytes += ((long long)t.tsize() * (long long)t.dfreq()) / 1000;
        }
        else if(direction == "subscriber")
        {
          host->subscriber_count++;
          host->data_received_bytes += ((long long)t.tsize() * (long long)t.dfreq()) / 1000;
        }
      }
      auto &topic = topics.emplace_back();
      topic.registration_clock = t.rclock();
      topic.host_id = t.hid();
      topic.host_name = std::move(*t.mutable_hname());
      topic.pid = t.pid();
      topic.process_name = std::move(*t.mutable_pname());
      topic.unit_name = std::move(*t.mutable_uname());
      topic.id = std::move(*t.mutable_tid());
      topic.name = std::move(*t.mutable_tname());
      topic.direction = TopicDirection(t.direction());
      topic.type = std::move(*t.mutable_ttype());
      topic.type_descriptor = std::move(*t.mutable_tdesc());
      topic.history_kind = TopicHistoryKind(t.tqos().history());
      topic.history_depth = t.tqos().history_depth();
      topic.reliability = TopicReliability(t.tqos().reliability());
      for(auto &tl: t.tlayer())
      {
        topic.transport_layers.emplace_back(TopicTransportLayer(tl.type()));
      }
      topic.size = t.tsize();
      topic.local_connections_count = t.connections_loc();
      topic.external_connections_count = t.connections_ext();
      topic.message_drops = t.message_drops();
      topic.data_id = t.did();
      topic.data_clock = t.dclock();
      topic.data_frequency = t.dfreq();
      for(auto &attr: *t.mutable_attr())
      {
        topic.attributes.emplace(
          std::pair<std::string, std::string>(
            std::move(attr.first), std::move(attr.second)
          )
        );
      }
    }

    for(auto &s: *mon_.mutable_services())
    {
      auto &service = services.emplace_back();
      service.id = s.sid();
      service.name = std::move(*s.mutable_sname());
      service.host_name = std::move(*s.mutable_hname());
      service.process_name = std::move(*s.mutable_pname());
      service.unit_name = std::move(*s.mutable_uname());
      service.registration_clock = s.rclock();
      service.tcp_port = s.tcp_port_v1();
      for(auto &m: *s.mutable_methods())
      {
        auto &method = service.methods.emplace_back();
        method.name = std::move(*m.mutable_mname());
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
