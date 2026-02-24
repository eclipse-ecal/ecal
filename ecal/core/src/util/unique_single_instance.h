
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
        CUniqueSingleInstance<T>::clear_alive_();
        delete p;
      }
    };

    template <typename T>
    class CUniqueSingleInstance
    {
    public:
      using unique_t = std::unique_ptr<T, SSingleInstanceDeleter<T>>;

      template <typename... Args>
      static unique_t Create(Args&&... args)
      {
        std::lock_guard<std::mutex> lock(get_mutex_());
        if (get_alive_().load(std::memory_order_acquire))
          return unique_t{nullptr};

        unique_t ptr(new T(std::forward<Args>(args)...));
        get_alive_().store(true, std::memory_order_release);
        return ptr;
      }

      static bool alive() noexcept {
        return get_alive_().load(std::memory_order_acquire);
      }

    private:
      friend struct SSingleInstanceDeleter<T>;

      static void clear_alive_() noexcept
      {
        std::lock_guard<std::mutex> lock(get_mutex_());
        get_alive_().store(false, std::memory_order_release);
      }

      static std::atomic<bool>& get_alive_() {
        static std::atomic<bool> a{false};
        return a;
      }

      static std::mutex& get_mutex_() {
        static std::mutex m; 
        return m;
      }
    };
  }
}