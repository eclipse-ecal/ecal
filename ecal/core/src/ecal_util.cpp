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
        eCAL::Process::SleepMS(1000);
      }

      eCAL::pb::Monitoring monitoring;
      if (eCAL::g_monitoring()) eCAL::g_monitoring()->GetMonitoring(monitoring);

      return(monitoring);
    }

    /**
    * @brief Send shutdown event to specified local user process using it's process name.
     *
     * @param process_name_   Fully qualified process name (including the absolute path)
    **/
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

    /**
     * @brief Send shutdown event to specified local user process using it's process id.
     *
     * @param process_id_   Process id.
    **/
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

    /**
     * @brief Send shutdown event to all local user processes.
    **/
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

    /**
     * @brief Send shutdown event to all core components.
    **/
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

    /**
     * @brief Enable eCAL message loop back,
     *          that means subscriber will receive messages from
     *          publishers of the same process (default == false).
     *
     * @param  Switch on message loop back..
    **/
    void EnableLoopback(bool state_)
    {
      if (g_registration_receiver()) g_registration_receiver()->EnableLoopback(state_);
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
     * @brief Get complete topic map (including types and descriptions).
     *
     * @param topic_info_map_  Map to store the topic informations.
     *                         Map containing { TopicName -> (Type, Description) } mapping of all topics that are currently known.
    **/
    void GetTopics(std::unordered_map<std::string, STopicInfo>& topic_info_map_)
    {
      if (!g_descgate()) return;
      g_descgate()->GetTopics(topic_info_map_);
    }

    /**
     * @brief Get all topic names.
     *
     * @param topic_names_ Vector to store the topic names.
    **/
    void GetTopicNames(std::vector<std::string>& topic_names_)
    {
      if (!g_descgate()) return;
      g_descgate()->GetTopicNames(topic_names_);
    }

    /**
     * @brief Gets type name of the specified topic.
     *
     * @param topic_name_   Topic name.
     * @param topic_type_   String to store type name.
     *
     * @return  True if succeeded.
    **/
    bool GetTopicTypeName(const std::string& topic_name_, std::string& topic_type_)
    {
      if (!g_descgate()) return(false);
      return(g_descgate()->GetTopicTypeName(topic_name_, topic_type_));
    }

    // [[deprecated]]
    bool GetTypeName(const std::string& topic_name_, std::string& topic_type_)
    {
      return GetTopicTypeName(topic_name_, topic_type_);
    }

    /**
     * @brief Gets type name of the specified topic.
     *
     * @param topic_name_   Topic name.
     *
     * @return  Topic type name.
    **/
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

    /**
     * @brief Gets description of the specified topic.
     *
     * @param topic_name_   Topic name.
     * @param topic_desc_   String to store description.
     *
     * @return  True if succeeded.
    **/
    bool GetTopicDescription(const std::string& topic_name_, std::string& topic_desc_)
    {
      if (!g_descgate()) return(false);
      return(g_descgate()->GetTopicDescription(topic_name_, topic_desc_));
    }

    // [[deprecated]]
    bool GetDescription(const std::string& topic_name_, std::string& topic_desc_)
    {
      return GetTopicDescription(topic_name_, topic_desc_);
    }

    /**
     * @brief Gets description of the specified topic.
     *
     * @param topic_name_   Topic name.
     *
     * @return  Topic description.
    **/
    std::string GetTopicDescription(const std::string& topic_name_)
    {
      std::string topic_desc;
      if (GetTopicDescription(topic_name_, topic_desc))
      {
        return(topic_desc);
      }
      return("");
    }

    // [[deprecated]]
    std::string GetDescription(const std::string& topic_name_)
    {
      return GetTopicDescription(topic_name_);
    }

    /**
     * @brief Get complete service map (including request and response types and descriptions).
     *
     * @param service_info_map_  Map to store the topic informations.
     *                           Map { (ServiceName, MethodName) -> ( (ReqType, ReqDescription), (RespType, RespDescription) ) } mapping of all currently known services.
    **/
    void GetServices(std::map<std::tuple<std::string, std::string>, Util::SServiceMethodInfo>& service_info_map_)
    {
      if (!g_descgate()) return;
      g_descgate()->GetServices(service_info_map_);
    }

    /**
     * @brief Get all service/method names.
     *
     * @param service_names_ Vector to store the service/method tuples (Vector { (ServiceName, MethodName) }).
    **/
    void GetServiceNames(std::vector<std::tuple<std::string, std::string>>& service_method_names_)
    {
      if (!g_descgate()) return;
      g_descgate()->GetServiceNames(service_method_names_);
    }

    /**
     * @brief Gets service method request and response type names.
     *
     * @param service_name_  Service name.
     * @param method_name_   Method name.
     * @param req_type_      String to store request type.
     * @param resp_type_     String to store response type.
     *
     * @return  True if succeeded.
    **/
    bool GetServiceTypeNames(const std::string& service_name_, const std::string& method_name_, std::string& req_type_, std::string& resp_type_)
    {
      if (!g_descgate()) return(false);
      return(g_descgate()->GetServiceTypeNames(service_name_, method_name_, req_type_, resp_type_));
    }

    /**
     * @brief Gets service method request and response descriptions.
     *
     * @param service_name_  Service name.
     * @param method_name_   Method name.
     * @param req_desc_      String to store request description.
     * @param resp_desc_     String to store response description.
     *
     * @return  True if succeeded.
    **/
    bool GetServiceDescription(const std::string& service_name_, const std::string& method_name_, std::string& req_desc_, std::string& resp_desc_)
    {
      if (!g_descgate()) return(false);
      return(g_descgate()->GetServiceDescription(service_name_, method_name_, req_desc_, resp_desc_));
    }
  }
}
