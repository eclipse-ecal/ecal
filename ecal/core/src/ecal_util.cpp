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

#include "ecal_def.h"
#include "ecal_globals.h"
#include "ecal_event.h"
#include "registration/ecal_registration_receiver.h"
#include "pubsub/ecal_pubgate.h"

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace
{
  /**
   * @brief Extract a set of all SQualityTopicInfo matching the given topic name.
   *
   * @param topic_name_                        The topic name.
   * @param quality_data_type_info_multi_map_  MultiMap { TopicName -> SQualityTopicInfo }.
   *
   * @return                                   Set of SQualityTopicInfo
  **/
  std::set<eCAL::Util::SQualityTopicInfo> GetQualityTopicInfoSet(const std::string& topic_name_, const eCAL::Util::QualityTopicInfoMultiMap& quality_data_type_info_multi_map_)
  {
    std::set<eCAL::Util::SQualityTopicInfo> quality_topic_info_set;

    const auto topic_info_range = quality_data_type_info_multi_map_.equal_range(topic_name_);
    for (auto topic_info_range_it = topic_info_range.first; topic_info_range_it != topic_info_range.second; ++topic_info_range_it)
    {
      quality_topic_info_set.insert(topic_info_range_it->second);
    }

    return quality_topic_info_set;
  }

  /**
   * @brief Extract a set of all SQualityServiceInfo matching the given service name/method name.
   *
   * @param service_name_                    The service name.
   * @param method_name_                     The method name.
   * @param quality_service_info_multi_map_  MultiMap { <ServiceName, MethodName>-> SQualityServiceInfo }.
   *
   * @return                                 Set of SQualityServiceInfo
  **/
  std::set<eCAL::Util::SQualityServiceInfo> GetQualityServiceInfoSet(const std::string& service_name_, const std::string& method_name_, const eCAL::Util::QualityServiceInfoMultimap& quality_service_info_multi_map_)
  {
    std::set<eCAL::Util::SQualityServiceInfo> quality_service_info_set;

    eCAL::Util::SServiceMethod key;
    key.service_name = service_name_;
    key.method_name  = method_name_;
    const auto service_info_range = quality_service_info_multi_map_.equal_range(key);
    for (auto service_info_range_it = service_info_range.first; service_info_range_it != service_info_range.second; ++service_info_range_it)
    {
      quality_service_info_set.insert(service_info_range_it->second);
    }

    return quality_service_info_set;
  }

  /**
   * @brief Reducing std::map<(TopicName, TopicID), SQualityTopicInfo> to
   *        std::map<TopicName, SQualityTopicInfo> based on the quality
   *
   * @param source_map_  std::map<(TopicName, TopicID), SQualityTopicInfo>.
   *
   * @return             std::map<TopicName, SQualityTopicInfo>
  **/
  std::map<std::string, eCAL::Util::SQualityTopicInfo> ReduceQualityTopicIdMap(const eCAL::Util::QualityTopicInfoMultiMap& source_map_)
  {
    std::map<std::string, eCAL::Util::SQualityTopicInfo> target_map;

    for (const auto& source_pair : source_map_)
    {
      const auto& source_key   = source_pair.first;
      const auto& source_value = source_pair.second;

      auto target_it = target_map.find(source_key);
      if (target_it != target_map.end())
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
        target_map.insert(std::make_pair(source_key, source_value));
      }
    }

    return target_map;
  }

  /**
   * @brief Reducing std::map<(ServiceName, ServiceId, MethodName), SQualityServiceInfo> to
   *        std::map<std::tuple<ServiceName, MethodName>, SQualityServiceInfo> based on the quality
   *
   * @param source_map_  std::map<(ServiceName, ServiceId, MethodName), SQualityServiceInfo>.
   *
   * @return             std::map<std::tuple<ServiceName, MethodName>, SQualityServiceInfo>
  **/
  std::map<eCAL::Util::SServiceMethod, eCAL::Util::SQualityServiceInfo> ReduceQualityServiceIdMap(const eCAL::Util::QualityServiceInfoMultimap& source_map_)
  {
    std::map<eCAL::Util::SServiceMethod, eCAL::Util::SQualityServiceInfo> target_map;

    for (const auto& source_pair : source_map_)
    {
      const auto& source_key   = source_pair.first;
      const auto& source_value = source_pair.second;

      eCAL::Util::SServiceMethod target_key;
      target_key.service_name = source_key.service_name;
      target_key.method_name  = source_key.method_name;
      auto target_it = target_map.find(target_key);
      if (target_it != target_map.end())
      {
        // key exists in target map
        if ( (source_value.request_quality  > target_it->second.request_quality)
          || (source_value.response_quality > target_it->second.response_quality))
        {
          // source quality is greater, overwrite
          target_it->second = source_value;
        }
      }
      else
      {
        // key does not exist in target map, insert source pair
        target_map.insert(std::make_pair(target_key, source_pair.second));
      }
    }

    return target_map;
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

    QualityTopicInfoMultiMap GetPublishers()
    {
      if (g_descgate() == nullptr) return QualityTopicInfoMultiMap();
      return g_descgate()->GetPublishers();
    }

    QualityTopicInfoSet GetPublishers(const std::string& topic_name_)
    {
      return ::GetQualityTopicInfoSet(topic_name_, GetPublishers());
    }

    QualityTopicInfoMultiMap GetSubscribers()
    {
      if (g_descgate() == nullptr) return QualityTopicInfoMultiMap();
      return g_descgate()->GetSubscribers();
    }

    QualityTopicInfoSet GetSubscribers(const std::string& topic_name_)
    {
      return ::GetQualityTopicInfoSet(topic_name_, GetSubscribers());
    }

    SDataTypeInformation GetHighestQualityDataTypeInformation(const QualityTopicInfoSet& quality_topic_info_set_)
    {
      SQualityTopicInfo highest_quality_topic_info;
      for (const auto& info : quality_topic_info_set_)
      {
        if (info.quality > highest_quality_topic_info.quality)
        {
          highest_quality_topic_info = info;
        }
      }
      return highest_quality_topic_info.info;
    }

    QualityServiceInfoMultimap GetServices()
    {
      if (g_descgate() == nullptr) return QualityServiceInfoMultimap();
      return g_descgate()->GetServices();
    }

    QualityServiceInfoMultimap GetClients()
    {
      if (g_descgate() == nullptr) return QualityServiceInfoMultimap();
      return g_descgate()->GetClients();
    }

    SServiceMethodInformation GetHighestQualityServiceMethodInformation(const SQualityServiceInfoSet& quality_service_info_set_)
    {
      SQualityServiceInfo highest_quality_service_info;
      for (const auto& info : quality_service_info_set_)
      {
        if ( (info.request_quality  > highest_quality_service_info.request_quality)
          || (info.response_quality > highest_quality_service_info.response_quality))
        {
          highest_quality_service_info = info;
        }
      }
      return highest_quality_service_info.info;
    }

    void GetTopics(std::map<std::string, SDataTypeInformation>& data_type_info_map_)
    {
      data_type_info_map_.clear();

      std::map<std::string, SQualityTopicInfo> quality_data_type_info_map;
      GetTopics(quality_data_type_info_map);

      // transform into target map
      for (const auto& quality_data_type_info : quality_data_type_info_map)
      {
        data_type_info_map_.insert(std::pair<std::string, SDataTypeInformation>(quality_data_type_info.first, quality_data_type_info.second.info));
      }
    }

    void GetTopics(std::map<std::string, SQualityTopicInfo>& quality_topic_info_map_)
    {
      quality_topic_info_map_.clear();
      if (g_descgate() == nullptr) return;

      QualityTopicInfoMultiMap pub_sub_map = g_descgate()->GetPublishers();
      QualityTopicInfoMultiMap sub_map     = g_descgate()->GetSubscribers();
      pub_sub_map.insert(sub_map.begin(), sub_map.end());

      // transform into a map with the highest quality data type information
      quality_topic_info_map_ = ReduceQualityTopicIdMap(pub_sub_map);
    }

    void GetTopicNames(std::set<std::string>& topic_names_)
    {
      topic_names_.clear();

      // get publisher & subscriber multi maps
      auto pub_multi_map = GetPublishers();
      auto sub_multi_map = GetSubscribers();

      // filter out unique topic names into a set
      for (const auto& publisher : pub_multi_map)
      {
        topic_names_.insert(publisher.first);
      }
      for (const auto& subscriber : sub_multi_map)
      {
        topic_names_.insert(subscriber.first);
      }
    }

    bool GetTopicDataTypeInformation(const std::string& topic_name_, SDataTypeInformation& data_type_info_)
    {
      auto       info_set     = GetPublishers(topic_name_);
      const auto sub_info_set = GetSubscribers(topic_name_);

      info_set.insert(sub_info_set.begin(), sub_info_set.end());
      data_type_info_ = GetHighestQualityDataTypeInformation(info_set);

      return !info_set.empty();
    }

    void GetServices(std::map<SServiceMethod, SServiceMethodInformation>& service_method_info_map_)
    {
      service_method_info_map_.clear();

      std::map<SServiceMethod, SQualityServiceInfo> quality_service_method_info_map;
      GetServices(quality_service_method_info_map);

      // transform into target map
      for (const auto& quality_service_method_info : quality_service_method_info_map)
      {
        service_method_info_map_.insert(std::pair<SServiceMethod, SServiceMethodInformation>(quality_service_method_info.first, quality_service_method_info.second.info));
      }
    }

    void GetServices(std::map<SServiceMethod, SQualityServiceInfo>& quality_service_info_map_)
    {
      quality_service_info_map_.clear();
      if (g_descgate() == nullptr) return;

      // transform into a map with the highest quality service method information
      quality_service_info_map_ = ReduceQualityServiceIdMap(g_descgate()->GetServices());
    }

    void GetServiceMethodNames(std::set<SServiceMethod>& service_method_names_)
    {
      service_method_names_.clear();

      // get services multi map
      auto multi_map = GetServices();

      // filter out unique service names into a set
      for (const auto& service : multi_map)
      {
        service_method_names_.insert(service.first);
      }
    }

    bool GetServiceTypeNames(const std::string& service_name_, const std::string& method_name_, std::string& req_type_, std::string& resp_type_)
    {
      const auto service_method_info_set = GetQualityServiceInfoSet(service_name_, method_name_, GetServices());

      const SServiceMethodInformation service_method_info = GetHighestQualityServiceMethodInformation(service_method_info_set);
      req_type_  = service_method_info.request_type.name;
      resp_type_ = service_method_info.response_type.name;

      return !service_method_info_set.empty();
    }

    bool GetServiceDescription(const std::string& service_name_, const std::string& method_name_, std::string& req_desc_, std::string& resp_desc_)
    {
      const auto service_method_info_set = GetQualityServiceInfoSet(service_name_, method_name_, GetServices());

      const SServiceMethodInformation service_method_info = GetHighestQualityServiceMethodInformation(service_method_info_set);
      req_desc_  = service_method_info.request_type.descriptor;
      resp_desc_ = service_method_info.response_type.descriptor;

      return !service_method_info_set.empty();
    }

    void GetClients(std::map<SServiceMethod, SServiceMethodInformation>& client_method_info_map_)
    {
      client_method_info_map_.clear();

      std::map<SServiceMethod, SQualityServiceInfo> quality_client_method_info_map_;
      GetClients(quality_client_method_info_map_);

      // transform into target map
      for (const auto& quality_client_method_info : quality_client_method_info_map_)
      {
        client_method_info_map_.insert(std::pair<SServiceMethod, SServiceMethodInformation>(quality_client_method_info.first, quality_client_method_info.second.info));
      }
    }

    void GetClients(std::map<SServiceMethod, SQualityServiceInfo>& quality_client_info_map_)
    {
      quality_client_info_map_.clear();
      if (g_descgate() == nullptr) return;

      // transform into a map with the highest quality service method information
      quality_client_info_map_ = ReduceQualityServiceIdMap(g_descgate()->GetClients());
    }

    void GetClientMethodNames(std::set<SServiceMethod>& client_method_names_)
    {
      client_method_names_.clear();

      // get services multi map
      auto multi_map = GetClients();

      // filter out unique service names into a set
      for (const auto& service : multi_map)
      {
        client_method_names_.insert(service.first);
      }
    }

    bool GetClientTypeNames(const std::string& client_name_, const std::string& method_name_, std::string& req_type_, std::string& resp_type_)
    {
      const auto service_method_info_set = GetQualityServiceInfoSet(client_name_, method_name_, GetClients());

      const SServiceMethodInformation service_method_info = GetHighestQualityServiceMethodInformation(service_method_info_set);
      req_type_  = service_method_info.request_type.name;
      resp_type_ = service_method_info.response_type.name;

      return !service_method_info_set.empty();
    }

    bool GetClientDescription(const std::string& client_name_, const std::string& method_name_, std::string& req_desc_, std::string& resp_desc_)
    {
      const auto service_method_info_set = GetQualityServiceInfoSet(client_name_, method_name_, GetClients());

      const SServiceMethodInformation service_method_info = GetHighestQualityServiceMethodInformation(service_method_info_set);
      req_desc_  = service_method_info.request_type.descriptor;
      resp_desc_ = service_method_info.response_type.descriptor;

      return !service_method_info_set.empty();
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
