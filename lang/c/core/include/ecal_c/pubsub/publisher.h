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

#include <ecal_c/callback.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  /**
   * @brief Instance a publisher.
   *
   * @return  Handle to new publisher or NULL if failed.
  **/
  ECALC_API ECAL_HANDLE eCAL_Pub_New();

  /**
   * @brief Create a publisher. 
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
  ECALC_API int eCAL_Pub_Create(ECAL_HANDLE handle_, const char* topic_name_, const char* topic_type_name_, const char* topic_type_encoding_, const char* topic_desc_, int topic_desc_len_);

  /**
   * @brief Destroy a publisher. 
   *
   * @param handle_  Publisher handle. 
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Pub_Destroy(ECAL_HANDLE handle_);

  /**
   * @brief Sets publisher attribute. 
   *
   * @param handle_             Publisher handle. 
   * @param attr_name_          Attribute name. 
   * @param attr_name_len_      Attribute name length. 
   * @param attr_value_         Attribute value. 
   * @param attr_value_len_     Attribute value length. 
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Pub_SetAttribute(ECAL_HANDLE handle_, const char* attr_name_, int attr_name_len_, const char* attr_value_, int attr_value_len_);

  /**
   * @brief Removes publisher attribute. 
   *
   * @param handle_             Publisher handle. 
   * @param attr_name_          Attribute name. 
   * @param attr_name_len_      Attribute name length. 
   *
   * @return  None zero if succeeded.
   * @experimental
  **/
  ECALC_API int eCAL_Pub_ClearAttribute(ECAL_HANDLE handle_, const char* attr_name_, int attr_name_len_);

  /**
   * @brief Set the specific topic id.
   *
   * @param handle_  Publisher handle.
   * @param id_      The topic id for subscriber side filtering (0 == no id).
   *
   * @return  True if it succeeds, false if it fails.
  **/
  ECALC_API int eCAL_Pub_SetID(ECAL_HANDLE handle_, long long id_);

  /**
   * @brief Query if the publisher is subscribed. 
   *
   * @param handle_  Publisher handle. 
   *
   * @return  None zero if subscribed. 
  **/
  ECALC_API int eCAL_Pub_IsSubscribed(ECAL_HANDLE handle_);

  /**
   * @brief Send a message to all subscribers. 
   *
   * @param handle_   Publisher handle. 
   * @param buf_      Buffer that contains content to send. 
   * @param buf_len_  Send buffer length. 
   * @param time_     Send time (-1 = use eCAL system time in us, default = -1).
   *
   * @return  Number of bytes sent. 
  **/
  ECALC_API int eCAL_Pub_Send(ECAL_HANDLE handle_, const void* const buf_, int buf_len_, long long time_);

  /**
   * @brief Add callback function for publisher events.
   *
   * @param handle_    Publisher handle.
   * @param type_      The event type to react on.
   * @param callback_  The callback function to add.
   * @param par_       User defined context that will be forwarded to the callback function.
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Pub_AddEventCallback(ECAL_HANDLE handle_, enum eCAL_Publisher_Event type_, PubEventCallbackCT callback_, void* par_);

  /**
   * @brief Remove callback function for publisher events.
   *
   * @param handle_  Publisher handle.
   * @param type_    The event type to remove.
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Pub_RemEventCallback(ECAL_HANDLE handle_, enum eCAL_Publisher_Event type_);

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
  ECALC_API int eCAL_Pub_Dump(ECAL_HANDLE handle_, void* buf_, int buf_len_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_pubsub_publisher_h_included*/
