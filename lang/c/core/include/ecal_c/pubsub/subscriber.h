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
  /**
   * @brief Instance a subscriber.
   *
   * @return  Handle to new subscriber or NULL if failed.
  **/
  ECALC_API ECAL_HANDLE eCAL_Sub_New();

  /**
   * @brief Create a subscriber.
   *
   * @param handle_                   Publisher handle.
   * @param topic_name_               Unique topic name.
   * @param topic_type_name_          Topic type name     (like 'string', 'person').
   * @param topic_type_encoding_      Topic type encoding (like 'base', 'proto').
   * @param topic_desc_               Topic type description.
   * @param topic_desc_len_           Topic type description length.
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Sub_Create(ECAL_HANDLE handle_, const char* topic_name_, const char* topic_type_name_, const char* topic_type_encoding_, const char* topic_desc_, int topic_desc_len_);

  /**
   * @brief Destroy a subscriber. 
   *
   * @param handle_  Subscriber handle. 
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Sub_Destroy(ECAL_HANDLE handle_);

  /**
   * @brief Set a set of id's to prefiltering topics (see eCAL_Pub_SetID).
   *
   * @param handle_    Subscriber handle.
   * @param id_array_  Array of id's (Use nullptr to reset id's).
   * @param id_num_    Number of id's.
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Sub_SetID(ECAL_HANDLE handle_, const long long* id_array_, const int id_num_);

  /**
   * @brief Sets subscriber attribute. 
   *
   * @param handle_             Subscriber handle. 
   * @param attr_name_          Attribute name. 
   * @param attr_name_len_      Attribute name length. 
   * @param attr_value_         Attribute value. 
   * @param attr_value_len_     Attribute value length. 
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Sub_SetAttribute(ECAL_HANDLE handle_, const char* attr_name_, int attr_name_len_, const char* attr_value_, int attr_value_len_);

  /**
   * @brief Removes subscriber attribute. 
   *
   * @param handle_             Subscriber handle. 
   * @param attr_name_          Attribute name. 
   * @param attr_name_len_      Attribute name length. 
   *
   * @return  None zero if succeeded.
   * @experimental
  **/
  ECALC_API int eCAL_Sub_ClearAttribute(ECAL_HANDLE handle_, const char* attr_name_, int attr_name_len_);

  /**
   * @brief Receive a message from the publisher in a preallocated buffer.
   *
   * @param       handle_       Subscriber handle. 
   * @param       buf_          Buffer to store the received message content.
   * @param       buf_len_      Length of the receive buffer.
   * @param [out] time_         Time from publisher in us.
   * @param       rcv_timeout_  Maximum time before receive operation returns (in milliseconds, -1 means infinite).
   *
   * @return  Length of received buffer. 
  **/
  ECALC_API int eCAL_Sub_Receive_ToBuffer(ECAL_HANDLE handle_, void* buf_, int buf_len_, long long* time_, int rcv_timeout_);

  /**
   * @brief Receive a message from the publisher and let eCAL allocate the memory.
   *
   * @param       handle_       Subscriber handle. 
   * @param [out] buf_          Buffer to store the pointer to the received message content.
   *                            You need to free the memory finally calling eCAL_FreeMem.
   * @param [out] time_         Time from publisher in us.
   * @param       rcv_timeout_  Maximum time before receive operation returns (in milliseconds, -1 means infinite).
   *
   * @return  Length of received buffer. 
  **/
  ECALC_API int eCAL_Sub_Receive_Alloc(ECAL_HANDLE handle_, void** buf_, long long* time_, int rcv_timeout_);

  /**
   * @brief Receive a message from the publisher and let eCAL allocate the memory (able to process zero length buffer).
   *
   * @param       handle_       Subscriber handle.
   * @param [out] buf_          Buffer to store the received message content.
   * @param [out] buf_len_      Length of allocated buffer,
   *                            eCAL is allocating the buffer for you, use ecal_free_mem to free the buffer finally.
   *                            You need to free the memory finally calling eCAL_FreeMem.
   * @param [out] time_         Time from publisher in us.
   * @param       rcv_timeout_  Maximum time before receive operation returns (in milliseconds, -1 means infinite).
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Sub_Receive_Buffer_Alloc(ECAL_HANDLE handle_, void** buf_, int* buf_len_, long long* time_, int rcv_timeout_);

  /**
   * @brief Add callback function for incoming receives. 
   * @since eCAL 5.10.0
   *
   * @param handle_    Subscriber handle. 
   * @param callback_  The callback function to add.
   * @param par_       User defined context that will be forwarded to the callback function.  
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Sub_AddReceiveCallback(ECAL_HANDLE handle_, ReceiveCallbackCT callback_, void* par_);

  /**
   * @brief Remove callback function for incoming receives. 
   *
   * @param  handle_  Subscriber handle. 
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Sub_RemReceiveCallback(ECAL_HANDLE handle_);

  /**
   * @brief Add callback function for subscriber events.
   *
   * @param handle_    Subscriber handle.
   * @param type_      The event type to react on.
   * @param callback_  The callback function to add.
   * @param par_       User defined context that will be forwarded to the callback function.
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Sub_AddEventCallback(ECAL_HANDLE handle_, enum eCAL_Subscriber_Event type_, SubEventCallbackCT callback_, void* par_);

  /**
   * @brief Remove callback function for subscriber events.
   *
   * @param handle_  Subscriber handle.
   * @param type_    The event type to remove.
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Sub_RemEventCallback(ECAL_HANDLE handle_, enum eCAL_Subscriber_Event type_);

  /**
   * @brief Gets type name of the connected topic.
   *
   * @param       handle_   Subscriber handle.
   * @param [out] buf_      Pointer to store the subscriber type name string.
   * @param       buf_len_  Length of allocated buffer or ECAL_ALLOCATE_4ME if
   *                        eCAL should allocate the buffer for you (see eCAL_FreeMem).
   *
   * @return  Type name buffer length or zero if failed.
  **/
  ECALC_API int eCAL_Sub_GetTypeName(ECAL_HANDLE handle_, void* buf_, int buf_len_);

  /**
   * @brief Gets encoding of the connected topic.
   *
   * @param       handle_   Subscriber handle.
   * @param [out] buf_      Pointer to store the subscriber encoding string.
   * @param       buf_len_  Length of allocated buffer or ECAL_ALLOCATE_4ME if
   *                        eCAL should allocate the buffer for you (see eCAL_FreeMem).
   *
   * @return  Encoding buffer length or zero if failed.
  **/
  ECALC_API int eCAL_Sub_GetEncoding(ECAL_HANDLE handle_, void* buf_, int buf_len_);

  /**
   * @brief Gets description of the connected topic.
   *
   * @param       handle_   Subscriber handle.
   * @param [out] buf_      Pointer to store the subscriber description string.
   * @param       buf_len_  Length of allocated buffer or ECAL_ALLOCATE_4ME if
   *                        eCAL should allocate the buffer for you (see eCAL_FreeMem).
   *
   * @return  Description buffer length or zero if failed.
  **/
  ECALC_API int eCAL_Sub_GetDescription(ECAL_HANDLE handle_, void* buf_, int buf_len_);

  /**
   * @brief Dump the whole class state into a string buffer. 
   *
   * @param handle_         Publisher handle. 
   * @param [out] buf_      Pointer to store the monitoring information. 
   * @param       buf_len_  Length of allocated buffer or ECAL_ALLOCATE_4ME if
   *                        eCAL should allocate the buffer for you (see eCAL_FreeMem). 
   *
   * @return  Dump buffer length or zero if failed. 
  **/
  ECALC_API int eCAL_Sub_Dump(ECAL_HANDLE handle_, void* buf_, int buf_len_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_pubsub_subscriber_h_included*/
