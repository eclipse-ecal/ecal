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
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <mutex>
#include <condition_variable>

namespace
{
  struct alignas(8) named_event
  {
    pthread_mutex_t mtx;
    pthread_cond_t  cvar;
    uint8_t         set;
  };
  typedef struct named_event named_event_t;

  named_event_t* named_event_create(const char* event_name_)
  {
    // create shared memory file
    int previous_umask = umask(000);  // set umask to nothing, so we can create files with all possible permission bits
    int fd = ::shm_open(event_name_, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    umask(previous_umask);            // reset umask to previous permissions
    if (fd < 0) return nullptr;

    // set size to size of named mutex struct 
    if(ftruncate(fd, sizeof(named_event_t)) == -1)
    {
      ::close(fd);
      return nullptr;
    }

    // create mutex
    pthread_mutexattr_t shmtx;
    pthread_mutexattr_init(&shmtx);
    pthread_mutexattr_setpshared(&shmtx, PTHREAD_PROCESS_SHARED);

    // create condition variable
    pthread_condattr_t  shattr;
    pthread_condattr_init(&shattr);
    pthread_condattr_setpshared(&shattr, PTHREAD_PROCESS_SHARED);
#ifndef ECAL_OS_MACOS
    pthread_condattr_setclock(&shattr, CLOCK_MONOTONIC);
#endif // ECAL_OS_MACOS
    named_event_t* evt = static_cast<named_event_t*>(mmap(nullptr, sizeof(named_event_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    ::close(fd);

    // map them into shared memory
    pthread_mutex_init(&evt->mtx, &shmtx);
    pthread_cond_init(&evt->cvar, &shattr);

    // start with unset state
    evt->set = 0;

    return evt;
  }

  int named_event_destroy(const char* event_name_)
  {
    // destroy (unlink) shared memory file
    return(::shm_unlink(event_name_));
  }

  named_event_t* named_event_open(const char* event_name_)
  {
    // try to open existing shared memory file
    int fd = ::shm_open(event_name_, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (fd < 0) return nullptr;

    // map file content to mutex
    named_event_t* evt = static_cast<named_event_t*>(mmap(nullptr, sizeof(named_event_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    ::close(fd);

    return evt;
  }

  void named_event_close(named_event_t* evt_)
  {
    // unmap condition mutex from shared memory file
    munmap(static_cast<void*>(evt_), sizeof(named_event_t));
  }

  void named_event_set(named_event_t* evt_)
  {
    // lock condition mutex
    pthread_mutex_lock(&evt_->mtx);
    // set state
    evt_->set = 1;
    // signal change
    pthread_cond_signal(&evt_->cvar);
    // unlock condition mutex
    pthread_mutex_unlock(&evt_->mtx);
  }

  bool named_event_wait(named_event_t* evt_, struct timespec* ts_)
  {
    // lock condition mutex
    pthread_mutex_lock(&evt_->mtx);
    // state is set ?, fine !
    if (evt_->set)
    {
      // reset state
      evt_->set = 0;
      // unlock condition mutex
      pthread_mutex_unlock(&evt_->mtx);
      // return success
      return true;
    }
    // state is not set
    else
    {
      // while condition wait did not return failure (or timeout) and
      // state is still locked by another one
      int ret(0);
      while ((ret == 0) && (evt_->set == 0))
      {
        // wait with timeout for unlock signal
        if (ts_)
        {
#ifndef ECAL_OS_MACOS
            ret = pthread_cond_timedwait(&evt_->cvar, &evt_->mtx, ts_);
#else
            ret = pthread_cond_timedwait_relative_np(&evt_->cvar, &evt_->mtx, ts_);
#endif
        }
        // blocking wait for unlock signal
        else
        {
          ret = pthread_cond_wait(&evt_->cvar, &evt_->mtx);
        }
      }
      // if wait (with timeout) returned successfully
      // reset event state
      if (ret == 0) evt_->set = 0;
      // unlock condition mutex
      pthread_mutex_unlock(&evt_->mtx);
      // sucess == wait returned 0
      return (ret == 0);
    }
  }

  bool named_event_trywait(named_event_t* evt_)
  {
    bool set(false);
    // lock condition mutex
    pthread_mutex_lock(&evt_->mtx);
    // check state
    if (evt_->set)
    {
      // reset event state
      evt_->set = 0;
      set = true;
    }
    // unlock condition mutex
    pthread_mutex_unlock(&evt_->mtx);
    // return success
    return set;
  }
}

namespace eCAL
{
  class CEvent
  {
  public:
    CEvent() : m_sigcount(0) { }

    void set()
    {
      std::unique_lock< std::mutex > lock(m_mutex);
      ++m_sigcount;
      m_condition.notify_one();
    }

    bool wait()
    {
      std::unique_lock< std::mutex > lock(m_mutex);
      m_condition.wait(lock,[&]()->bool{ return m_sigcount>0; });
      --m_sigcount;
      return true;
    }

    template< typename R,typename P >
    bool wait(const std::chrono::duration<R,P>& timeout_)
    {
      std::unique_lock< std::mutex > lock(m_mutex);
      if (!m_condition.wait_for(lock, timeout_, [&]()->bool{ return m_sigcount>0; }))
      {
        return false;
      }
      --m_sigcount;
      return true;
    }

  private:
    unsigned int             m_sigcount;
    std::mutex               m_mutex;
    std::condition_variable  m_condition;
  };

  class CNamedEvent
  {
  public:
    explicit CNamedEvent(const std::string& name_) :
      m_name(name_ + "_evt"),
      m_event(nullptr)
    {
      m_name = (m_name[0] != '/') ? "/" + m_name : m_name; // make memory file path compatible for all posix systems
      m_event = named_event_open(m_name.c_str());
      if(m_event == nullptr)
      {
        m_event = named_event_create(m_name.c_str());
      }
    }

    ~CNamedEvent()
    {
      if(m_event == nullptr) return;
      named_event_close(m_event);
      named_event_destroy(m_name.c_str());
    }

    void set()
    {
      if(m_event == nullptr) return;
      named_event_set(m_event);
    }

    bool wait()
    {
      if(m_event == nullptr) return false;
      return(named_event_wait(m_event, nullptr));
    }

    bool wait(long timeout_)
    {
      // check event handle
      if (m_event == nullptr) return false;

      // timeout_ < 0 -> wait infinite
      if (timeout_ < 0)
      {
        return(named_event_wait(m_event, nullptr));
      }
      // timeout_ == 0 -> check state only
      else if (timeout_ == 0)
      {
        return(named_event_trywait(m_event));
      }
      // timeout_ > 0 -> wait timeout_ ms
      else
      {
        struct timespec abstime;
        clock_gettime(CLOCK_MONOTONIC, &abstime);

        abstime.tv_sec = abstime.tv_sec + timeout_ / 1000;
        abstime.tv_nsec = abstime.tv_nsec + (timeout_ % 1000) * 1000000;
        while (abstime.tv_nsec >= 1000000000)
        {
          abstime.tv_nsec -= 1000000000;
          abstime.tv_sec++;
        }
        return(named_event_wait(m_event, &abstime));
      }
    }

  private:
    CNamedEvent(const CNamedEvent&);             // prevent copy-construction
    CNamedEvent& operator=(const CNamedEvent&);  // prevent assignment

    std::string     m_name;
    named_event_t*  m_event;
  };

  bool gOpenEvent(EventHandleT* event_, const std::string& event_name_)
  {
    if(event_ == nullptr) return(false);

    EventHandleT event;
    event.name = event_name_;

    if(event.name.empty())
    {
      event.handle = new CEvent();
    }
    else
    {
      event.handle = new CNamedEvent(event.name);
    }

    if(event.handle != nullptr)
    {
      *event_ = event;
      return true;
    }
    return false;
  }

  bool gCloseEvent(const EventHandleT& event_)
  {
    if(!event_.handle) return false;
    if(event_.name.empty())
    {
      delete static_cast<CEvent*>(event_.handle);
    }
    else
    {
      delete static_cast<CNamedEvent*>(event_.handle);
    }
    return true;
  }

  bool gSetEvent(const EventHandleT& event_)
  {
    if(!event_.handle) return false;
    if(event_.name.empty())
    {
      static_cast<CEvent*>(event_.handle)->set();
    }
    else
    {
      static_cast<CNamedEvent*>(event_.handle)->set();
    }
    return true;
  }

  bool gWaitForEvent(const EventHandleT& event_, const long timeout_)
  {
    if(!event_.handle) return false;
    if(event_.name.empty())
    {
      if(timeout_ < 0)
      {
        return(static_cast<CEvent*>(event_.handle)->wait());
      }
      else
      {
        return(static_cast<CEvent*>(event_.handle)->wait(std::chrono::milliseconds(timeout_)));
      }
    }
    else
    {
      if(timeout_ < 0)
      {
        return(static_cast<CNamedEvent*>(event_.handle)->wait());
      }
      else
      {
        return(static_cast<CNamedEvent*>(event_.handle)->wait(timeout_));
      }
    }
  }

  bool gInvalidateEvent(EventHandleT* event_)
  {
    if(!event_->handle) return false;
    if(event_->handle == nullptr) return false;
    event_->handle = nullptr;
    return true;
  }

  bool gEventIsValid(const EventHandleT& event_)
  {
    return(event_.handle != nullptr);
  }
}

#endif /* ECAL_OS_LINUX */
