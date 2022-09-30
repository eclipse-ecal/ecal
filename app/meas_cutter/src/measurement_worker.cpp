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

#include "measurement_worker.h"
#include "thread_pool.h"

MeasurementWorker::MeasurementWorker(ThreadPool& thread_pool, std::mutex& mutex):
   _thread_pool(thread_pool), 
   _buffer_mutex(mutex),
   _is_busy(false),  
  _is_initialized(false),
  _current_job(eCALMeasCutterUtils::MeasurementJob())
{
  _current_thread = std::make_unique<std::thread>(&MeasurementWorker::doWork, this);
}

void MeasurementWorker::join()
{
  _current_thread->join();
}

bool MeasurementWorker::isThreadBusy()
{
  return _is_busy;
}

bool MeasurementWorker::isThreadInitialized()
{
  return _is_initialized;
}

void MeasurementWorker::abort()
{
  _measurement_converter.abort();
}

void MeasurementWorker::doWork()
{
  _is_initialized = true;

  while (true)
  {
    {
      std::unique_lock<std::mutex> buffer_lock(_buffer_mutex);
      _thread_pool._buffer_cv.wait(buffer_lock, [&]()-> bool {return !_thread_pool._measurements_to_work_on.empty() || !_thread_pool._is_running; });

      _is_busy = true;

      if (!_thread_pool._is_running)
      {
        _is_busy = false;
        return;
      }

      _current_job = _thread_pool._measurements_to_work_on.front();
      _thread_pool._measurements_to_work_on.pop_front();
    }
   
    //// do work here
    _measurement_converter.setCurrentJob(_current_job);
    if (_measurement_converter.isConfigurationValid())
      _measurement_converter.convert();
     
    _is_busy = false;
  }
}
