
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
 * @brief This file provides a template class to ensure a single instance of a type.
**/

#pragma once
#include <memory>

namespace eCAL
{
  namespace Util
  {
      template <typename T>
      class CSingleInstanceHelper
      {
      public:
        template <typename... Args>
        static std::shared_ptr<T> Create(Args&&... args)
        {
          auto instance = m_instance.lock();;
          if (instance)
            return std::shared_ptr<T>{};

          auto ptr = std::shared_ptr<T>(new T(std::forward<Args>(args)...));
          m_instance = ptr;
         
          return ptr;
        }

      private:
        static std::weak_ptr<T> m_instance;
      };

      template <typename T>
      std::weak_ptr<T> CSingleInstanceHelper<T>::m_instance;
  }
}