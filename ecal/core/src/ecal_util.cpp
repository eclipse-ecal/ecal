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

#include "ecal_globals.h"
#include "ecal_event.h"
#include "registration/ecal_registration_receiver.h"
#include "pubsub/ecal_pubgate.h"

#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace eCAL
{
  namespace Util
  {
    void EnableLoopback(bool state_)
    {
#if ECAL_CORE_REGISTRATION
      if (g_registration_receiver() != nullptr) g_registration_receiver()->EnableLoopback(state_);
#endif
    }

    void PubShareType(bool state_)
    {
#if ECAL_CORE_PUBLISHER
      if (g_pubgate() != nullptr) g_pubgate()->ShareType(state_);
#endif
    }

    void PubShareDescription(bool state_)
    {
#if ECAL_CORE_PUBLISHER
      if (g_pubgate() != nullptr) g_pubgate()->ShareDescription(state_);
#endif
    }

#if ECAL_CORE_MONITORING
    // take monitoring snapshot
    static Monitoring::SMonitoring GetMonitoring()
    {
      if (IsInitialized(Init::Monitoring) == 0)
      {
        Initialize(0, nullptr, "", Init::Monitoring);
        Process::SleepMS(1000);
      }

      Monitoring::SMonitoring monitoring;
      if (g_monitoring() != nullptr) g_monitoring()->GetMonitoring(monitoring);

      return(monitoring);
    }

    void ShutdownProcess(const std::string& process_name_)
    {
      const Monitoring::SMonitoring monitoring = GetMonitoring();
      const std::string             host_name  = Process::GetHostName();

      std::vector<int> proc_id_list;
      for (const auto& process : monitoring.processes)
      {
        const std::string pname = process.pname;
        if ( (pname         == process_name_)
          && (process.hname == host_name)
          )
        {
          proc_id_list.push_back(process.pid);
        }
      }

      for (auto id : proc_id_list)
      {
        ShutdownProcess(id);
      }
    }

    void ShutdownProcess(const int process_id_)
    {
      const std::string event_name = EVENT_SHUTDOWN_PROC + std::string("_") + std::to_string(process_id_);
      EventHandleT event;
      if (gOpenNamedEvent(&event, event_name, true))
      {
        std::cout << "Shutdown local eCAL process " << process_id_ << '\n';
        gSetEvent(event);
        gCloseEvent(event);
      }
    }

    void ShutdownProcesses()
    {
      const Monitoring::SMonitoring monitoring = GetMonitoring();
      const std::string             host_name = eCAL::Process::GetHostName();

      std::vector<int> proc_id_list;
      for (const auto& process : monitoring.processes)
      {
        const std::string uname = process.uname;
        if  ((uname != "eCALMon")
          && (uname != "eCALPlay")
          && (uname != "eCALPlayGUI")
          && (uname != "eCALRec")
          && (uname != "eCALRecGUI")
          && (uname != "eCALSys")
          && (uname != "eCALSysGUI")
          && (uname != "eCALStop")
          && (uname != "eCALTopic")
          && (process.hname == host_name)
          )
        {
          proc_id_list.push_back(process.pid);
        }
      }

      for (auto id : proc_id_list)
      {
        ShutdownProcess(id);
      }
    }

    void ShutdownCore()
    {
      const Monitoring::SMonitoring monitoring = GetMonitoring();
      const std::string             host_name  = Process::GetHostName();

      std::vector<int> proc_id_list;
      for (const auto& process : monitoring.processes)
      {
        const std::string uname = process.uname;
        if (((uname == "eCALMon")
          || (uname == "eCALPlay")
          || (uname == "eCALPlayGUI")
          || (uname == "eCALRec")
          || (uname == "eCALRecGUI")
          || (uname == "eCALSys")
          || (uname == "eCALSysGUI")
          || (uname == "eCALStop")
          || (uname == "eCALTopic")
          )
          && (process.hname == host_name)
          )
        {
          proc_id_list.push_back(process.pid);
        }
      }

      for (auto id : proc_id_list)
      {
        ShutdownProcess(id);
      }
    }
#endif // ECAL_CORE_MONITORING

    void GetTopics(std::unordered_map<std::string, SDataTypeInformation>& topic_info_map_)
    {
      if (g_descgate() == nullptr) return;
      g_descgate()->GetTopics(topic_info_map_);
    }

    void GetTopicNames(std::vector<std::string>& topic_names_)
    {
      if (g_descgate() == nullptr) return;
      g_descgate()->GetTopicNames(topic_names_);
    }

    bool GetTopicDataTypeInformation(const std::string& topic_name_, SDataTypeInformation& topic_info_)
    {
      if (g_descgate() == nullptr) return(false);
      return(g_descgate()->GetDataTypeInformation(topic_name_, topic_info_));
    }

    void GetServices(std::map<std::tuple<std::string, std::string>, SServiceMethodInformation>& service_info_map_)
    {
      if (g_descgate() == nullptr) return;
      g_descgate()->GetServices(service_info_map_);
    }

    void GetServiceNames(std::vector<std::tuple<std::string, std::string>>& service_method_names_)
    {
      if (g_descgate() == nullptr) return;
      g_descgate()->GetServiceNames(service_method_names_);
    }

    bool GetServiceTypeNames(const std::string& service_name_, const std::string& method_name_, std::string& req_type_, std::string& resp_type_)
    {
      if (g_descgate() == nullptr) return(false);
      return(g_descgate()->GetServiceTypeNames(service_name_, method_name_, req_type_, resp_type_));
    }

    bool GetServiceDescription(const std::string& service_name_, const std::string& method_name_, std::string& req_desc_, std::string& resp_desc_)
    {
      if (g_descgate() == nullptr) return(false);
      return(g_descgate()->GetServiceDescription(service_name_, method_name_, req_desc_, resp_desc_));
    }

    std::pair<std::string, std::string> SplitCombinedTopicType(const std::string& combined_topic_type_)
    {
      auto pos = combined_topic_type_.find(':');
      if (pos == std::string::npos)
      {
        std::string encoding;
        std::string type{ combined_topic_type_ };
        return std::make_pair(encoding, type);
      }
      else
      {
        std::string encoding = combined_topic_type_.substr(0, pos);
        std::string type = combined_topic_type_.substr(pos + 1);
        return std::make_pair(encoding, type);
      }
    }

    std::string CombinedTopicEncodingAndType(const std::string& topic_encoding_, const std::string& topic_type_)
    {
      if (topic_encoding_.empty())
      {
        return topic_type_;
      }
      else
      {
        return topic_encoding_ + ":" + topic_type_;
      }
    }
  }
}
