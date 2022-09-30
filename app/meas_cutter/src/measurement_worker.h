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
#include <mutex>
#include <thread>
#include <atomic>

#include "measurement_converter.h"
#include "utils.h"

class ThreadPool;
class MeasurementWorker
{
public:
  MeasurementWorker(ThreadPool& thread_pool, std::mutex& mutex);

  void join();
  bool isThreadBusy();
  bool isThreadInitialized();
  void abort();

private:
  void doWork();
  std::unique_ptr<std::thread>        _current_thread;
  ThreadPool&                         _thread_pool;
  std::mutex&                         _buffer_mutex;
  std::atomic<bool>                   _is_busy;
  std::atomic<bool>                   _is_initialized;

  eCALMeasCutterUtils::MeasurementJob _current_job;
  MeasurementConverter                _measurement_converter;
};

