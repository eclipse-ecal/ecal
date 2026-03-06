
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
#include <mutex>


namespace eCAL
{
  namespace Util
  {
    /** @brief Helper class to ensure a single instance of a type. 
     * 
     * Usage:
     * 1) Make the class a friend of CSingleInstanceHelper and provide a 
     *    static Create function that calls CSingleInstanceHelper::Create 
     *    with the appropriate arguments to create an instance of the class. 
     *    The constructor of the class should be private to prevent creation
     *    of instances outside of the Create function.
     * 
     * 2) Call the Create function of the class to create an instance. 
     *    If an instance already exists, the Create function will return a
     *    shared pointer to the existing instance.
    */
    template <typename T>
    class CSingleInstanceHelper
    {
    public:
      template <typename... Args>
      static std::shared_ptr<T> Create(Args&&... args)
      {
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);

        auto instance = m_instance.lock();;
        if (instance)
          return instance;

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