/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
 * @brief Registration public API.
 *
**/

#include <ecal/ecal_registration.h>

#include "ecal_def.h"
#include "ecal_globals.h"
#include "ecal_event.h"
#include "registration/ecal_registration_receiver.h"
#include "pubsub/ecal_pubgate.h"

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
  std::set<eCAL::Registration::SQualityTopicInfo> GetQualityTopicInfoSet(const std::string& topic_name_, const eCAL::Registration::QualityTopicInfoMultiMap& quality_data_type_info_multi_map_)
  {
    std::set<eCAL::Registration::SQualityTopicInfo> quality_topic_info_set;

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
  std::set<eCAL::Registration::SQualityServiceInfo> GetQualityServiceInfoSet(const std::string& service_name_, const std::string& method_name_, const eCAL::Registration::QualityServiceInfoMultimap& quality_service_info_multi_map_)
  {
    std::set<eCAL::Registration::SQualityServiceInfo> quality_service_info_set;

    eCAL::Registration::SServiceMethod key;
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
  std::map<std::string, eCAL::Registration::SQualityTopicInfo> ReduceQualityTopicIdMap(const eCAL::Registration::QualityTopicInfoMultiMap& source_map_)
  {
    std::map<std::string, eCAL::Registration::SQualityTopicInfo> target_map;

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
  std::map<eCAL::Registration::SServiceMethod, eCAL::Registration::SQualityServiceInfo> ReduceQualityServiceIdMap(const eCAL::Registration::QualityServiceInfoMultimap& source_map_)
  {
    std::map<eCAL::Registration::SServiceMethod, eCAL::Registration::SQualityServiceInfo> target_map;

    for (const auto& source_pair : source_map_)
    {
      const auto& source_key   = source_pair.first;
      const auto& source_value = source_pair.second;

      eCAL::Registration::SServiceMethod target_key;
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
  namespace Registration
  {
    std::set<STopicId> GetPublisherIDs()
    {
      if (g_descgate() == nullptr) return std::set<STopicId>();
      return g_descgate()->GetPublisherIDs();
    }

    bool GetPublisherInfo(const STopicId& id_, SQualityTopicInfo& topic_info_)
    {
      if (g_descgate() == nullptr) return false;
      return g_descgate()->GetPublisherInfo(id_, topic_info_);
    }

    ECAL_API CallbackToken AddPublisherEventCallback(const TopicIDCallbackT& callback_)
    {
      if (g_descgate() == nullptr) return CallbackToken();
      return g_descgate()->AddPublisherEventCallback(callback_);
    }

    ECAL_API void RemPublisherEventCallback(CallbackToken token_)
    {
      if (g_descgate() == nullptr) return;
      return g_descgate()->RemPublisherEventCallback(token_);
    }

    std::set<STopicId> GetSubscriberIDs()
    {
      if (g_descgate() == nullptr) return std::set<STopicId>();
      return g_descgate()->GetSubscriberIDs();
    }

    bool GetSubscriberInfo(const STopicId& id_, SQualityTopicInfo& topic_info_)
    {
      if (g_descgate() == nullptr) return false;
      return g_descgate()->GetSubscriberInfo(id_, topic_info_);
    }

    ECAL_API CallbackToken AddSubscriberEventCallback(const TopicIDCallbackT& callback_)
    {
      if (g_descgate() == nullptr) return CallbackToken();
      return g_descgate()->AddSubscriberEventCallback(callback_);
    }

    ECAL_API void RemSubscriberEventCallback(CallbackToken token_)
    {
      if (g_descgate() == nullptr) return;
      return g_descgate()->RemSubscriberEventCallback(token_);
    }

    std::set<SServiceId> GetServiceIDs()
    {
      if (g_descgate() == nullptr) return std::set<SServiceId>();
      return g_descgate()->GetServiceIDs();
    }

    bool GetServiceInfo(const SServiceId& id_, SQualityServiceInfo& service_info_)
    {
      if (g_descgate() == nullptr) return false;
      return g_descgate()->GetServiceInfo(id_, service_info_);
    }

    std::set<SServiceId> GetClientIDs()
    {
      if (g_descgate() == nullptr) return std::set<SServiceId>();
      return g_descgate()->GetClientIDs();
    }

    bool GetClientInfo(const SServiceId& id_, SQualityServiceInfo& service_info_)
    {
      if (g_descgate() == nullptr) return false;
      return g_descgate()->GetClientInfo(id_, service_info_);
    }

    QualityTopicInfoMultiMap GetPublishers()
    {
      const std::set<STopicId> id_set = GetPublisherIDs();

      Registration::QualityTopicInfoMultiMap multi_map;
      for (const auto& id : id_set)
      {
        SQualityTopicInfo quality_info;
        if (GetPublisherInfo(id, quality_info))
        {
          multi_map.insert(std::pair<std::string, Registration::SQualityTopicInfo>(id.topic_name, quality_info));
        }
      }
      return multi_map;
    }

    QualityTopicInfoSet GetPublishers(const std::string& topic_name_)
    {
      return ::GetQualityTopicInfoSet(topic_name_, GetPublishers());
    }

    QualityTopicInfoMultiMap GetSubscribers()
    {
      const std::set<STopicId> id_set = GetSubscriberIDs();

      Registration::QualityTopicInfoMultiMap multi_map;
      for (const auto& id : id_set)
      {
        SQualityTopicInfo quality_info;
        if (GetSubscriberInfo(id, quality_info))
        {
          multi_map.insert(std::pair<std::string, Registration::SQualityTopicInfo>(id.topic_name, quality_info));
        }
      }
      return multi_map;
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
      const std::set<SServiceId> id_set = GetServiceIDs();

      Registration::QualityServiceInfoMultimap multi_map;
      for (const auto& id : id_set)
      {
        SQualityServiceInfo quality_info;
        if (GetServiceInfo(id, quality_info))
        {
          multi_map.insert(std::pair<SServiceMethod, Registration::SQualityServiceInfo>(SServiceMethod{ id.service_name, id.method_name }, quality_info));
        }
      }
      return multi_map;
    }

    QualityServiceInfoMultimap GetClients()
    {
      const std::set<SServiceId> id_set = GetClientIDs();

      Registration::QualityServiceInfoMultimap multi_map;
      for (const auto& id : id_set)
      {
        SQualityServiceInfo quality_info;
        if (GetClientInfo(id, quality_info))
        {
          multi_map.insert(std::pair<SServiceMethod, Registration::SQualityServiceInfo>(SServiceMethod{ id.service_name, id.method_name }, quality_info));
        }
      }
      return multi_map;
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

      QualityTopicInfoMultiMap pub_sub_map = GetPublishers();
      QualityTopicInfoMultiMap sub_map     = GetSubscribers();
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

      // transform into a map with the highest quality service method information
      quality_service_info_map_ = ReduceQualityServiceIdMap(GetServices());
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

      // transform into a map with the highest quality service method information
      quality_client_info_map_ = ReduceQualityServiceIdMap(GetClients());
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
  }
}
