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
 * @file   ecal_event_internal.h
 * @brief  eCAL event interface (internal)
 *
 *         This file will be renamed back to ecal_event.h after removing event API from eCAL's public API.
**/

#pragma once

#include <ecal/ecal_eventhandle.h>

#include <string>

namespace eCAL
{
  /**
   * @brief Open a named event with ownership.
   *
   * @param [out] event_       Returned event struct.
   * @param       event_name_  Event name.
   * @param       ownership_   Event is owned by the caller and will be destroyed on CloseEvent
   *
   * @return  True if succeeded.
  **/
  bool gOpenNamedEvent(eCAL::EventHandleT* event_, const std::string& event_name_, bool ownership_);

  /**
   * @brief Open an unnamed event.
   *
   * @param [out] event_  Returned event struct.
   *
   * @return  True if succeeded.
  **/
  bool gOpenUnnamedEvent(eCAL::EventHandleT* event_);
}
