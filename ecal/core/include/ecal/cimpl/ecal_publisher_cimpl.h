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
 * @file   ecal_publisher_cimpl.h
 * @brief  eCAL publisher c interface
**/

#ifndef ecal_publisher_cimpl_h_included
#define ecal_publisher_cimpl_h_included

#include <ecal/cimpl/ecal_callback_cimpl.h>

#include "ecal_qos_cimpl.h"
#include "ecal_tlayer_cimpl.h"

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
   * @param handle_          Publisher handle.
   * @param topic_name_      Unique topic name.
   * @param topic_type_      Topic type name. 
   * @param topic_desc_      Topic type description. 
   * @param topic_desc_len_  Topic type description length. 
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Pub_Create(ECAL_HANDLE handle_, const char* topic_name_, const char* topic_type_, const char* topic_desc_, int topic_desc_len_);

  /**
   * @brief Destroy a publisher. 
   *
   * @param handle_  Publisher handle. 
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Pub_Destroy(ECAL_HANDLE handle_);

  /**
   * @brief Setup topic type description. 
   *
   * @param handle_          Publisher handle. 
   * @param topic_desc_      Topic type description. 
   * @param topic_desc_len_  Topic type description length. 
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Pub_SetDescription(ECAL_HANDLE handle_, const char* topic_desc_, int topic_desc_len_);

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
   * @brief Share topic type.
   *
   * @param handle_ Publisher handle. 
   * @param state_  Set type share mode (none zero == share type).
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Pub_ShareType(ECAL_HANDLE handle_, int state_);

  /**
   * @brief Share topic description.
   *
   * @param handle_ Publisher handle. 
   * @param state_  Set description share mode (none zero == share description).
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Pub_ShareDescription(ECAL_HANDLE handle_, int state_);

  /**
   * @brief Set publisher quality of service attributes.
   *
   * @param handle_  Publisher handle.
   * @param qos_     Quality of service policies.
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Pub_SetQOS(ECAL_HANDLE handle_, struct SWriterQOSC qos_);

  /**
   * @brief Get publisher quality of service attributes.
   *
   * @param handle_  Publisher handle.
   * @param qos_     Quality of service policies.
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Pub_GetQOS(ECAL_HANDLE handle_, struct SWriterQOSC* qos_);

  /**
   * @brief Set publisher send mode for specific transport layer.
   *
   * @param handle_  Publisher handle.
   * @param layer_   Transport layer.
   * @param mode_    Send mode.
   *
   * @return  True if it succeeds, false if it fails.
  **/
  ECALC_API int eCAL_Pub_SetLayerMode(ECAL_HANDLE handle_, enum eTransportLayerC layer_, enum eSendModeC mode_);

  /**
   * @brief Set publisher maximum transmit bandwidth for the udp layer.
   *
   * @param handle_     Publisher handle.
   * @param bandwidth_  Maximum bandwidth in bytes/s (-1 == unlimited).
   *
   * @return  True if it succeeds, false if it fails.
  **/
  ECALC_API int eCAL_Pub_SetMaxBandwidthUDP(ECAL_HANDLE handle_, long bandwidth_);

  /**
   * @brief Set publisher maximum number of used shared memory buffers.
   *
   * @param buffering_  Maximum number of used buffers (needs to be greater than 1, default = 1).
   *
   * @return  True if it succeeds, false if it fails.
  **/
  ECALC_API int eCAL_Pub_ShmSetBufferCount(ECAL_HANDLE handle_, long buffering_);

  /**
   * @brief Enable zero copy shared memory trasnport mode.
   *
   * By default, the builtin shared memory layer is configured to make one memory copy
   * on the receiver side. That means the payload is copied by the internal eCAL memory pool manager
   * out of the memory file and the file is closed immediately after this.
   * The intention of this implementation is to free the file as fast as possible after reading
   * its content to allow other subscribing processes to access the content with minimal latency.
   * The different reading subscribers are fully decoupled and can access their memory copy
   * independently.
   *
   * If ShmEnableZeroCopy is switched on no memory will be copied at all. The user message callback is
   * called right after opening the memory file. A direct pointer to the memory payload is forwarded
   * and can be processed with no latency. The memory file will be closed after the user callback function
   * returned. The advantage of this configuration is a much higher performance for large payloads (> 1024 kB).
   * The disadvantage of this configuration is that in the time when the callback is executed the memory file
   * is blocked for other subscribers and for writing publishers too. Maybe this can be eliminated
   * by a better memory file read/write access implementation (lock free read) in future releases.
   *
   * Today, for specific scenarios (1:1 pub/sub connections with large payloads for example) this feature
   * can increase the performance remarkable. But please keep in mind to return from the message callback function
   * as fast as possible to not delay subsequent read/write access operations.
   *
   * @state_  Set type zero copy mode for shared memory trasnport layer (true == zero copy enabled).
   *
   * @return  True if it succeeds, false if it fails.
  **/
  ECALC_API int eCAL_Pub_ShmEnableZeroCopy(ECAL_HANDLE handle_, int state_);

  /**
   * @brief Set publisher maximum transmit bandwidth for the udp layer.
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

#endif /*ecal_publisher_cimpl_h_included*/
