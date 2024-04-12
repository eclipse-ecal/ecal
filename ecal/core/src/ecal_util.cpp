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

namespace
{
  std::vector<eCAL::Util::SQualityDataTypeInformation> GetTopicDataTypeInformation(const std::string& topic_name_, const std::multimap<std::string, eCAL::Util::SQualityDataTypeInformation>& data_info_multi_map_)
  {
    std::vector<eCAL::Util::SQualityDataTypeInformation> data_info_vec;

    const auto data_info_range = data_info_multi_map_.equal_range(topic_name_);
    for (auto data_info_range_it = data_info_range.first; data_info_range_it != data_info_range.second; ++data_info_range_it)
    {
      data_info_vec.push_back(data_info_range_it->second);
    }

    return data_info_vec;
  }

  std::vector<eCAL::Util::SQualityServiceMethodInformation> GetServiceTypeInformation(const std::string& service_name_, const std::string& method_name_, std::multimap<std::tuple<std::string, std::string>, eCAL::Util::SQualityServiceMethodInformation> service_info_map_)
  {
    std::vector<eCAL::Util::SQualityServiceMethodInformation> service_method_info_vec;

    const auto service_info_range = service_info_map_.equal_range(std::make_pair(service_name_, method_name_));
    for (auto service_info_range_it = service_info_range.first; service_info_range_it != service_info_range.second; ++service_info_range_it)
    {
      service_method_info_vec.push_back(service_info_range_it->second);
    }

    return service_method_info_vec;
  }

  void CopyDataTypeInfoWithQualityComparison(const std::multimap<std::string, eCAL::Util::SQualityDataTypeInformation>& source_multi_map_, std::unordered_map<std::string, eCAL::Util::SQualityDataTypeInformation>& target_map_)
  {
    for (const auto& source_pair : source_multi_map_)
    {
      const std::string&                             source_key   = source_pair.first;
      const eCAL::Util::SQualityDataTypeInformation& source_value = source_pair.second;

      auto target_it = target_map_.find(source_key);
      if (target_it != target_map_.end())
      {
        // key exists in target map
        if (source_value.quality > target_it->second.quality)
        {
          // source quality is greater, overwrite
          target_it->second = source_value;
        }
      }
      else
      {
        // key does not exist in target map, insert source pair
        target_map_.insert(source_pair);
      }
    }
  }
}

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

    std::multimap<std::string, SQualityDataTypeInformation> GetPublisher()
    {
      std::multimap<std::string, SQualityDataTypeInformation> multi_map;
      if (g_descgate() == nullptr) return multi_map;

      // insert publisher into target multimap
      for (const auto& topic : g_descgate()->GetPublisher())
      {
        multi_map.insert(std::pair<std::string, SQualityDataTypeInformation>(topic.first.topic_name, topic.second));
      }

      return multi_map;
    }

    std::vector<SQualityDataTypeInformation> GetPublisher(const std::string& topic_name_)
    {
      return ::GetTopicDataTypeInformation(topic_name_, GetPublisher());
    }

    std::multimap<std::string, SQualityDataTypeInformation> GetSubscriber()
    {
      std::multimap<std::string, SQualityDataTypeInformation> multi_map;
      if (g_descgate() == nullptr) return multi_map;

      // insert subscriber into target multimap
      for (const auto& topic : g_descgate()->GetSubscriber())
      {
        multi_map.insert(std::pair<std::string, SQualityDataTypeInformation>(topic.first.topic_name, topic.second));
      }

      return multi_map;
    }

    std::vector<SQualityDataTypeInformation> GetSubscriber(const std::string& topic_name_)
    {
      return ::GetTopicDataTypeInformation(topic_name_, GetSubscriber());
    }

    SDataTypeInformation GetHighestQualifiedDataTypeInformation(const std::vector<SQualityDataTypeInformation>& data_type_info_vec_)
    {
      SQualityDataTypeInformation quality_info;
      for (const auto& info : data_type_info_vec_)
      {
        if (info.quality > quality_info.quality)
        {
          quality_info = info;
        }
      }
      return quality_info.info;
    }

    ECAL_API std::multimap<std::tuple<std::string, std::string>, SQualityServiceMethodInformation> GetServices()
    {
      std::multimap<std::tuple<std::string, std::string>, SQualityServiceMethodInformation> multi_map;
      if (g_descgate() == nullptr) return multi_map;

      // insert services into target multimap
      for (const auto& service : g_descgate()->GetServices())
      {
        multi_map.insert(std::pair<std::tuple<std::string, std::string>, SQualityServiceMethodInformation>(std::make_tuple(service.first.service_name, service.first.method_name), service.second));
      }
      return multi_map;
    }

    ECAL_API std::multimap<std::tuple<std::string, std::string>, SQualityServiceMethodInformation> GetClients()
    {
      std::multimap<std::tuple<std::string, std::string>, SQualityServiceMethodInformation> multi_map;
      if (g_descgate() == nullptr) return multi_map;

      // insert clients into target multimap
      for (const auto& client : g_descgate()->GetClients())
      {
        multi_map.insert(std::pair<std::tuple<std::string, std::string>, SQualityServiceMethodInformation>(std::make_tuple(client.first.service_name, client.first.method_name), client.second));
      }
      return multi_map;
    }

    SServiceMethodInformation GetHighestQualifiedServiceMethodInformation(const std::vector<SQualityServiceMethodInformation>& service_method_info_vec_)
    {
      SQualityServiceMethodInformation quality_info;
      for (const auto& info : service_method_info_vec_)
      {
        if (info.quality > quality_info.quality)
        {
          quality_info = info;
        }
      }
      return quality_info.info;
    }

    void GetTopics(std::unordered_map<std::string, SDataTypeInformation>& data_type_info_map_)
    {
      std::unordered_map<std::string, SQualityDataTypeInformation> qualified_data_type_info_map;
      GetTopics(qualified_data_type_info_map);

      // transform into target map
      for (const auto& qualified_data_type_info : qualified_data_type_info_map)
      {
        data_type_info_map_.insert(std::pair<std::string, SDataTypeInformation>(qualified_data_type_info.first, qualified_data_type_info.second.info));
      }
    }

    void GetTopics(std::unordered_map<std::string, SQualityDataTypeInformation>& qualified_data_type_info_map_)
    {
      if (g_descgate() == nullptr) return;

      std::multimap<std::string, SQualityDataTypeInformation> merged_pub_sub_map;

      // insert publisher
      for (const auto& topic : g_descgate()->GetPublisher())
      {
        merged_pub_sub_map.insert(std::pair<std::string, SQualityDataTypeInformation>(topic.first.topic_name, topic.second));
      }

      // insert subscriber
      for (const auto& topic : g_descgate()->GetSubscriber())
      {
        merged_pub_sub_map.insert(std::pair<std::string, SQualityDataTypeInformation>(topic.first.topic_name, topic.second));
      }

      // transform into a map with the highest qualified data type information
      ::CopyDataTypeInfoWithQualityComparison(merged_pub_sub_map, qualified_data_type_info_map_);
    }

    void GetTopicNames(std::vector<std::string>& topic_names_)
    {
      // get publisher & subscriber multi maps
      auto pub_multi_map = GetPublisher();
      auto sub_multi_map = GetSubscriber();

      // filter out unique topic names into a set
      std::set<std::string> set;
      for (const auto& publisher : pub_multi_map)
      {
        set.insert(publisher.first);
      }
      for (const auto& subscriber : sub_multi_map)
      {
        set.insert(subscriber.first);
      }

      // transform set into target vector
      topic_names_ = std::vector<std::string>(set.begin(), set.end());
    }

    bool GetTopicDataTypeInformation(const std::string& topic_name_, SDataTypeInformation& topic_info_)
    {
      auto       info_vec     = GetPublisher(topic_name_);
      const auto sub_info_vec = GetSubscriber(topic_name_);

      info_vec.insert(info_vec.end(), sub_info_vec.begin(), sub_info_vec.end());
      topic_info_ = GetHighestQualifiedDataTypeInformation(sub_info_vec);

      return !info_vec.empty();
    }

    void GetServices(std::map<std::tuple<std::string, std::string>, SServiceMethodInformation>& service_info_map_)
    {
      // TODO with comparison
    }

    void GetServiceMethodNames(std::vector<std::tuple<std::string, std::string>>& service_method_names_)
    {
      // TODO with comparison
    }

    bool GetServiceTypeNames(const std::string& service_name_, const std::string& method_name_, std::string& req_type_, std::string& resp_type_)
    {
      std::vector<SQualityServiceMethodInformation> service_method_info_vec = GetServiceTypeInformation(service_name_, method_name_, GetServices());

      SServiceMethodInformation service_method_info = GetHighestQualifiedServiceMethodInformation(service_method_info_vec);
      req_type_  = service_method_info.request_type.name;
      resp_type_ = service_method_info.response_type.name;

      return !service_method_info_vec.empty();
    }

    bool GetServiceDescription(const std::string& service_name_, const std::string& method_name_, std::string& req_desc_, std::string& resp_desc_)
    {
      std::vector<SQualityServiceMethodInformation> service_method_info_vec = GetServiceTypeInformation(service_name_, method_name_, GetServices());

      SServiceMethodInformation service_method_info = GetHighestQualifiedServiceMethodInformation(service_method_info_vec);
      req_desc_ = service_method_info.request_type.descriptor;
      resp_desc_ = service_method_info.response_type.descriptor;

      return !service_method_info_vec.empty();
    }

    void GetClients(std::map<std::tuple<std::string, std::string>, SServiceMethodInformation>& client_info_map_)
    {
      // TODO: Implement this again
    }

    void GetClientMethodNames(std::vector<std::tuple<std::string, std::string>>& client_method_names_)
    {
      // TODO: Implement this again
    }

    bool GetClientTypeNames(const std::string& client_name_, const std::string& method_name_, std::string& req_type_, std::string& resp_type_)
    {
      std::vector<SQualityServiceMethodInformation> service_method_info_vec = GetServiceTypeInformation(client_name_, method_name_, GetClients());

      SServiceMethodInformation service_method_info = GetHighestQualifiedServiceMethodInformation(service_method_info_vec);
      req_type_  = service_method_info.request_type.name;
      resp_type_ = service_method_info.response_type.name;

      return !service_method_info_vec.empty();
    }

    bool GetClientDescription(const std::string& client_name_, const std::string& method_name_, std::string& req_desc_, std::string& resp_desc_)
    {
      std::vector<SQualityServiceMethodInformation> service_method_info_vec = GetServiceTypeInformation(client_name_, method_name_, GetClients());

      SServiceMethodInformation service_method_info = GetHighestQualifiedServiceMethodInformation(service_method_info_vec);
      req_desc_  = service_method_info.request_type.descriptor;
      resp_desc_ = service_method_info.response_type.descriptor;

      return !service_method_info_vec.empty();
    }

#if 0
    std::multimap<std::string, SQualityDataTypeInformation> GetTopics()
    {
      std::multimap<std::string, SQualityDataTypeInformation> multi_map;
      if (g_descgate() == nullptr) return multi_map;

      // insert publisher topics
      for (const auto& topic : g_descgate()->GetPublisher())
      {
        multi_map.insert(std::pair<std::string, SQualityDataTypeInformation>(topic.first.topic_name, topic.second));
      }

      // insert subscriber topics
      for (const auto& topic : g_descgate()->GetSubscriber())
      {
        multi_map.insert(std::pair<std::string, SQualityDataTypeInformation>(topic.first.topic_name, topic.second));
      }

      return multi_map;
    }

    std::set<std::string> GetTopicNames()
    {
      // get all topics as multimap
      std::multimap<std::string, SQualityDataTypeInformation> topic_info_multi_map = GetTopics();

      // filter out unique topic names into a set
      std::set<std::string> set;
      for (const auto& topic : topic_info_multi_map)
      {
        set.insert(topic.first);
      }

      // return set
      return set;
    }
#endif

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
