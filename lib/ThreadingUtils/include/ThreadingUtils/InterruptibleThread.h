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

#pragma once

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

/**
 * @brief A Thread class that is designed to stop execution when it gets interrupted
 *
 * For implementing an interruptible thread, there are three important
 * functions:
 *
 * void Run():
 *    This is the function that has to be implemented. This function is called
 *    in a parallel context, once Start() is called.
 *
 * bool IsInterrupted():
 *    This function should be checked regularily. By calling Interrupt(), an
 *    external thread can interrupt the thread, but the thread has to perform
 *    a gracefull shutdown itself.
 *
 * void SleepFor(dur):
 *    A interruptible thread should not use blocking functions that are not
 *    interruptible. Therefore, the SleepFor() function stops sleeping once any
 *    thread calls Interrupt(). The implementation should always check whether
 *    the thread has been interrupted after sleeping and perform a gracefull
 *    shutdown if that is the case.
 *
 * @author Florian Reimold
 */
class InterruptibleThread
{
public:
  /**
   * @brief Creates a new Interruptible Thread
   */
  InterruptibleThread() :
    m_isRunning(false),
    m_isInterrupted(false)
  {};

  InterruptibleThread(const InterruptibleThread&) = delete;
  InterruptibleThread& operator=(const InterruptibleThread&) = delete;

  /**
   * @brief Starts the thread, if it is not currently running.
   * Starting the Thread also clears the interrupt state.
   *
   * @return True, if the thread has been started
   */
  bool Start() {
    std::lock_guard<std::mutex> startup_lock(m_startup_mutex);
    if (IsRunning()) {
      return false;
    }
    // Before destroying the old thread, we might have to join it.
    Join();
    m_isInterrupted = false;
    m_isRunning = true;
    m_thread = std::thread(&InterruptibleThread::RunWrapper, this);
    return true;
  }

  /**
   * @brief Returns true, if the thread has been interrupted
   * @return true if the thread has been interrupted
   */
  bool IsInterrupted() const{
    return m_isInterrupted;
  }

  /**
   * @brief Returns true, if the thread has been started and is still running.
   * @return true if the thread is running
   */
  bool IsRunning() const {
    return m_isRunning;
  }

  /**
   * @brief Interrupts the thread, i.e. sets the interrupt state to true and wakes up the SleepFor function
   */
  virtual void Interrupt() {
    m_isInterrupted = true;
    m_interruptCv.notify_all();
  }

  /**
   * @brief Blocks until the thread exits
   */
  virtual void Join() {
    if (m_thread.joinable()) {
      m_thread.join();
    }
  }

  /**
   * @brief Gets the ID of the thread
   * @return The ID of the thread
   */
  std::thread::id GetId() const {
    return m_thread.get_id();
  }

  virtual ~InterruptibleThread() {};

protected:
  /**
   * @brief The function that is executed in a parallel context. This function has to be overwritten by the developer.
   */
  virtual void Run() = 0;

  /**
   * @brief blocks for the given time or until the thread gets interrupted.
   * After using this function, you should always check whether the thread has
   * been interrupted and take appropriate actions
   *
   * @param sleep_duration  the desired duration to block
   */
  void SleepFor(std::chrono::nanoseconds sleep_duration){
    std::unique_lock<std::mutex> interruptLock(m_interruptMutex);
    m_interruptCv.wait_for(interruptLock, sleep_duration, [=] {return bool(m_isInterrupted); });
  }

  std::mutex              m_startup_mutex;      /**< A Mutex for thread safe starting of this thread */

  std::atomic<bool>       m_isRunning;          /**< Whether the interruptible thread is currently running*/
  std::atomic<bool>       m_isInterrupted;      /**< Whether the interruptible thread has been interrupted*/

  std::thread             m_thread;             /**< The "actual" thread executing the Run() function*/
  std::mutex              m_interruptMutex;     /**< A Mutex used for that condidition variable that is used for interrupting the thread */
  std::condition_variable m_interruptCv;        /**< A Condidition variable that is used for interrupting the thread*/

private:
  /**
   * @brief Wraps the run function. If the thread is interrupted, the function is not executed.
   */
  void RunWrapper() {
    if (IsInterrupted()) {
      return;
    }
    Run();
    m_isRunning     = false;
  }
};

