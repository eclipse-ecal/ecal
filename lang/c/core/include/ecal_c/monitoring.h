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
 * @file   ecal_c/monitoring.h
 * @brief  eCAL monitoring c interface
**/

#ifndef ecal_c_monitoring_h_included
#define ecal_c_monitoring_h_included

#include <ecal_c/export.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  /**
   * @brief Get monitoring protobuf string. 
   *
   * @param [out] buf_      Pointer to store the monitoring information. 
   * @param       buf_len_  Length of allocated buffer or ECAL_ALLOCATE_4ME if
   *                        eCAL should allocate the buffer for you (see eCAL_FreeMem). 
   *
   * @return  Monitoring buffer length or zero if failed. 
  **/
  /**
   * @code
   *            // let eCAL allocate memory for the monitoring buffer and return the pointer to 'buf'
   *            void*     buf      = NULL;
   *            int       buf_len  = eCAL_Monitoring_GetMonitoring(&buf, ECAL_ALLOCATE_4ME);
   *            if(buf_len > 0)
   *            {
   *              ...
   *              // PROCESS THE BUFFER CONTENT HERE
   *              ...
   *              // finally free the allocated memory
   *              eCAL_FreeMem(buf);
   *            }
   * @endcode
  **/
  ECALC_API int eCAL_Monitoring_GetMonitoring(void* buf_, int buf_len_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_monitoring_h_included*/
