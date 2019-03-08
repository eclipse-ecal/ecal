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
 * @file   ecal_util_cimpl.h
 * @brief  eCAL utility c interface
**/

#ifndef ecal_util_cimpl_h_included
#define ecal_util_cimpl_h_included

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  /**
   * @brief Send shutdown event to specified local user process using it's unit name.
   *
   * @param unit_name_  Process unit name. 
  **/
  ECALC_API void eCAL_Util_ShutdownUnitName(const char* unit_name_);

  /**
   * @brief Send shutdown event to specified local user process using it's process id.
   *
   * @param process_id_  Process id. 
  **/
  ECALC_API void eCAL_Util_ShutdownProcessID(int process_id_);

  /**
   * @brief Send shutdown event to all local user processes.
  **/
  ECALC_API void eCAL_Util_ShutdownProcesses();

  /**
   * @brief Send shutdown event to all local core components.
  **/
  ECALC_API void eCAL_Util_ShutdownCore();

  /**
   * @brief Enable eCAL message loop back,
   *          that means subscriber will receive messages from
   *          publishers of the same process (default == false).
   *
   * @param state_  Switch on message loop back.. 
  **/
  ECALC_API void eCAL_Util_EnableLoopback(int state_);

  /**
   * @brief Gets type name of the specified topic. 
   *
   * @param       topic_name_      Topic name. 
   * @param [out] topic_type_      Pointer to store the type name information. 
   * @param       topic_type_len_  Length of allocated buffer or ECAL_ALLOCATE_4ME if
   *                               eCAL should allocate the buffer for you (see eCAL_FreeMem). 
   *
   * @return  Type name buffer length or zero if failed. 
  **/
  ECALC_API int  eCAL_Util_GetTypeName(const char* topic_name_, void* topic_type_, int topic_type_len_);

  /**
   * @brief Gets type description of the specified topic. 
   *
   * @param       topic_name_      Topic name. 
   * @param [out] topic_desc_      Pointer to store the type description information. 
   * @param       topic_desc_len_  Length of allocated buffer or ECAL_ALLOCATE_4ME if
   *                               eCAL should allocate the buffer for you (see eCAL_FreeMem). 
   *
   * @return  Type description buffer length or zero if failed. 
  **/
  ECALC_API int  eCAL_Util_GetDescription(const char* topic_name_, void* topic_desc_, int topic_desc_len_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_util_cimpl_h_included*/
