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
 * @file   ecal_c/pubsub/subscriber.h
 * @brief  eCAL subscriber c interface
**/

#ifndef ecal_c_pubsub_subscriber_h_included
#define ecal_c_pubsub_subscriber_h_included

#include <ecal_c/export.h>
#include <ecal_c/types.h>

#include <ecal_c/callback.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/

  typedef struct eCAL_Subscriber eCAL_Subscriber;

  struct eCAL_SReceiveCallbackData
  {
    const void* buffer;
    size_t buffer_size;
    int64_t send_timestamp;
    int64_t send_clock;
  };

  enum eCAL_eSubscriberEvent
  {
    eCAL_eSubscriberEvent_none,
    eCAL_eSubscriberEvent_connected,
    eCAL_eSubscriberEvent_disconnected,
    eCAL_eSubscriberEvent_dropped
  };

  struct eCAL_SSubEventCallbackData
  {
    enum eCAL_eSubscriberEvent event_type;
    long long event_time;
    struct eCAL_SDataTypeInformation publisher_datatype;
  };

  typedef void (*eCAL_ReceiveCallbackT)(const struct eCAL_STopicId*, const struct eCAL_SDataTypeInformation*, const struct eCAL_SReceiveCallbackData*);
  typedef void (*eCAL_SubEventCallbackT)(const struct eCAL_STopicId*, const struct eCAL_SSubEventCallbackData*);

  ECALC_API eCAL_Subscriber* eCAL_Subscriber_New(const char* topic_name_, const struct eCAL_SDataTypeInformation* data_type_information_, const struct eCAL_Subscriber_Configuration* subscriber_configuration_);
  ECALC_API eCAL_Subscriber* eCAL_Subscriber_New2(const char* topic_name_, const struct eCAL_SDataTypeInformation* data_type_information_, const eCAL_SubEventCallbackT sub_event_callback, const struct eCAL_Subscriber_Configuration* subscriber_configuration_);
  ECALC_API void eCAL_Subscriber_Delete(eCAL_Subscriber* subscriber_);

  ECALC_API int eCAL_Subscriber_SetReceiveCallback(eCAL_Subscriber* subscriber_, eCAL_ReceiveCallbackT callback_);
  ECALC_API int eCAL_Subscriber_RemoveReceiveCallback(eCAL_Subscriber* subscriber_);

  ECALC_API size_t eCAL_Subscriber_GetPublisherCount(eCAL_Subscriber* subscriber_);

  ECALC_API const char* eCAL_Subscriber_GetTopicName(eCAL_Subscriber* subscriber_);

  ECALC_API struct eCAL_STopicId* eCAL_Subscriber_GetTopicId(eCAL_Subscriber* subscriber_);
  //ECALC_API void eCAL_STopicId_Free(struct eCAL_STopicId* topic_id_);

  ECALC_API struct eCAL_SDataTypeInformation* eCAL_Subscriber_GetDataTypeInformation(eCAL_Subscriber* subscriber_);
  //ECALC_API void eCAL_SDataTypeInformation_Free(struct eCAL_SDataTypeInformation* data_type_information_);

  //ECALC_API struct eCAL_Subscriber_Configuration* eCAL_GetSubscriberConfiguration();
  //ECALC_API void eCAL_Subscriber_Configuration_Free(eCAL_Subscriber_Configuration* subscriber_configuration_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_pubsub_subscriber_h_included*/
