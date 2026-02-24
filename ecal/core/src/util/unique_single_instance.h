
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
#include <memory>
#include <mutex>
#include <utility>

namespace eCAL
{
  namespace Util
  {
    template <typename T> class CUniqueSingleInstance;

    template <typename T>
    struct SSingleInstanceDeleter {
      void operator()(T* p) const noexcept {
        CUniqueSingleInstance<T>::clearAlive();
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
        std::lock_guard<std::mutex> lock(getMutex());
        if (getAlive().load(std::memory_order_acquire))
          return UniqueT{nullptr};

        UniqueT ptr(new T(std::forward<Args>(args)...));
        getAlive().store(true, std::memory_order_release);
        return ptr;
      }

      static bool isAlive() noexcept {
        return getAlive().load(std::memory_order_acquire);
      }

    private:
      friend struct SSingleInstanceDeleter<T>;

      static void clearAlive() noexcept
      {
        std::lock_guard<std::mutex> lock(getMutex());
        getAlive().store(false, std::memory_order_release);
      }

      static std::atomic<bool>& getAlive() {
        static std::atomic<bool> a{false};
        return a;
      }

      static std::mutex& getMutex() {
        static std::mutex m; 
        return m;
      }
    };
  }
}