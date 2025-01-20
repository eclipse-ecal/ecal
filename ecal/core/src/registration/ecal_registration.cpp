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

namespace eCAL
{
  namespace Registration
  {
    std::set<STopicId> GetPublisherIDs()
    {
      if (g_descgate() == nullptr) return std::set<STopicId>();
      return g_descgate()->GetPublisherIDs();
    }

    bool GetPublisherInfo(const STopicId& id_, SDataTypeInformation& topic_info_)
    {
      if (g_descgate() == nullptr) return false;
      return g_descgate()->GetPublisherInfo(id_, topic_info_);
    }

    CallbackToken AddPublisherEventCallback(const TopicEventCallbackT& callback_)
    {
      if (g_descgate() == nullptr) return CallbackToken();
      return g_descgate()->AddPublisherEventCallback(callback_);
    }

    void RemPublisherEventCallback(CallbackToken token_)
    {
      if (g_descgate() == nullptr) return;
      return g_descgate()->RemPublisherEventCallback(token_);
    }

    std::set<STopicId> GetSubscriberIDs()
    {
      if (g_descgate() == nullptr) return std::set<STopicId>();
      return g_descgate()->GetSubscriberIDs();
    }

    bool GetSubscriberInfo(const STopicId& id_, SDataTypeInformation& topic_info_)
    {
      if (g_descgate() == nullptr) return false;
      return g_descgate()->GetSubscriberInfo(id_, topic_info_);
    }

    ECAL_API CallbackToken AddSubscriberEventCallback(const TopicEventCallbackT& callback_)
    {
      if (g_descgate() == nullptr) return CallbackToken();
      return g_descgate()->AddSubscriberEventCallback(callback_);
    }

    void RemSubscriberEventCallback(CallbackToken token_)
    {
      if (g_descgate() == nullptr) return;
      return g_descgate()->RemSubscriberEventCallback(token_);
    }

    std::set<SServiceMethodId> GetServerIDs()
    {
      if (g_descgate() == nullptr) return std::set<SServiceMethodId>();
      return g_descgate()->GetServerIDs();
    }

    bool GetServerInfo(const SServiceMethodId& id_, SServiceMethodInformation& service_info_)
    {
      if (g_descgate() == nullptr) return false;
      return g_descgate()->GetServerInfo(id_, service_info_);
    }

    std::set<SServiceMethodId> GetClientIDs()
    {
      if (g_descgate() == nullptr) return std::set<SServiceMethodId>();
      return g_descgate()->GetClientIDs();
    }

    bool GetClientInfo(const SServiceMethodId& id_, SServiceMethodInformation& service_info_)
    {
      if (g_descgate() == nullptr) return false;
      return g_descgate()->GetClientInfo(id_, service_info_);
    }

    void GetTopicNames(std::set<std::string>& topic_names_)
    {
      topic_names_.clear();

      // get publisher & subscriber id sets and insert names into the topic_names set
      const std::set<STopicId> pub_id_set = GetPublisherIDs();
      for (const auto& pub_id : pub_id_set)
      {
        topic_names_.insert(pub_id.topic_name);
      }
      const std::set<STopicId> sub_id_set = GetSubscriberIDs();
      for (const auto& sub_id : sub_id_set)
      {
        topic_names_.insert(sub_id.topic_name);
      }
    }

    void GetServiceMethodNames(std::set<SServiceMethod>& service_method_names_)
    {
      service_method_names_.clear();

      // get services id set and insert names into the service_method_names set
      const std::set<SServiceMethodId> service_id_set = GetServerIDs();
      for (const auto& service_id : service_id_set)
      {
        service_method_names_.insert({ service_id.service_name, service_id.method_name });
      }
    }

    void GetClientMethodNames(std::set<SServiceMethod>& client_method_names_)
    {
      client_method_names_.clear();

      // get clients id set and insert names into the client_method_names set
      const std::set<SServiceMethodId> client_id_set = GetClientIDs();
      for (const auto& client_id : client_id_set)
      {
        client_method_names_.insert({ client_id.service_name, client_id.method_name });
      }
    }
  }
}
