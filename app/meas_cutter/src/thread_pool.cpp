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

#include "thread_pool.h"

ThreadPool::ThreadPool(int count_threads):
  _is_running(true),
  _maximum_worker_number(count_threads)
{
}

ThreadPool::~ThreadPool()
{
  Stop();
}

void ThreadPool::Enque(eCALMeasCutterUtils::MeasurementJob& measurement_job)
{
  std::lock_guard<std::mutex> buffer_lock(_mutex_job);
  _measurements_to_work_on.emplace_back(std::move(measurement_job));
  _buffer_cv.notify_one();
}

void ThreadPool::Start()
{
  for (int i = 0; i < _maximum_worker_number; i++)
  {
    auto worker = std::make_unique<MeasurementWorker>(*this,_mutex_job);
    _worker_list.push_back(std::move(worker));
  }
  while (!areThreadsInitialized())
  {
  }
}

void ThreadPool::Stop()
{
  {
    std::unique_lock<std::mutex> lock(_mutex_job);
    _is_running = false;
  }
  _buffer_cv.notify_all();

  // Join all threads
  for (auto const& worker : _worker_list)
  {
    worker->abort();
    worker->join();
  }

  _worker_list.clear();
}

bool ThreadPool::areThreadsBusy()
{
 for (auto const& worker : _worker_list)
  {
    if (worker->isThreadBusy())
      return true;
  }
  return false;
}

bool ThreadPool::areThreadsInitialized()
{
  for (auto const& worker : _worker_list)
  {
    if (!worker->isThreadInitialized())
      return false;
  }
  return true;
}

void ThreadPool::processJobs()
{
  while (true)
  {
    {
      std::lock_guard<std::mutex> buffer_lock(_mutex_job);
      if (_measurements_to_work_on.empty() && !areThreadsBusy())
      {
        break;
      }
    }
  }
}
