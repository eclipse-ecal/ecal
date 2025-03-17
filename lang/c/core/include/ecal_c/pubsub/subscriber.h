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
#include <ecal_c/pubsub/types.h>
#include <ecal_c/config/subscriber.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  typedef struct eCAL_Subscriber eCAL_Subscriber;

  /**
   * @brief Creates a new subscriber instance.
   *
   * @param topic_name_      Unique topic name.
   * @param data_type_info_  Topic data type information (encoding, type, descriptor). Optional, can be NULL.
   * @param event_callback_  Callback for subscriber events. Optional, can be NULL.
   * @param config_          Configuration parameter. Optional, can be NULL.
   * 
   * @return Subscriber handle if succeeded, otherwise NULL. The handle needs to be deleted by eCAL_Subscriber_Delete().
  **/
  ECALC_API eCAL_Subscriber* eCAL_Subscriber_New(const char* topic_name_, const struct eCAL_SDataTypeInformation* data_type_information_, const eCAL_SubEventCallbackT sub_event_callback, const struct eCAL_Subscriber_Configuration* subscriber_configuration_);
  
  /**
   * @brief Deletes a subscriber instance
   *
   * @param subscriber_  Subscriber handle.
  **/
  ECALC_API void eCAL_Subscriber_Delete(eCAL_Subscriber* subscriber_);

  /**
   * @brief Set/overwrite callback function for incoming receives.
   *
   * @param subscriber_      Subscriber handle.
   * @param callback_        The callback function to set.
   * @param user_argument_   User argument that is forwarded to the callback. Optional, can be NULL.
   *
   * @return Zero if succeeded, non-zero otherwise.
  **/
  ECALC_API int eCAL_Subscriber_SetReceiveCallback(eCAL_Subscriber* subscriber_, eCAL_ReceiveCallbackT callback_, void* user_argument_);

  /**
   * @brief Remove callback function for incoming receives.
   *
   * @param subscriber_  Subscriber handle.
   * 
   * @return Zero if succeeded, non-zero otherwise.
  **/
  ECALC_API int eCAL_Subscriber_RemoveReceiveCallback(eCAL_Subscriber* subscriber_);

  /**
   * @brief Query the number of connected publishers.
   *
   * @param subscriber_  Subscriber handle.
   * 
   * @return Number of publishers.
  **/
  ECALC_API size_t eCAL_Subscriber_GetPublisherCount(eCAL_Subscriber* subscriber_);

  /**
   * @brief Retrieve the topic name.
   *
   * @param subscriber_  Subscriber handle.
   * 
   * @return  The topic name.
  **/
  ECALC_API const char* eCAL_Subscriber_GetTopicName(eCAL_Subscriber* subscriber_);

  /**
   * @brief Retrieve the topic id.
   *
   * @param subscriber_  Subscriber handle.
   * 
   * @return  The topic id.
  **/
  ECALC_API const struct eCAL_STopicId* eCAL_Subscriber_GetTopicId(eCAL_Subscriber* subscriber_);

  /**
   * @brief Retrieve the topic information.
   * 
   * @param subscriber_  Subscriber handle.
   *
   * @return  The topic information.
  **/
  ECALC_API const struct eCAL_SDataTypeInformation* eCAL_Subscriber_GetDataTypeInformation(eCAL_Subscriber* subscriber_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_pubsub_subscriber_h_included*/
