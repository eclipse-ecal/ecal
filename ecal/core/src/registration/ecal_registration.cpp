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

#include <ecal/registration.h>

#include "ecal_def.h"
#include "ecal_globals.h"
#include "ecal_event.h"
#include "registration/ecal_registration_receiver.h"
#include "pubsub/ecal_pubgate.h"

namespace eCAL
{
  namespace Registration
  {
    bool GetPublisherIDs(std::set<STopicId>& topic_ids_)
    {
      if (g_descgate() == nullptr) return false;
      topic_ids_ = std::move(g_descgate()->GetPublisherIDs());
      return true;
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

    bool GetSubscriberIDs(std::set<STopicId>& topic_ids_)
    {
      if (g_descgate() == nullptr) return false;
      topic_ids_ = std::move(g_descgate()->GetSubscriberIDs());
      return true;
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

    bool GetServerIDs(std::set<SServiceId>& service_ids_)
    {
      if (g_descgate() == nullptr) return false;
      service_ids_ = std::move(g_descgate()->GetServerIDs());
      return true;
    }

    bool GetServerInfo(const SServiceId& id_, ServiceMethodInformationSetT& service_info_)
    {
      if (g_descgate() == nullptr) return false;
      return g_descgate()->GetServerInfo(id_, service_info_);
    }

    bool GetClientIDs(std::set<SServiceId>& service_ids_)
    {
       if (g_descgate() == nullptr) return false;
       service_ids_ = std::move(g_descgate()->GetClientIDs());
       return true;
    }

    bool GetClientInfo(const SServiceId& id_, ServiceMethodInformationSetT& service_info_)
    {
      if (g_descgate() == nullptr) return false;
      return g_descgate()->GetClientInfo(id_, service_info_);
    }

    bool GetPublishedTopicNames(std::set<std::string>& topic_names_)
    {
      topic_names_.clear();

      // get publisher id sets and insert names into the topic_names set
      std::set<STopicId> pub_id_set;
      bool return_value = GetPublisherIDs(pub_id_set);
      for (const auto& pub_id : pub_id_set)
      {
        topic_names_.insert(pub_id.topic_name);
      }
      return return_value;
    }

    bool GetSubscribedTopicNames(std::set<std::string>& topic_names_)
    {
      topic_names_.clear();

      // get subscriber id sets and insert names into the topic_names set
      std::set<STopicId> sub_id_set;
      bool return_value = GetSubscriberIDs(sub_id_set);
      for (const auto& sub_id : sub_id_set)
      {
        topic_names_.insert(sub_id.topic_name);
      }
      return return_value;
    }
    
    bool GetServerMethodNames(std::set<SServiceMethod>& server_method_names_)
    {
      server_method_names_.clear();

      // get servers id set and insert names into the server_method_names_ set
      std::set<SServiceId> server_id_set;
      bool return_value   = GetServerIDs(server_id_set);
      for (const auto& server_id : server_id_set)
      {
        eCAL::ServiceMethodInformationSetT methods;
        (void)GetServerInfo(server_id, methods);
        for (const auto& method : methods)
        {
          server_method_names_.insert({ server_id.service_name, method.method_name });
        }
      }
      return return_value;
    }

    bool GetClientMethodNames(std::set<SServiceMethod>& client_method_names_)
    {
      client_method_names_.clear();

      // get clients id set and insert names into the client_method_names set
      std::set<SServiceId> client_id_set;
      bool return_value = GetClientIDs(client_id_set);
      for (const auto& client_id : client_id_set)
      {
        eCAL::ServiceMethodInformationSetT methods;
        (void)GetClientInfo(client_id, methods);
        for (const auto& method : methods)
        {
          client_method_names_.insert({ client_id.service_name, method.method_name });
        }
      }
      return return_value;
    }
    
  }
}
