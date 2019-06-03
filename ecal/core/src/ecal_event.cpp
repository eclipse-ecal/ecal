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
 * @brief  eCAL handle helper class - windows platform
**/

#include <ecal/ecal.h>
#include <ecal/ecal_os.h>

#include <ecal/ecal_event.h>

#include <sstream>
#include <memory>
#include <chrono>
#include <thread>

#ifdef ECAL_OS_WINDOWS

#include "ecal_win_main.h"

namespace eCAL
{
  bool gOpenEvent(EventHandleT* event_, const std::string& event_name_)
  {
    if(event_ == nullptr) return(false);
    EventHandleT event;
    event.name   = event_name_;
    event.handle = ::CreateEvent(nullptr, false, false, event_name_.c_str());
    if(event.handle != nullptr)
    {
      *event_ = event;
      return(true);
    }
    return(false);
  }

  bool gCloseEvent(const EventHandleT& event_)
  {
    if(!event_.handle) return(false);
    return(::CloseHandle(event_.handle) != 0);
  }

  bool gSetEvent(const EventHandleT& event_)
  {
    if(!event_.handle) return(false);
    return(::SetEvent(event_.handle) != 0);
  }

  bool gWaitForEvent(const EventHandleT& event_, const long timeout_)
  {
    if(!event_.handle) return(false);
    if(timeout_ < 0)
    {
      return(::WaitForSingleObject(event_.handle, INFINITE) == WAIT_OBJECT_0);
    }
    else
    {
      return(::WaitForSingleObject(event_.handle, timeout_) == WAIT_OBJECT_0);
    }
  }

  bool gInvalidateEvent(EventHandleT* event_)
  {
    if(event_ == nullptr) return(false);
    if(event_->handle == nullptr) return(false);
    event_->handle = nullptr;
    return(true);
  }

  bool gEventIsValid(const EventHandleT& event_)
  {
    return(event_.handle != nullptr);
  }
}
#endif /* ECAL_OS_WINDOWS */

#ifdef ECAL_OS_LINUX

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <semaphore.h>
#include <time.h>

namespace eCAL
{
  class CNamedEvent
  {
  public:
    explicit CNamedEvent(const std::string& name_) :
      m_sema(nullptr)
    {
      if(name_[0] != '/')
      {
        m_name = "/";
      }
      m_name += name_;

      int previous_umask = umask(000);  // Set umask to nothing, so we can create files with all possible permission bits
      m_sema = sem_open(m_name.c_str(), O_CREAT | O_EXCL, (S_IRUSR | S_IWUSR) | (S_IRGRP | S_IWGRP) | (S_IROTH | S_IWOTH), 0);
      umask(previous_umask);            // Reset umask to previous permissions

      if(m_sema == SEM_FAILED)
      {
        m_sema = sem_open(m_name.c_str(), 0);
        if(m_sema == SEM_FAILED)
        {
          m_sema = nullptr;
        }
      }
    };

    ~CNamedEvent()
    {
      if(m_sema == nullptr) return;
      sem_close(m_sema);
    }

    void signal()
    {
      if(m_sema == nullptr) return;
      sem_post(m_sema);
    }

    bool wait(const long timeout_)
    {
      if(m_sema == nullptr) return(false);
      if(timeout_ < 0)
      {
        return(sem_wait(m_sema) == 0);
      }
      else
      {
        auto wait_until = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_);
        
        do
        {
          if (sem_trywait(m_sema) == 0)
          {
            // The semaphore could be locked!
            return true;
          }
          else if (errno == EAGAIN)
          {
            // The semaphore could not be locked. Let's wait some time and try again.
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
          }
        } while (std::chrono::steady_clock::now() < wait_until);
        
        // When reaching this code, we were never able to lock the semaphore
        return false;
        
        /*
         * Why the code below is commented out:
         * The timeout in the code below is used to compute an absolute time,
         * until the process should wait to lock the semaphore. The time basis
         * used is the CLOCK_REALTIME (gettimeofday), as sem_timedwait() needs
         * an absolute CLOCK_REALTIME time-point.
         * This however causes the whole system to hang, when the system time
         * is set to a time-point in the past, as it may happen when the NPT
         * deamon obtains a new time, or the system time is synchronized with 
         * using PTP.
         */
        
//        struct timespec abstime;
//        struct timeval  tv;
//        gettimeofday(&tv, NULL);
//        abstime.tv_sec  = tv.tv_sec + timeout_ / 1000;
//        abstime.tv_nsec = tv.tv_usec*1000 + (timeout_ % 1000)*1000000;
//        if (abstime.tv_nsec >= 1000000000)
//        {
//          abstime.tv_nsec -= 1000000000;
//          abstime.tv_sec++;
//        }
//        return(sem_timedwait(m_sema, &abstime) == 0);
      }
    }

  private:
    CNamedEvent(const CNamedEvent&);                               // prevent copy-construction
    CNamedEvent& operator=(const CNamedEvent&);                    // prevent assignment

    std::string  m_name;
    sem_t*       m_sema;
  };

  bool gOpenEvent(EventHandleT* event_, const std::string& event_name_)
  {
    if(event_ == nullptr) return(false);

    EventHandleT event;
    if(event_name_.size() == 0)
    {
      std::stringstream counter;
      static int cnt = 0;
      counter << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() + cnt++;;
      event.name = "ecal_semaphore_" + counter.str();
    }
    else
    {
      event.name = event_name_;
    }
    event.handle = new CNamedEvent(event.name);
    if(event.handle != nullptr)
    {
      *event_ = event;
      return(true);
    }
    return(false);
  }

  bool gCloseEvent(const EventHandleT& event_)
  {
    if(!event_.handle) return(false);
    delete static_cast<CNamedEvent*>(event_.handle);
    return(true);
  }

  bool gSetEvent(const EventHandleT& event_)
  {
    if(!event_.handle) return(false);
    static_cast<CNamedEvent*>(event_.handle)->signal();
    return(true);
  }

  bool gWaitForEvent(const EventHandleT& event_, const long timeout_)
  {
    if(!event_.handle) return(false);
    return(static_cast<CNamedEvent*>(event_.handle)->wait(timeout_));
  }

  bool gInvalidateEvent(EventHandleT* event_)
  {
    if(!event_->handle) return(false);
    if(event_->handle == nullptr) return(false);
    event_->handle = nullptr;
    return(true);
  }

  bool gEventIsValid(const EventHandleT& event_)
  {
    return(event_.handle != nullptr);
  }
}

#endif /* ECAL_OS_LINUX */
