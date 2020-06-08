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

#include <ecal/rec/recorder_impl_base.h>

#include <shared_mutex>
#include <unordered_map>
#include <cstdint>

#include "recorder_types.h"
#include "time_limited_queue.h"

namespace eCAL
{
  namespace rec 
  {
    namespace addon 
    {
      class Recorder : public ConcurrentStatusInterface<std::string>
      {
      public:

        Recorder(RecorderImplBase& recorder_backend);
        ~Recorder();

        bool Initialize();
        bool Deinitialize();
        bool StartRecording(std::int64_t job_id, const std::string& path);
        bool StopRecording(std::int64_t job_id);
        bool SavePrebuffer(std::int64_t job_id, const std::string& path);
        bool SetPrebufferLength(std::chrono::steady_clock::duration duration);
        std::chrono::steady_clock::duration GetPrebufferLength() const;
        std::size_t GetPrebufferFrameCount() const;
        bool EnablePrebuffering();
        bool DisablePrebuffering();
        Info GetInfo() const;
        JobStatuses GetJobStatuses() const;

      private:
        mutable std::shared_timed_mutex mutex_;
        RecorderImplBase& impl_;
        bool is_initialzed_ = false;
        bool pre_buffering_enabled_ = false;
        TimeLimtedQueue<BaseFrame> pre_buffer_;
        std::unordered_map<std::int64_t, std::shared_ptr<TimeLimtedQueue<BaseFrame>>> buffers_;
        JobStatuses job_statuses_;
      };
    }
  }
}

