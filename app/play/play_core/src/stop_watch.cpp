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

#include "stop_watch.h"

Stopwatch::Stopwatch()
  : paused_(true)
  , elapsed_time_(0)
  , last_start_(std::chrono::nanoseconds(0))
{}

Stopwatch::~Stopwatch()
{}

void Stopwatch::Pause()
{
  auto now       = std::chrono::steady_clock::now();
  
  std::lock_guard<std::mutex> lock(stopwatch_mutex_);

  if (!paused_)
  {
    paused_        = true;
    elapsed_time_ += (now - last_start_);
  }
}

void Stopwatch::Resume()
{
  auto now = std::chrono::steady_clock::now();

  std::lock_guard<std::mutex> lock(stopwatch_mutex_);

  if (paused_)
  {
    last_start_ = now;
    paused_     = false;
  }
}

bool Stopwatch::IsPaused()
{
  std::lock_guard<std::mutex> lock(stopwatch_mutex_);
  return paused_;
}


std::chrono::nanoseconds Stopwatch::GetElapsedTime()
{
  auto now = std::chrono::steady_clock::now();

  std::lock_guard<std::mutex> lock(stopwatch_mutex_);

  if (!paused_)
  {
    return elapsed_time_ + (now - last_start_);
  }
  else
  {
    return elapsed_time_;
  }
}

void Stopwatch::Restart()
{
  auto now = std::chrono::steady_clock::now();

  std::lock_guard<std::mutex> lock(stopwatch_mutex_);

  last_start_   = now;
  elapsed_time_ = std::chrono::nanoseconds(0);
  paused_       = false;
}

std::chrono::nanoseconds Stopwatch::GetElapsedTimeAndRestart()
{
  auto now = std::chrono::steady_clock::now();
  std::chrono::nanoseconds elapsed;

  std::lock_guard<std::mutex> lock(stopwatch_mutex_);

  if (!paused_)
  {
    elapsed =  elapsed_time_ + (now - last_start_);
  }
  else
  {
    elapsed = elapsed_time_;
  }

  last_start_   = now;
  elapsed_time_ = std::chrono::nanoseconds(0);
  paused_       = false;

  return elapsed;
}
