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
 * @brief  thread interface - windows platform
**/

#include <ecal/ecal_event.h>

#include "ecal_thread.h"
#include <thread>

namespace eCAL
{
  CThread::CThread()
  {
  }

  CThread::~CThread()
  {
    try { Stop(); } catch(...) { /*??*/ }
  }

  int CThread::Start(int period_, std::function<int()> ext_caller_)
  {
    if(m_tdata.is_started) return(0);

    gOpenEvent(&m_tdata.event);
    m_tdata.do_stop     = false;
    m_tdata.period      = period_;
    m_tdata.ext_caller  = ext_caller_;
    m_tdata.thread      = std::thread(CThread::HelperThread, (void*)&m_tdata);
    m_tdata.is_started  = true;

    gSetEvent(m_tdata.event);

    return(1);
  }

  int CThread::Stop()
  {
    if(m_tdata.is_started)
    {
      // signal thread to stop
      m_tdata.do_stop = true;

      // release thread wait barrier
      Fire();

      // join thread if joinable and wait for return
      if(m_tdata.thread.joinable()) m_tdata.thread.join();

      // ok the thread is stopped
      m_tdata.is_started = false;
	  
	  // close event
	  gCloseEvent(m_tdata.event);
    }

    return(1);
  }

  int CThread::Fire()
  {
    gSetEvent(m_tdata.event);
    return(1);
  }

  void CThread::HelperThread(void* par_)
  {
    if(!par_) return;

    struct ThreadData* tdata = static_cast<ThreadData*>(par_);
    if(!gEventIsValid(tdata->event)) return;

    // mark as running
    tdata->is_running = true;

    int state = 0;
    while(!tdata->do_stop)
    {
      // wait for timeout 'period'
      if(tdata->period > 0) gWaitForEvent(tdata->event, tdata->period);

      // call external code
      if(!tdata->do_stop && gEventIsValid(tdata->event) && tdata->ext_caller)
      {
        state = (tdata->ext_caller)();
      }
      else
      {
        state = -1;
      }

      if(state < 0) break;
    }

    // mark as stopped
    tdata->is_running = false;

    return;
  }
}
