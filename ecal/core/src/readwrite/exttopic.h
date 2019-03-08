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
 * @brief  eCAL extended topic
**/

#pragma once

#include <chrono>
#include <unordered_map>

namespace eCAL
{
  struct ExtTopic
  {
    typedef std::chrono::steady_clock clock;

    ExtTopic()
    {
      direction = none;
      tsize     = 0;
      tclock    = 0;
      ttime     = clock::time_point(std::chrono::nanoseconds(0));
      dfreq     = 0;
      updated   = false;
    }
    enum ePubSub
    {
      none       = 0,
      publisher  = 1,
      subscriber = 2
    };
    ePubSub           direction;
    int               tsize;
    int               tclock;
    clock::time_point ttime;
    clock::duration   dtime;
    int               dfreq;
    bool              updated;
  };
  typedef std::unordered_map<std::string, ExtTopic> ExtMapT;
};
