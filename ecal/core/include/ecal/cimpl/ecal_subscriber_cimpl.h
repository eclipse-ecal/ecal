/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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
 * @file   ecal_subscriber_cimpl.h
 * @brief  eCAL subscriber c interface
**/

#ifndef ecal_subscriber_cimpl_h_included
#define ecal_subscriber_cimpl_h_included

#include <ecal/cimpl/ecal_callback_cimpl.h>

#include "ecal_qos_cimpl.h"
#include "ecal_tlayer_cimpl.h"

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
   * @param handle_          Subscriber handle.
   * @param topic_name_      Unique topic name.
   * @param topic_type_      Topic type name. 
   * @param topic_desc_      Topic description.
   * @param topic_desc_len_  Topic type description length.
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Sub_Create(ECAL_HANDLE handle_, const char* topic_name_, const char* topic_type_, const char* topic_desc_, int topic_desc_len_);

  /**
   * @brief Destroy a subscriber. 
   *
   * @param handle_  Subscriber handle. 
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Sub_Destroy(ECAL_HANDLE handle_);

  /**
   * @brief Set subscriber quality of service attributes.
   *
   * @param handle_  Subscriber handle.
   * @param qos_     Quality of service policies.
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Sub_SetQOS(ECAL_HANDLE handle_, struct SReaderQOSC qos_);

  /**
   * @brief Get subscriber quality of service attributes.
   *
   * @param handle_  Subscriber handle.
   * @param qos_     Quality of service policies.
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Sub_GetQOS(ECAL_HANDLE handle_, struct SReaderQOSC* qos_);

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
   * @brief Receive a message from the publisher. 
   *
   * @param       handle_       Subscriber handle. 
   * @param [out] buf_          Buffer to store the received message content.
   * @param       buf_len_      Length of the receive buffer or ECAL_ALLOCATE_4ME if
   *                            eCAL should allocate the buffer for you (see eCAL_FreeMem).
   * @param [out] time_         Time from publisher in us.
   * @param       rcv_timeout_  Maximum time before receive operation returns (in milliseconds, -1 means infinite).
   *
   * @return  Length of received buffer. 
  **/
  ECALC_API int eCAL_Sub_Receive(ECAL_HANDLE handle_, void** buf_, int buf_len_, long long* time_, int rcv_timeout_);

  /**
   * @brief Add callback function for incoming receives. 
   *
   * @param handle_    Subscriber handle. 
   * @param callback_  The callback function to add.
   * @param par_       User defined context that will be forwarded to the callback function.  
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Sub_AddReceiveCallbackC(ECAL_HANDLE handle_, ReceiveCallbackCT callback_, void* par_);

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
  ECALC_API int eCAL_Sub_AddEventCallbackC(ECAL_HANDLE handle_, enum eCAL_Subscriber_Event type_, SubEventCallbackCT callback_, void* par_);

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
   * @brief Set the timeout parameter for triggering
   *          the timeout callback.
   *
   * @param handle_   Subscriber handle.
   * @param timeout_  The timeout in milliseconds.
   *
   * @return  True if succeeded, false if not.
  **/
  ECALC_API int eCAL_Sub_SetTimeout(ECAL_HANDLE handle_, int timeout_);

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

#endif /*ecal_subscriber_cimpl_h_included*/
