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
 * @file   ecal_event.h
 * @brief  eCAL event interface
**/

#pragma once

#include <ecal/ecal_os.h>
#include <ecal/ecal_eventhandle.h>
#include <string>

namespace eCAL
{
  /**
   * @brief Open a named or unnamed event.
   *
   * @param [out] event_       Returned event struct.
   * @param       event_name_  Event name ("" == unnamed).
   *
   * @return  True if succeeded.
  **/
  ECAL_API bool gOpenEvent(eCAL::EventHandleT* event_, const std::string& event_name_ = "");

  /**
   * @brief Close an event.
   *
   * @param event_  Event struct.
   *
   * @return  True if succeeded.
  **/
  ECAL_API bool gCloseEvent(const EventHandleT& event_);

  /**
   * @brief Set an event active.
   *
   * @param event_  Event struct.
   *
   * @return  True if succeeded.
  **/
  ECAL_API bool gSetEvent(const EventHandleT& event_);

  /**
   * @brief Wait for an event with timeout.
   *
   * @param event_    Event struct.
   * @param timeout_  Timeout in ms (-1 == infinite).
   *
   * @return  True if succeeded.
  **/
  ECAL_API bool gWaitForEvent(const EventHandleT& event_, long timeout_);

  /**
   * @brief Invalidate an event.
   *
   * @return  True if event is invalidated.
  **/
  ECAL_API bool gInvalidateEvent(EventHandleT* event_);

  /**
   * @brief Check whether an event is valid or not.
   *
   * @return  True if event is valid.
  **/
  ECAL_API bool gEventIsValid(const EventHandleT& event_);
}

#ifdef ECAL_C_DLL

#include <ecal/ecalc.h>

namespace eCAL
{
  inline bool gOpenEvent(eCAL::EventHandleT* event_, const std::string& event_name_)
  {
    if(event_ == nullptr) return(false);
    ECAL_HANDLE event = eCAL_Event_gOpenEvent(event_name_.c_str());
    if(event == nullptr) return(false);
    event_->handle = event;
    event_->name   = event_name_;
    return(true);
  }

  inline bool gCloseEvent(const EventHandleT& event_)
  {
    return(eCAL_Event_gCloseEvent(event_.handle) != 0);
  }

  inline bool gSetEvent(const EventHandleT& event_)
  {
    return(eCAL_Event_gSetEvent(event_.handle) != 0);
  }

  inline bool gWaitForEvent(const EventHandleT& event_, long timeout_)
  {
    return(eCAL_Event_gWaitForEvent(event_.handle, timeout_) != 0);
  }

  inline bool gInvalidateEvent(eCAL::EventHandleT* event_)
  {
    if(event_ == nullptr) return(false);
    if(event_->handle == nullptr) return(false);
    event_->handle = nullptr;
    return(true);
  }

  inline bool gEventIsValid(const EventHandleT& event_)
  {
    return(eCAL_Event_gEventIsValid(event_.handle) != 0);
  }
}

#endif /* ECAL_C_DLL */
