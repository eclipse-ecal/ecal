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

/**
 * @brief  eCAL utility functions
**/

#include <ecal/ecal_os.h>
#include <ecal/ecal_core.h>

#include "ecal_def.h"
#include "ecal_reggate.h"
#include "ecal_descgate.h"
#include "pubsub/ecal_pubgate.h"
#include "mon/ecal_monitoring_def.h"

#include <ecal/ecal_apps.h>

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
  /**
   * @brief Return the eCAL process state.
   *
   * @return  True if eCAL is in proper state. 
  **/
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
        Process::SleepMS(1000);
      }

      eCAL::pb::Monitoring monitoring;
      if (eCAL::g_monitoring()) eCAL::g_monitoring()->Monitor(monitoring);

      return(monitoring);
    }

    /**
    * @brief Send shutdown event to specified local user process using it's process name.
     *
     * @param process_name_   Fully qualified process name (including the absolute path)
    **/
    void ShutdownProcess(const std::string& process_name_)
    {
      eCAL::pb::Monitoring monitoring = GetMonitoring();
      std::string        host_name = Process::GetHostName();

      std::vector<int> proc_id_list;
      for (int i = 0; i < monitoring.processes().size(); i++)
      {
        const eCAL::pb::Process& process = monitoring.processes(i);
        std::string pname = process.pname();
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

    /**
     * @brief Send shutdown event to specified local user process using it's process id.
     *
     * @param process_id_   Process id.
    **/
    void ShutdownProcess(const int process_id_)
    {
      std::string event_name = EVENT_SHUTDOWN_PROC + std::string("_") + std::to_string(process_id_);
      EventHandleT event;
      if (gOpenEvent(&event, event_name))
      {
        std::cout << "Shutdown local eCAL process " << process_id_ << std::endl;
        gSetEvent(event);
        gCloseEvent(event);
      }
    }

    /**
     * @brief Send shutdown event to all local user processes.
    **/
    void ShutdownProcesses()
    {
      eCAL::pb::Monitoring monitoring = GetMonitoring();
      std::string        host_name = Process::GetHostName();

      std::vector<int> proc_id_list;
      for (int i = 0; i < monitoring.processes().size(); i++)
      {
        const eCAL::pb::Process& process = monitoring.processes(i);
        std::string uname = process.uname();
        if ((uname != "eCALMon")
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

    /**
     * @brief Send shutdown event to all core components.
    **/
    void ShutdownCore()
    {
      eCAL::pb::Monitoring monitoring = GetMonitoring();
      std::string        host_name = Process::GetHostName();

      std::vector<int> proc_id_list;
      for (int i = 0; i < monitoring.processes().size(); i++)
      {
        const eCAL::pb::Process& process = monitoring.processes(i);
        std::string uname = process.uname();
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

    /**
     * @brief Enable eCAL message loop back,
     *          that means subscriber will receive messages from
     *          publishers of the same process (default == false).
     *
     * @param  Switch on message loop back..
    **/
    void EnableLoopback(bool state_)
    {
      if (g_reggate()) g_reggate()->EnableLoopback(state_);
    }

    /**
     * @brief Enable process wide eCAL publisher topic type sharing
     *          that is needed for reflection on subscriber side.
     *
     * @param state_  Switch on type sharing
    **/
    void PubShareType(bool state_)
    {
      if (g_pubgate()) g_pubgate()->ShareType(state_);
    }

    /**
     * @brief Enable process wide eCAL publisher topic description sharing
     *          that is needed for reflection on subscriber side.
     *
     * @param state_  Switch on description sharing
    **/
    void PubShareDescription(bool state_)
    {
      if (g_pubgate()) g_pubgate()->ShareDescription(state_);
    }

    /**
     * @brief Gets type name of the specified topic.
     *
     * @param topic_name_   Topic name.
     * @param topic_type_   String to store type name.
     *
     * @return  True if succeeded.
    **/
    bool GetTypeName(const std::string& topic_name_, std::string& topic_type_)
    {
      if (!g_descgate()) return(false);
      return(g_descgate()->GetTypeName(topic_name_, topic_type_));
    }

    /**
     * @brief Gets type name of the specified topic.
     *
     * @param topic_name_   Topic name.
     *
     * @return  Topic type name.
    **/
    std::string GetTypeName(const std::string& topic_name_)
    {
      std::string topic_type;
      if (GetTypeName(topic_name_, topic_type))
      {
        return(topic_type);
      }
      return("");
    }

    /**
     * @brief Gets description of the specified topic.
     *
     * @param topic_name_   Topic name.
     * @param topic_desc_   String to store description.
     *
     * @return  True if succeeded.
    **/
    bool GetDescription(const std::string& topic_name_, std::string& topic_desc_)
    {
      if (!g_descgate()) return(false);
      return(g_descgate()->GetDescription(topic_name_, topic_desc_));
    }

    /**
     * @brief Gets description of the specified topic.
     *
     * @param topic_name_   Topic name.
     *
     * @return  Topic description.
    **/
    std::string GetDescription(const std::string& topic_name_)
    {
      std::string topic_desc;
      if (GetDescription(topic_name_, topic_desc))
      {
        return(topic_desc);
      }
      return("");
    }
  }
}
