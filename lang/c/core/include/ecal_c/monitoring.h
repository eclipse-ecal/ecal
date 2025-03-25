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
#include <ecal_c/types/monitoring.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  /**
   * @brief Get monitoring as serialized protobuf buffer.
   *
   * @param [out] monitoring_buffer          Pointer to a protobuf serialized monitoring buffer. Must point to NULL and needs to be released by eCAL_Free().
   * @param [out] monitoring_buffer_length_  Length of the log buffer.
   * @param       entities_                  Entities to be included. Optional, can be NULL.
   *
   * @return Zero if succeeded, non-zero otherwise.
  **/
  ECALC_API int eCAL_Monitoring_GetMonitoringBuffer(void** monitoring_buffer_, size_t* monitoring_buffer_length_, const unsigned int* entities_);

  /**
   * @brief Get monitoring as deserialized structure.
   *
   * @param [out] monitoring_  Pointer to a deserialized monitoring structure. Must point to NULL and needs to be released by eCAL_Free().
   * @param       entities_    Entities to be included. Optional, can be NULL.
   *
   * @return Zero if succeeded, non-zero otherwise.
  **/
  ECALC_API int eCAL_Monitoring_GetMonitoring(struct eCAL_Monitoring_SMonitoring** monitoring_, const unsigned int* entities_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_monitoring_h_included*/
