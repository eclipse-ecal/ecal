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

#include <ecal/ecal.h>
#include <ecal_c/registration.h>

#include "common.h"

#include <map>

namespace
{
  size_t ExtSize_SServiceMethod(const eCAL::Registration::SServiceMethod& service_method_)
  {
    return ExtSize_String(service_method_.service_name) +
      ExtSize_String(service_method_.method_name);
  }

  void Convert_SServiceMethod(struct eCAL_Registration_SServiceMethod* service_method_c_, const eCAL::Registration::SServiceMethod& service_method_, char** offset_)
  {
    service_method_c_->service_name = Convert_String(service_method_.service_name, offset_);
    service_method_c_->method_name = Convert_String(service_method_.method_name, offset_);
  }

  enum eCAL_Registration_RegistrationEventType Convert_RegistrationEventType(eCAL::Registration::RegistrationEventType registration_event_type)
  {
    static const std::map<eCAL::Registration::RegistrationEventType, enum eCAL_Registration_RegistrationEventType> registration_event_type_map
    {
      {eCAL::Registration::RegistrationEventType::new_entity, eCAL_Registration_RegistrationEventType_new_entity},
      {eCAL::Registration::RegistrationEventType::deleted_entity, eCAL_Registration_RegistrationEventType_deleted_entity}
    };

    return registration_event_type_map.at(registration_event_type);
  }
}

extern "C"
{

  /*
Usage:
  eCAL_STopicId* topic_ids = NULL;
  size_t length = 0;
  if(eCAL_Registration_GetPublisherIDs(&topic_ids, &length)
  {
    for(int i = 0; i < length; ++i)
    {
      topic_ids[i]....
    }
  }
  free(topic_ids);

*/
  ECALC_API int eCAL_Registration_GetPublisherIDs(struct eCAL_STopicId** topic_ids_, size_t* topic_ids_length_)
  {
    if (topic_ids_ == NULL || topic_ids_length_ == NULL) return 1;
    if (*topic_ids_ != NULL || *topic_ids_length_ != 0) return 1;

    std::set<eCAL::STopicId> topic_ids;
    if (eCAL::Registration::GetPublisherIDs(topic_ids))
    {
      size_t extended_size{ 0 };
      for (const auto& topic_id : topic_ids)
        extended_size += ExtSize_STopicId(topic_id);
      const auto base_size = aligned_size(topic_ids.size() * sizeof(struct eCAL_STopicId));

      *topic_ids_ = reinterpret_cast<struct eCAL_STopicId*>(std::malloc(extended_size + base_size));
      if (*topic_ids_ != NULL)
      {
        size_t i{ 0 };
        *topic_ids_length_ = topic_ids.size();
        auto* offset = reinterpret_cast<char*>(*topic_ids_) + base_size;
        for (const auto& topic_id : topic_ids)
          Convert_STopicId(&((*topic_ids_)[i++]), topic_id, &offset);
      }
      
    }
    return !static_cast<int>((*topic_ids_ != NULL));
  }

  ECALC_API int eCAL_Registration_GetPublisherInfo(const struct eCAL_STopicId* topic_id_, struct eCAL_SDataTypeInformation** topic_info_)
  {
    if (topic_id_ == NULL || topic_info_ == NULL) return 1;
    if (*topic_info_ != NULL) return 1;

    eCAL::SDataTypeInformation topic_info;
    eCAL::STopicId topic_id;
    Convert_STopicId(topic_id, topic_id_);

    if (eCAL::Registration::GetPublisherInfo(topic_id, topic_info))
    {
      const auto extended_size = ExtSize_SDataTypeInformation(topic_info);
      const auto base_size = aligned_size(sizeof(struct eCAL_SDataTypeInformation));
      *topic_info_ = reinterpret_cast<struct eCAL_SDataTypeInformation*>(std::malloc(base_size + extended_size));
      if (*topic_info_ != NULL)
      {
        auto* offset = reinterpret_cast<char*>(*topic_info_) + base_size;
        Convert_SDataTypeInformation(*topic_info_, topic_info, &offset);
      }
    }

    return !static_cast<int>(*topic_info_ != NULL);
  }

  ECALC_API int eCAL_Registration_GetSubscriberIDs(struct eCAL_STopicId** topic_ids_, size_t* topic_ids_length_)
  {
    if (topic_ids_ == NULL || topic_ids_length_ == NULL) return 1;
    if (*topic_ids_ != NULL || *topic_ids_length_ != 0) return 1;

    std::set<eCAL::STopicId> topic_ids;
    if (eCAL::Registration::GetSubscriberIDs(topic_ids))
    {
      size_t extended_size{ 0 };

      for (const auto& topic_id : topic_ids)
        extended_size += ExtSize_STopicId(topic_id);

      const auto base_size = aligned_size(topic_ids.size() * sizeof(struct eCAL_STopicId));

      *topic_ids_ = reinterpret_cast<struct eCAL_STopicId*>(std::malloc(extended_size + base_size));
      if (*topic_ids_ != NULL)
      {
        size_t i{ 0 };
        *topic_ids_length_ = topic_ids.size();
        auto* offset = reinterpret_cast<char*>(*topic_ids_) + base_size;
        for (const auto& topic_id : topic_ids)
          Convert_STopicId(&((*topic_ids_)[i++]), topic_id, &offset);
      }
    }
    return !static_cast<int>((*topic_ids_ != NULL));
  }

  ECALC_API int eCAL_Registration_GetSubscriberInfo(const struct eCAL_STopicId* topic_id_, struct eCAL_SDataTypeInformation** topic_info_)
  {
    if (topic_id_ == NULL || topic_info_ == NULL) return 1;
    if (*topic_info_ != NULL) return 1;

    eCAL::SDataTypeInformation topic_info;
    eCAL::STopicId topic_id;
    Convert_STopicId(topic_id, topic_id_);

    if (eCAL::Registration::GetSubscriberInfo(topic_id, topic_info))
    {
      const auto extended_size = ExtSize_SDataTypeInformation(topic_info);
      const auto base_size = aligned_size(sizeof(struct eCAL_SDataTypeInformation));
      *topic_info_ = reinterpret_cast<struct eCAL_SDataTypeInformation*>(std::malloc(base_size + extended_size));
      if (*topic_info_ != NULL)
      {
        auto* offset = reinterpret_cast<char*>(*topic_info_) + base_size;
        Convert_SDataTypeInformation(*topic_info_, topic_info, &offset);
      }
    }
    return !static_cast<int>(*topic_info_ != NULL);
  }

  ECALC_API int eCAL_Registration_GetServerIDs(struct eCAL_SServiceId** service_ids_, size_t* service_ids_length_)
  {
    if (service_ids_ == NULL || service_ids_length_ == NULL) return 1;
    if (*service_ids_ != NULL || *service_ids_length_ != 0) return 1;

    std::set<eCAL::SServiceId> service_ids;
    if (eCAL::Registration::GetServerIDs(service_ids))
    {
      size_t extended_size{ 0 };

      for (const auto& service_id : service_ids)
        extended_size += ExtSize_SServiceId(service_id);

      const auto base_size = aligned_size(service_ids.size() * sizeof(struct eCAL_SServiceId));

      *service_ids_ = reinterpret_cast<struct eCAL_SServiceId*>(std::malloc(extended_size + base_size));
      if (*service_ids_ != NULL)
      {
        size_t i{ 0 };
        *service_ids_length_ = service_ids.size();
        auto* offset = reinterpret_cast<char*>(*service_ids_) + base_size;
        for (const auto& service_id : service_ids)
          Convert_SServiceId(&((*service_ids_)[i++]), service_id, &offset);
      }
    }
    return !static_cast<int>((*service_ids_ != NULL));
  }

  ECALC_API int eCAL_Registration_GetServerInfo(const struct eCAL_SServiceId* service_id_, struct eCAL_SServiceMethodInformation** service_method_info_, size_t* service_method_info_length_)
  {
    if (service_id_ == NULL || service_method_info_ == NULL || service_method_info_length_ == NULL) return 1;
    if (*service_method_info_ != NULL || *service_method_info_length_ != 0) return 1;

    eCAL::SServiceId service_id;
    Convert_SServiceId(service_id, service_id_);

    eCAL::ServiceMethodInformationSetT service_method_info;
    if (eCAL::Registration::GetServerInfo(service_id, service_method_info))
    {
      size_t extended_size{ 0 };

      for (const auto& method_info : service_method_info)
        extended_size += ExtSize_SServiceMethodInformation(method_info);

      const auto base_size = aligned_size(service_method_info.size() * sizeof(struct eCAL_SServiceMethodInformation));

      *service_method_info_ = reinterpret_cast<struct eCAL_SServiceMethodInformation*>(std::malloc(extended_size + base_size));
      if (*service_method_info_ != NULL)
      {
        size_t i{ 0 };
        *service_method_info_length_ = service_method_info.size();
        auto* offset = reinterpret_cast<char*>(*service_method_info_) + base_size;
        for (const auto& method_info : service_method_info)
          Convert_SServiceMethodInformation(&((*service_method_info_)[i++]), method_info, &offset);
      }
    }
    return !static_cast<int>((*service_method_info_ != NULL));
  }

  ECALC_API int eCAL_Registration_GetClientIDs(struct eCAL_SServiceId** service_ids_, size_t* service_ids_length_)
  {
    if (service_ids_ == NULL || service_ids_length_ == NULL) return 1;
    if (*service_ids_ != NULL || *service_ids_length_ != 0) return 1;

    std::set<eCAL::SServiceId> service_ids;
    if (eCAL::Registration::GetClientIDs(service_ids))
    {
      size_t extended_size{ 0 };

      for (const auto& service_id : service_ids)
        extended_size += ExtSize_SServiceId(service_id);

      const auto base_size = aligned_size(service_ids.size() * sizeof(struct eCAL_SServiceId));

      *service_ids_ = reinterpret_cast<struct eCAL_SServiceId*>(std::malloc(extended_size + base_size));
      if (*service_ids_ != NULL)
      {
        size_t i{ 0 };
        *service_ids_length_ = service_ids.size();
        auto* offset = reinterpret_cast<char*>(*service_ids_) + base_size;
        for (const auto& service_id : service_ids)
          Convert_SServiceId(&((*service_ids_)[i++]), service_id, &offset);
      }
    }
    return !static_cast<int>((*service_ids_ != NULL));
  }

  ECALC_API int eCAL_Registration_GetClientInfo(const struct eCAL_SServiceId* service_id_, struct eCAL_SServiceMethodInformation** service_method_info_, size_t* service_method_info_length_)
  {
    if (service_id_ == NULL || service_method_info_ == NULL || service_method_info_length_ == NULL) return 1;
    if (*service_method_info_ != NULL || *service_method_info_length_ != 0) return 1;

    eCAL::SServiceId service_id;
    Convert_SServiceId(service_id, service_id_);

    eCAL::ServiceMethodInformationSetT service_method_info;
    if (eCAL::Registration::GetClientInfo(service_id, service_method_info))
    {
      size_t extended_size{ 0 };

      for (const auto& method_info : service_method_info)
        extended_size += ExtSize_SServiceMethodInformation(method_info);

      const auto base_size = aligned_size(service_method_info.size() * sizeof(struct eCAL_SServiceMethodInformation));

      *service_method_info_ = reinterpret_cast<struct eCAL_SServiceMethodInformation*>(std::malloc(extended_size + base_size));
      if (*service_method_info_ != NULL)
      {
        size_t i{ 0 };
        *service_method_info_length_ = service_method_info.size();
        auto* offset = reinterpret_cast<char*>(*service_method_info_) + base_size;
        for (const auto& method_info : service_method_info)
          Convert_SServiceMethodInformation(&((*service_method_info_)[i++]), method_info, &offset);
      }
    }
    return !static_cast<int>((*service_method_info_ != NULL));
  }

  ECALC_API int eCAL_Registration_GetPublishedTopicNames(char*** topic_names_, size_t* topic_names_length_)
  {
    if (topic_names_ == NULL || topic_names_length_ == NULL) return 1;
    if (*topic_names_ != NULL || *topic_names_length_ != 0) return 1;

    std::set<std::string> topic_names;
    if (eCAL::Registration::GetPublishedTopicNames(topic_names))
    {
      Convert_StringSet(topic_names_, topic_names_length_, topic_names);
    }
    return !static_cast<int>((*topic_names_ != NULL));
  }

  ECALC_API int eCAL_Registration_GetSubscribedTopicNames(char*** topic_names_, size_t* topic_names_length_)
  {
    if (topic_names_ == NULL || topic_names_length_ == NULL) return 1;
    if (*topic_names_ != NULL || *topic_names_length_ != 0) return 1;

    std::set<std::string> topic_names;
    if (eCAL::Registration::GetSubscribedTopicNames(topic_names))
    {
      Convert_StringSet(topic_names_, topic_names_length_, topic_names);
    }
    return !static_cast<int>((*topic_names_ != NULL));
  }

  ECALC_API int eCAL_Registration_GetServerMethodNames(struct eCAL_Registration_SServiceMethod** server_method_names_, size_t* server_method_names_length_)
  {
    if (server_method_names_ == NULL || server_method_names_length_ == NULL) return 1;
    if (*server_method_names_ != NULL || *server_method_names_length_ != 0) return 1;

    std::set<eCAL::Registration::SServiceMethod> server_method_names;
    if (eCAL::Registration::GetServerMethodNames(server_method_names))
    {
      size_t extended_size{ 0 };

      for (const auto& method : server_method_names)
        extended_size += ExtSize_SServiceMethod(method);

      const auto base_size = aligned_size(server_method_names.size() * sizeof(struct eCAL_Registration_SServiceMethod));

      *server_method_names_ = reinterpret_cast<struct eCAL_Registration_SServiceMethod*>(std::malloc(extended_size + base_size));
      if (*server_method_names_ != NULL)
      {
        size_t i{ 0 };
        *server_method_names_length_ = server_method_names.size();
        auto* offset = reinterpret_cast<char*>(*server_method_names_) + base_size;
        for (const auto& method : server_method_names)
          Convert_SServiceMethod(&((*server_method_names_)[i++]), method, &offset);
      }
    }
    return !static_cast<int>((*server_method_names_ != NULL));
  }

  ECALC_API int eCAL_Registration_GetClientMethodNames(struct eCAL_Registration_SServiceMethod** client_method_names_, size_t* client_method_names_length_)
  {
    if (client_method_names_ == NULL || client_method_names_length_ == NULL) return 1;
    if (*client_method_names_ != NULL || *client_method_names_length_ != 0) return 1;

    std::set<eCAL::Registration::SServiceMethod> client_method_names;
    if (eCAL::Registration::GetClientMethodNames(client_method_names))
    {
      size_t extended_size{ 0 };

      for (const auto& method : client_method_names)
        extended_size += ExtSize_SServiceMethod(method);

      const auto base_size = aligned_size(client_method_names.size() * sizeof(struct eCAL_Registration_SServiceMethod));

      *client_method_names_ = reinterpret_cast<struct eCAL_Registration_SServiceMethod*>(std::malloc(extended_size + base_size));
      if (*client_method_names_ != NULL)
      {
        size_t i{ 0 };
        *client_method_names_length_ = client_method_names.size();
        auto* offset = reinterpret_cast<char*>(*client_method_names_) + base_size;
        for (const auto& method : client_method_names)
          Convert_SServiceMethod(&((*client_method_names_)[i++]), method, &offset);
      }
    }
    return !static_cast<int>((*client_method_names_ != NULL));
  }

  ECALC_API eCAL_Registration_CallbackToken eCAL_Registration_AddPublisherEventCallback(eCAL_Registration_TopicEventCallbackT callback_)
  {
    if (callback_ == nullptr) return 0;

    auto callback = [callback_](const eCAL::STopicId & topic_id_, eCAL::Registration::RegistrationEventType registration_event_type_)
    {
      struct eCAL_STopicId topic_id_c;
      Assign_STopicId(&topic_id_c, topic_id_);
      callback_(&topic_id_c, Convert_RegistrationEventType(registration_event_type_));
    };

    return eCAL::Registration::AddPublisherEventCallback(callback);
  }

  ECALC_API void eCAL_Registration_RemPublisherEventCallback(eCAL_Registration_CallbackToken token_)
  {
    eCAL::Registration::RemPublisherEventCallback(token_);
  }

  ECALC_API eCAL_Registration_CallbackToken eCAL_Registration_AddSubscriberEventCallback(eCAL_Registration_TopicEventCallbackT callback_)
  {
    if (callback_ == nullptr) return 0;

    auto callback = [callback_](const eCAL::STopicId& topic_id_, eCAL::Registration::RegistrationEventType registration_event_type_)
    {
      struct eCAL_STopicId topic_id_c;
      Assign_STopicId(&topic_id_c, topic_id_);
      callback_(&topic_id_c, Convert_RegistrationEventType(registration_event_type_));
    };

    return eCAL::Registration::AddSubscriberEventCallback(callback);
  }

  ECALC_API void eCAL_Registration_RemSubscriberEventCallback(eCAL_Registration_CallbackToken token_)
  {
    eCAL::Registration::RemSubscriberEventCallback(token_);
  }
}