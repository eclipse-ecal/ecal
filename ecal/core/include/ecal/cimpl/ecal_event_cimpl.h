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
 * @file   ecal_event_cimpl.h
 * @brief  eCAL event c interface
**/

#ifndef ecal_event_cimpl_h_included
#define ecal_event_cimpl_h_included

#ifdef _MSC_VER
#pragma message("WARNING: The header file cimpl/ecal_event_cimpl.h is deprecated. It will be removed in future eCAL versions.")
#endif /*_MSC_VER*/
#ifdef __GNUC__
#pragma message "WARNING: The header file cimpl/ecal_event_cimpl.h is deprecated. It will be removed in future eCAL versions."
#endif /* __GNUC__ */

#include <ecal/ecal_os.h>
#include <ecal/ecalc_types.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  /**
   * @brief Open a named or unnamed event.
   * @deprecated Will be removed in future eCAL versions.
   *
   * @param  event_name_  Event name ("" == unnamed).
   *
   * @return  Handle to opened event or NULL if failed.
  **/
  ECALC_API_DEPRECATED ECAL_HANDLE eCAL_Event_gOpenEvent(const char* event_name_);

  /**
   * @brief Close an event.
   * @deprecated Will be removed in future eCAL versions.
   *
   * @param handle_  Event handle.
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API_DEPRECATED int eCAL_Event_gCloseEvent(ECAL_HANDLE handle_);

  /**
   * @brief Set an event active.
   * @deprecated Will be removed in future eCAL versions.
   *
   * @param handle_  Event handle.
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API_DEPRECATED int eCAL_Event_gSetEvent(ECAL_HANDLE handle_);

  /**
   * @brief Wait for an event with timeout.
   * @deprecated Will be removed in future eCAL versions.
   *
   * @param handle_   Event handle.
   * @param timeout_  Timeout in ms (-1 == infinite).
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API_DEPRECATED int eCAL_Event_gWaitForEvent(ECAL_HANDLE handle_, long timeout_);

  /**
   * @brief Check whether an event is valid or not.
   * @deprecated Will be removed in future eCAL versions.
   *
   * @param handle_  Event handle.
   *
   * @return  None zero if event is valid.
  **/
  ECALC_API_DEPRECATED int eCAL_Event_gEventIsValid(ECAL_HANDLE handle_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_event_cimpl_h_included*/
