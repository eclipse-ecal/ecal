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

#include "ecal/util.h"
#include "ecal_def.h"
#include "ecal_globals.h"
#include "ecal_event.h"
#include "registration/ecal_registration_receiver.h"
#include "pubsub/ecal_pubgate.h"
#include "config/ecal_path_processing.h"

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace eCAL
{
  namespace Util
  {
    std::string GeteCALDataDir()
    {
      return eCAL::Config::GeteCALDataDirImpl();
    }

    std::string GeteCALLogDir()
    {      
      return eCAL::Config::GeteCALLogDirImpl();
    }

#if ECAL_CORE_MONITORING
    // take monitoring snapshot
    static Monitoring::SMonitoring GetMonitoring()
    {
      if (IsInitialized(Init::Monitoring) == 0)
      {
        Initialize("", Init::Monitoring);
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
        const std::string process_name = process.process_name;
        if ( (process_name         == process_name_)
          && (process.host_name == host_name)
          )
        {
          proc_id_list.push_back(process.process_id);
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
        //std::cout << "Shutdown local eCAL process " << process_id_ << '\n';
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
        const std::string unit_name = process.unit_name;
        if  ((unit_name != "eCALMon")
          && (unit_name != "eCALPlay")
          && (unit_name != "eCALPlayGUI")
          && (unit_name != "eCALRec")
          && (unit_name != "eCALRecGUI")
          && (unit_name != "eCALSys")
          && (unit_name != "eCALSysGUI")
          && (unit_name != "eCALStop")
          && (unit_name != "eCALTopic")
          && (process.host_name == host_name)
          )
        {
          proc_id_list.push_back(process.process_id);
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
        const std::string unit_name = process.unit_name;
        if (((unit_name == "eCALMon")
          || (unit_name == "eCALPlay")
          || (unit_name == "eCALPlayGUI")
          || (unit_name == "eCALRec")
          || (unit_name == "eCALRecGUI")
          || (unit_name == "eCALSys")
          || (unit_name == "eCALSysGUI")
          || (unit_name == "eCALStop")
          || (unit_name == "eCALTopic")
          )
          && (process.host_name == host_name)
          )
        {
          proc_id_list.push_back(process.process_id);
        }
      }

      for (auto id : proc_id_list)
      {
        ShutdownProcess(id);
      }
    }
#endif // ECAL_CORE_MONITORING

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
