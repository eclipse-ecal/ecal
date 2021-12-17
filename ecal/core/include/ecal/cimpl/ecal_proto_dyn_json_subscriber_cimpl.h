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
 * @file   ecal_proto_dyn_json_subscriber_cimpl.h
 * @brief  eCAL subscriber c interface
**/

#ifndef ecal_proto_dyn_json_subscriber_cimpl_h_included
#define ecal_proto_dyn_json_subscriber_cimpl_h_included

#include <ecal/cimpl/ecal_callback_cimpl.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  /**
   * @brief Create a subscriber. 
   *
   * @param topic_name_  Unique topic name. 
   *
   * @return  Handle to created subscriber or NULL if failed.
  **/
  ECALC_API ECAL_HANDLE eCAL_Proto_Dyn_JSON_Sub_Create(const char* topic_name_);

  /**
   * @brief Destroy a subscriber. 
   *
   * @param handle_  Subscriber handle. 
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Proto_Dyn_JSON_Sub_Destroy(ECAL_HANDLE handle_);

  /**
   * @brief Add callback function for incoming receives. 
   *
   * @param handle_    Subscriber handle. 
   * @param callback_  The callback function to add.
   * @param par_       User defined context that will be forwarded to the callback function.  
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Proto_Dyn_JSON_Sub_AddReceiveCallback(ECAL_HANDLE handle_, ReceiveCallbackCT callback_, void* par_);

  /**
   * @brief Remove callback function for incoming receives. 
   *
   * @param  handle_  Subscriber handle. 
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Proto_Dyn_JSON_Sub_RemReceiveCallback(ECAL_HANDLE handle_);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_proto_dyn_json_subscriber_cimpl_h_included*/
