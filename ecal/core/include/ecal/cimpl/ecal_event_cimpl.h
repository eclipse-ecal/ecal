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

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  /**
   * @brief Open a named or unnamed event.
   *
   * @param  event_name_  Event name ("" == unnamed).
   *
   * @return  Handle to opened event or NULL if failed.
  **/
  ECALC_API ECAL_HANDLE eCAL_Event_gOpenEvent(const char* event_name_);

  /**
   * @brief Close an event.
   *
   * @param handle_  Event handle.
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Event_gCloseEvent(ECAL_HANDLE handle_);

  /**
   * @brief Set an event active.
   *
   * @param handle_  Event handle.
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Event_gSetEvent(ECAL_HANDLE handle_);

  /**
   * @brief Wait for an event with timeout.
   *
   * @param handle_   Event handle.
   * @param timeout_  Timeout in ms (-1 == infinite).
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Event_gWaitForEvent(ECAL_HANDLE handle_, long timeout_);

  /**
   * @brief Check whether an event is valid or not.
   *
   * @param handle_  Event handle.
   *
   * @return  None zero if event is valid.
  **/
  ECALC_API int eCAL_Event_gEventIsValid(ECAL_HANDLE handle_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_event_cimpl_h_included*/
