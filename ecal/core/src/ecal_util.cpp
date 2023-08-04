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

#include <ecal/ecal_os.h>
#include <ecal/ecal_core.h>
#include <ecal/ecal_types.h>

#include "ecal_def.h"
#include "ecal_descgate.h"
#include "ecal_process.h"
#include "ecal_registration_receiver.h"
#include "pubsub/ecal_pubgate.h"
#include "mon/ecal_monitoring_def.h"

#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <iostream>
#include <list>
#include <string>

#ifdef ECAL_OS_WINDOWS
#include "ecal_win_main.h"
#endif /* ECAL_OS_WINDOWS */

namespace eCAL
{
  bool Ok()
  {
    return(g_shutdown == 0);
  }

  namespace Util
  {
    // take monitoring snapshot
    static eCAL::pb::Monitoring GetMonitoring()
    {
      if (!eCAL::IsInitialized(eCAL::Init::Monitoring))
      {
        eCAL::Initialize(0, nullptr, "", eCAL::Init::Monitoring);
        eCAL::Process::SleepMS(1000);
      }

      eCAL::pb::Monitoring monitoring;
      if (eCAL::g_monitoring()) eCAL::g_monitoring()->GetMonitoring(monitoring);

      return(monitoring);
    }

    void ShutdownProcess(const std::string& process_name_)
    {
      const eCAL::pb::Monitoring monitoring = GetMonitoring();
      const std::string          host_name  = eCAL::Process::GetHostName();

      std::vector<int> proc_id_list;
      for (int i = 0; i < monitoring.processes().size(); i++)
      {
        const eCAL::pb::Process& process = monitoring.processes(i);
        const std::string pname = process.pname();
        if ((pname == process_name_)
          && (process.hname() == host_name)
          )
        {
          proc_id_list.push_back(process.pid());
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
      if (gOpenEvent(&event, event_name))
      {
        std::cout << "Shutdown local eCAL process " << process_id_ << std::endl;
        gSetEvent(event);
        gCloseEvent(event);
      }
    }

    void ShutdownProcesses()
    {
      const eCAL::pb::Monitoring monitoring = GetMonitoring();
      const std::string          host_name  = eCAL::Process::GetHostName();

      std::vector<int> proc_id_list;
      for (int i = 0; i < monitoring.processes().size(); i++)
      {
        const eCAL::pb::Process& process = monitoring.processes(i);
        const std::string uname = process.uname();
        if  ((uname != "eCALMon")
          && (uname != "eCALRPCService")
          && (uname != "eCALParam")
          && (uname != "eCALPlay")
          && (uname != "eCALPlayGUI")
          && (uname != "eCALRec")
          && (uname != "eCALCanRec")
          && (uname != "eCALRecGUI")
          && (uname != "eCALStop")
          && (uname != "eCALTopic")
          && (process.hname() == host_name)
          )
        {
          proc_id_list.push_back(process.pid());
        }
      }

      for (auto id : proc_id_list)
      {
        ShutdownProcess(id);
      }
    }

    void ShutdownCore()
    {
      const eCAL::pb::Monitoring monitoring = GetMonitoring();
      const std::string          host_name  = eCAL::Process::GetHostName();

      std::vector<int> proc_id_list;
      for (int i = 0; i < monitoring.processes().size(); i++)
      {
        const eCAL::pb::Process& process = monitoring.processes(i);
        const std::string uname = process.uname();
        if (((uname == "eCALMon")
          || (uname == "eCALParam")
          || (uname == "eCALPlay")
          || (uname == "eCALPlayGUI")
          || (uname == "eCALRec")
          || (uname == "eCALCanRec")
          || (uname == "eCALRecGUI")
          || (uname == "eCALStop")
          || (uname == "eCALTopic")
          )
          && (process.hname() == host_name)
          )
        {
          proc_id_list.push_back(process.pid());
        }
      }

      for (auto id : proc_id_list)
      {
        ShutdownProcess(id);
      }
    }

    void EnableLoopback(bool state_)
    {
      if (g_registration_receiver()) g_registration_receiver()->EnableLoopback(state_);
    }

    void PubShareType(bool state_)
    {
      if (g_pubgate()) g_pubgate()->ShareType(state_);
    }

    void PubShareDescription(bool state_)
    {
      if (g_pubgate()) g_pubgate()->ShareDescription(state_);
    }

    void GetTopics(std::unordered_map<std::string, SDataTypeInformation>& topic_info_map_)
    {
      if (!g_descgate()) return;
      g_descgate()->GetTopics(topic_info_map_);
    }

    void GetTopicNames(std::vector<std::string>& topic_names_)
    {
      if (!g_descgate()) return;
      g_descgate()->GetTopicNames(topic_names_);
    }

    // [[deprecated]]
    bool GetTopicTypeName(const std::string& topic_name_, std::string& topic_type_)
    {
      SDataTypeInformation topic_info;
      auto ret = GetTopicDataTypeInformation(topic_name_, topic_info);
      topic_type_ = Util::CombinedTopicEncodingAndType(topic_info.encoding, topic_info.name);
      return ret;
    }

    // [[deprecated]]
    bool GetTypeName(const std::string& topic_name_, std::string& topic_type_)
    {
      return GetTopicTypeName(topic_name_, topic_type_);
    }

    // [[deprecated]]
    std::string GetTopicTypeName(const std::string& topic_name_)
    {
      std::string topic_type;
      if (GetTopicTypeName(topic_name_, topic_type))
      {
        return(topic_type);
      }
      return("");
    }

    // [[deprecated]]
    std::string GetTypeName(const std::string& topic_name_)
    {
      return GetTopicTypeName(topic_name_);
    }

    // [[deprecated]]
    bool GetTopicDescription(const std::string& topic_name_, std::string& topic_desc_)
    {
      SDataTypeInformation topic_info;
      auto ret = GetTopicDataTypeInformation(topic_name_, topic_info);
      topic_desc_ = topic_info.descriptor;
      return ret;
    }

    // [[deprecated]]
    bool GetDescription(const std::string& topic_name_, std::string& topic_desc_)
    {
      return GetTopicDescription(topic_name_, topic_desc_);
    }

    // [[deprecated]]
    std::string GetTopicDescription(const std::string& topic_name_)
    {
      std::string topic_desc;
      if (GetTopicDescription(topic_name_, topic_desc))
      {
        return(topic_desc);
      }
      return("");
    }

    bool GetTopicDataTypeInformation(const std::string& topic_name_, SDataTypeInformation& topic_info_)
    {
      if (g_descgate() == nullptr) return(false);
      return(g_descgate()->GetDataTypeInformation(topic_name_, topic_info_));
    }

    // [[deprecated]]
    std::string GetDescription(const std::string& topic_name_)
    {
      return GetTopicDescription(topic_name_);
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

    void GetServices(std::map<std::tuple<std::string, std::string>, SServiceMethodInformation>& service_info_map_)
    {
      if (!g_descgate()) return;
      g_descgate()->GetServices(service_info_map_);
    }

    void GetServiceNames(std::vector<std::tuple<std::string, std::string>>& service_method_names_)
    {
      if (!g_descgate()) return;
      g_descgate()->GetServiceNames(service_method_names_);
    }

    bool GetServiceTypeNames(const std::string& service_name_, const std::string& method_name_, std::string& req_type_, std::string& resp_type_)
    {
      if (!g_descgate()) return(false);
      return(g_descgate()->GetServiceTypeNames(service_name_, method_name_, req_type_, resp_type_));
    }

    bool GetServiceDescription(const std::string& service_name_, const std::string& method_name_, std::string& req_desc_, std::string& resp_desc_)
    {
      if (!g_descgate()) return(false);
      return(g_descgate()->GetServiceDescription(service_name_, method_name_, req_desc_, resp_desc_));
    }
  }
}
