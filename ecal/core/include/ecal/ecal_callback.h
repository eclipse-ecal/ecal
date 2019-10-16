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
 * @file   ecal_callback.h
 * @brief  eCAL subscriber callback interface
**/

#pragma once

#include <ecal/cimpl/ecal_callback_cimpl.h>

#include <functional>

namespace eCAL
{
  /**
   * @brief eCAL subscriber receive callback struct.
  **/
  struct SReceiveCallbackData
  {
    SReceiveCallbackData()
    {
      buf   = nullptr;
      size  = 0;
      id    = 0;
      time  = 0;
      clock = 0;
    };
    void*     buf;
    long      size;
    long long id;
    long long time;
    long long clock;
  };

  typedef std::function<void(const char* topic_name_, const struct SReceiveCallbackData* data_)> ReceiveCallbackT;
  typedef std::function<void(void)> TimerCallbackT;
  typedef std::function<void(const char* sample_, int sample_size_)> RegistrationCallbackT;

  struct SPubEventCallbackData
  {
    SPubEventCallbackData()
    {
      type = pub_event_none;
      time = 0;
      clock = 0;
    };
    eCAL_Publisher_Event type;
    long long            time;
    long long            clock;
  };

  struct SSubEventCallbackData
  {
    SSubEventCallbackData()
    {
      type  = sub_event_none;
      time  = 0;
      clock = 0;
    };
    eCAL_Subscriber_Event type;
    long long             time;
    long long             clock;
  };

  typedef std::function<void(const char* topic_name_, const struct SPubEventCallbackData* data_)> PubEventCallbackT;
  typedef std::function<void(const char* topic_name_, const struct SSubEventCallbackData* data_)> SubEventCallbackT;
};
