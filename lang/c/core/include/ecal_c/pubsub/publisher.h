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
 * @file   ecal_c/pubsub/publisher.h
 * @brief  eCAL publisher c interface
**/

#ifndef ecal_c_pubsub_publisher_h_included
#define ecal_c_pubsub_publisher_h_included

#include <ecal_c/export.h>
#include <ecal_c/types.h>
#include <ecal_c/config/publisher.h>
#include <ecal_c/pubsub/types.h>
#include <ecal_c/pubsub/payload_writer.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  typedef struct eCAL_Publisher eCAL_Publisher;

  /**
   * @brief Creates a new publisher instance
   *
   * @param topic_name_          Unique topic name.
   * @param data_type_info_      Topic data type information (encoding, type, descriptor). Optional, can be NULL.
   * @param pub_event_callback_  Publisher event callback funtion. Optional, can be NULL.
   * @param config_              Configuration parameter. Optional, can be NULL.
   * 
   * @return Publisher handle is returned on success, otherwise NULL. The handle needs to be deleted by eCAL_Publisher_Delete().
  **/
  ECALC_API eCAL_Publisher* eCAL_Publisher_New(const char* topic_name_, const struct eCAL_SDataTypeInformation* data_type_information_, const eCAL_PubEventCallbackT pub_event_callback, const struct eCAL_Publisher_Configuration* publisher_configuration_);

  /**
   * @brief Deletes a publisher instance
   *
   * @param publisher_          Publisher handle.
  **/
  ECALC_API void eCAL_Publisher_Delete(eCAL_Publisher* publisher_);

  /**
   * @brief Send a message to all subscribers.
   *
   * @param publisher_          Publisher handle.
   * @param buffer_             Pointer to content buffer.
   * @param buffer_len_         Length ofthe buffer.
   * @param timestamp_          Send timestamp in microseconds. Optional, can be NULL.
   *
   * @return Zero if succeeded, non-zero otherwise.
  **/
  ECALC_API int eCAL_Publisher_Send(eCAL_Publisher* publisher_, const void* buffer_, size_t buffer_len_, const long long* timestamp_);

  /**
   * @brief Send a message to all subscribers.
   *
   * @param publisher_         Publisher handle.
   * @param payload_writer_    Payload writer.
   * @param timestamp_         Send timestamp in microseconds. Optional, can be NULL.
   *
   * @return Zero if succeeded, non-zero otherwise.
  **/
  ECALC_API int eCAL_Publisher_SendPayloadWriter(eCAL_Publisher* publisher_, const struct eCAL_PayloadWriter* payload_writer_, const long long* timestamp_);

  /**
   * @brief Query the number of subscribers.
   *
   * @param publisher_ Publisher handle.
   * 
   * @return Number of subscribers.
  **/
  ECALC_API size_t eCAL_Publisher_GetSubscriberCount(eCAL_Publisher* publisher_);

  /**
   * @brief Retrieve the topic name.
   *
   * @param publisher_  Publisher handle.
   * 
   * @return The topic name.
  **/
  ECALC_API const char* eCAL_Publisher_GetTopicName(eCAL_Publisher* publisher_);

  /**
   * @brief Retrieve the topic id.
   *
   * @param publisher_  Publisher handle.
   * 
   * @return The topic id.
  **/
  ECALC_API const struct eCAL_STopicId* eCAL_Publisher_GetTopicId(eCAL_Publisher* publisher_);

  /**
   * @brief Gets description of the connected topic.
   *
   * @param publisher_  Publisher handle.
   * 
   * @return The topic information.
  **/
  ECALC_API const struct eCAL_SDataTypeInformation* eCAL_Publisher_GetDataTypeInformation(eCAL_Publisher* publisher_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_pubsub_publisher_h_included*/
