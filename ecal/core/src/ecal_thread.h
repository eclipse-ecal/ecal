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
 * @brief  eCAL threading helper class
**/

#pragma once

#include <ecal/ecal_eventhandle.h>

#include <atomic>
#include <thread>
#include <functional>

namespace eCAL
{
  class CThread
  {
  public:
    CThread();
    virtual ~CThread();

    int Start(int period, std::function<int()> ext_caller_);
    int Stop();
    int Fire();

    bool IsRunning()        {return(m_tdata.is_running);};

  protected:
    struct ThreadData
    {
      ThreadData() :
         period(0)
       , is_running(false)
       , is_started(false)
       , do_stop(false)
      {
      };
      std::thread             thread;
      int                     period;
      EventHandleT            event;
      std::atomic<bool>       is_running;
      std::atomic<bool>       is_started;
      std::atomic<bool>       do_stop;
      std::function<int()>    ext_caller;
    };
    struct ThreadData m_tdata;

    static void HelperThread(void* par_);
  };
}
