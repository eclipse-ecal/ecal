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
#include <thread>
#include <vector>
#include <mutex>
#include <deque>
#include <atomic>
#include <condition_variable>

#include "utils.h"
#include "measurement_worker.h"

class ThreadPool
{
public:
  ThreadPool(int count_threads = std::thread::hardware_concurrency());
  ~ThreadPool();
  ThreadPool(ThreadPool const&) = delete;
  ThreadPool& operator =(ThreadPool const&) = delete;
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

  void Enque(eCALMeasCutterUtils::MeasurementJob& measurement_job);
  void Start();
  void Stop();

  bool                                               areThreadsBusy();
  bool                                               areThreadsInitialized();
  void                                               processJobs();

private:
  std::atomic_bool                                   _is_running;
  std::deque<eCALMeasCutterUtils::MeasurementJob>    _measurements_to_work_on;
  std::condition_variable                            _buffer_cv;
  mutable std::mutex                                 _mutex_job;
  int                                                _maximum_worker_number;
  friend class MeasurementWorker;
  std::vector<std::unique_ptr<MeasurementWorker>>    _worker_list;
};
