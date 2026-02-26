
/* ========================= eCAL LICENSE =================================
 *
 * Copyright 2026 AUMOVIO and subsidiaries. All rights reserved.
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
 * @brief This file provides a template class to ensure a unique single instance of a type.
**/

#pragma once
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <utility>

namespace eCAL
{
  namespace Util
  {
    template <typename T> class CUniqueSingleInstance;

    template <typename T>
    struct SSingleInstanceDeleter {
      void operator()(T* p) const noexcept {
        CUniqueSingleInstance<T>::ClearAlive();
        delete p;
      }
    };

    template <typename T>
    class CUniqueSingleInstance
    {
    public:
      using UniqueT = std::unique_ptr<T, SSingleInstanceDeleter<T>>;

      template <typename... Args>
      static UniqueT Create(Args&&... args)
      {
        std::lock_guard<std::mutex> lock(GetMutex());
        if (GetAlive().load(std::memory_order_acquire))
          return UniqueT{nullptr};

        UniqueT ptr(new T(std::forward<Args>(args)...));
        GetAlive().store(true, std::memory_order_release);
        return ptr;
      }

      static bool IsAlive() noexcept {
        return GetAlive().load(std::memory_order_acquire);
      }

      class CSharedLockGuard
      {
      public:
        CSharedLockGuard()
        {
          std::unique_lock<std::mutex> lk(GetAccessMutex());

          AccessCondition().wait(lk, []{
              return !WriterWaiting().load(std::memory_order_acquire);
          });

          ActiveReaders().fetch_add(1, std::memory_order_acq_rel);
          // unlock AccessMutex before leaving; reader remains active
        }

        ~CSharedLockGuard()
        {
          std::unique_lock<std::mutex> lk(GetAccessMutex());
          if (ActiveReaders().fetch_sub(1, std::memory_order_acq_rel) == 1)
            AccessCondition().notify_all();
        }

      private:
        std::unique_lock<std::mutex> lock_;
      };

      class CUniqueLockGuard
      {
      public:
        CUniqueLockGuard()
        {
          std::unique_lock<std::mutex> lk(GetAccessMutex());
          WriterWaiting().store(true, std::memory_order_release);

          AccessCondition().wait(lk, []{
              return ActiveReaders().load(std::memory_order_acquire) == 0;
          });
        }

        ~CUniqueLockGuard()
        {
            std::unique_lock<std::mutex> lk(GetAccessMutex());
            WriterWaiting().store(false, std::memory_order_release);
            AccessCondition().notify_all();
        }
        
      private:
        std::unique_lock<std::mutex> lock_;
      };

    private:
      friend struct SSingleInstanceDeleter<T>;

      static void ClearAlive() noexcept
      {
        std::lock_guard<std::mutex> lock(GetMutex());
        GetAlive().store(false, std::memory_order_release);
      }

      static std::atomic<bool>& GetAlive() {
        static std::atomic<bool> a{false};
        return a;
      }

      static std::mutex& GetMutex() {
        static std::mutex m; 
        return m;
      }

      static std::mutex& GetAccessMutex() {
        static std::mutex m; 
        return m;
      }

      static std::condition_variable& AccessCondition() {
        static std::condition_variable cv;
        return cv;
      }

      static std::atomic<int>& ActiveReaders() {
        static std::atomic<int> r{0};
        return r;
      }

      static std::atomic<bool>& WriterWaiting() {
        static std::atomic<bool> w{false};
        return w;
      }
    };
  }
}