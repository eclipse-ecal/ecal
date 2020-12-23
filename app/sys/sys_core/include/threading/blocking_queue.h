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
#include <mutex>
#include <condition_variable>
#include <deque>

/**
 * @brief A thread safe FIFO queue with a blocking pop function
 *
 * @author Florian Reimold
 */
template <typename T>
class BlockingQueue
{
public:
  /**
   * @brief Adds a new element to the FIFO qu
   * @param value the new element
   */
  void push(T const& value) {
    {
      std::unique_lock<std::mutex> lock(this->m_mutex);
      m_queue.push_front(value);
    }
    this->m_condition.notify_one();
  }
  
  /**
   * @brief Gets and removes the oldest element of the FIFO queue.
   *
   * This function blocks indefinitively, if the queue is empty, until a new
   * Element is added to it.
   *
   * @return the next element from the FIFO queue
   */
  T pop() {
    std::unique_lock<std::mutex> lock(this->m_mutex);
    this->m_condition.wait(lock, [=] { return !this->m_queue.empty(); });
    T rc(std::move(this->m_queue.back()));
    this->m_queue.pop_back();
    return rc;
  }

  /**
   * @brief checks wether the queue is empty
   * @return true if the queue is empty
   */
  bool empty() {
    std::unique_lock<std::mutex> lock(this->m_mutex);
    return m_queue.empty();
  }

  /**
   * @brief clears the queue, so it will not contain any element afterwards
   */
  void clear() {
    std::unique_lock<std::mutex> lock(this->m_mutex);
    m_queue.clear();
  }

private:
  std::mutex              m_mutex;                                              /**< The mutex making this queue thread safe */
  std::condition_variable m_condition;                                          /**< The condidition variable used for notifying the waiting thread, that new data is available */

  std::deque<T>           m_queue;                                              /**< The actual queue that stores the data */
};
