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

#ifndef ecal_c_registration_h_included
#define ecal_c_registration_h_included

#include <ecal_c/export.h>
#include <ecal_c/pubsub/types.h>
#include <ecal_c/types.h>

#include <stddef.h>

struct eCAL_Registration_SServiceMethod
{
  const char* service_name;
  const char* method_name;
};

typedef size_t eCAL_Registration_CallbackToken;

enum eCAL_Registration_RegistrationEventType
{
  eCAL_Registration_RegistrationEventType_new_entity,     //!< Represents a new entity registration
  eCAL_Registration_RegistrationEventType_deleted_entity  //!< Represents a deletion of an entity
};

typedef void (*eCAL_Registration_TopicEventCallbackT)(const struct eCAL_STopicId*, enum eCAL_Registration_RegistrationEventType);

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  ECALC_API int eCAL_Registration_GetPublisherIDs(struct eCAL_STopicId** topic_ids_, size_t* topic_ids_length_);

  ECALC_API int eCAL_Registration_GetPublisherInfo(const struct eCAL_STopicId* topic_id_, struct eCAL_SDataTypeInformation** topic_info_);

  ECALC_API int eCAL_Registration_GetSubscriberIDs(struct eCAL_STopicId** topic_ids_, size_t* topic_ids_length_);

  ECALC_API int eCAL_Registration_GetSubscriberInfo(const struct eCAL_STopicId* topic_id_, struct eCAL_SDataTypeInformation** topic_info_);

  ECALC_API int eCAL_Registration_GetServerIDs(struct eCAL_SServiceId** service_ids_, size_t* service_ids_length_);

  ECALC_API int eCAL_Registration_GetServerInfo(const struct eCAL_SServiceId* service_id_, struct eCAL_SServiceMethodInformation** service_method_info_, size_t* service_method_info_length_);

  ECALC_API int eCAL_Registration_GetClientIDs(struct eCAL_SServiceId** service_ids_, size_t* service_ids_length_);

  ECALC_API int eCAL_Registration_GetClientInfo(const struct eCAL_SServiceId* service_id_, struct eCAL_SServiceMethodInformation** service_method_info_, size_t* service_method_info_length_);

  ECALC_API int eCAL_Registration_GetPublishedTopicNames(char*** topic_names_, size_t* topic_names_length_);

  ECALC_API int eCAL_Registration_GetSubscribedTopicNames(char*** topic_names_, size_t* topic_names_length_);

  ECALC_API int eCAL_Registration_GetServerMethodNames(struct eCAL_Registration_SServiceMethod** server_method_names_, size_t* server_method_names_length_);

  ECALC_API int eCAL_Registration_GetClientMethodNames(struct eCAL_Registration_SServiceMethod** client_method_names_, size_t* client_method_names_length_);

  ECALC_API eCAL_Registration_CallbackToken eCAL_Registration_AddPublisherEventCallback(eCAL_Registration_TopicEventCallbackT callback_);
  ECALC_API void eCAL_Registration_RemPublisherEventCallback(eCAL_Registration_CallbackToken token_);

  ECALC_API eCAL_Registration_CallbackToken eCAL_Registration_AddSubscriberEventCallback(eCAL_Registration_TopicEventCallbackT callback_);
  ECALC_API void eCAL_Registration_RemSubscriberEventCallback(eCAL_Registration_CallbackToken token_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_registration_h_included*/