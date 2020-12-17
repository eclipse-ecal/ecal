/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
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

#pragma once

#include <list>
#include <mutex>
#include <algorithm>
#include <functional>

/**
 * @brief A thread safe container with thread safe methods.
 *
 * Though it internally uses a std::list, a container can neither be iterated
 * over, nor can the elements accessed directly. Both would most likely violate
 * the thread safety. Elements however can be removed by value or predicate.
 *
 * If one function shall be applied to all elements of the container, the
 * for_each() function offers that functionality.
 */

template <class T>
class ThreadSafeContainer
{
public:
  ThreadSafeContainer()
  {}

  ~ThreadSafeContainer()
  {}

  // Capacity

  /**
   * @return The number of elements currently in the container
   */
  int size()
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_list.size();
  }

  /**
   * @return Whether the container is empty
   */
  bool empty()
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_list.empty();
  }

  /**
   * @brief Checks whether the element is in the container
   * @param t The element to check
   * @return Whether t is in the container
   */
  bool contains(T& t)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& element : m_list)
    {
      if (element == t)
      {
        return true;
      }
    }
    return false;
  }

  /**
   * @brief Counts the number of occurences of t in the container
   * @param t The element to count
   * @return The number of occurences
   */
  int count(T& t)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    int c = 0;
    for (auto& element : m_list)
    {
      if (element == t)
      {
        c++;
      }
    }
    return c;
  }

  /**
   * @brief Counts the elements for which the predicate returns true
   * @param p The predicate (e.g. a function bool(T& t))
   * @return The number of elements for which the predicate returns true
   */
  template <class predicate>
  int count_if(predicate p)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    int c = 0;
    for (auto& element : m_list)
    {
      if (p(element))
      {
        c++;
      }
    }
    return c;
  }

  // Modifiers

  /**
   * @brief Adds an element to the container
   * @param element The element to add
   */
  void add(T& element)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_list.push_back(element);
  }

  /**
   * @brief Removes an element from the container
   * @param element The element to remove
   */
  void remove(const T& element)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_list.remove(element);
  }

  /**
   * @brief Removes all elements
   */
  void clear()
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_list.clear();
  }

  /**
   * @brief Removes all elements for which the predicate returns true
   * @param p The predicate
   */
  template <class predicate>
  void remove_if(predicate p)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_list.remove_if(p);
  }

  /**
   * @brief Applies a function to all elements
   * @param f The function
   */
  template <class function>
  void for_each(const function& f)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& element : m_list)
    {
      f(element);
    }
  }

private:
  std::list<T> m_list;    /**< The internal list holding all elements*/
  std::mutex m_mutex;     /**< The mutex that makes this container thread safe*/
};
