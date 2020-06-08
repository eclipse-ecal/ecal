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
#include <functional>
#include <string>
#include <chrono>

#include <rec_client_core/job_config.h>
#include <rec_client_core/state.h>

#include "concurrent_queue.h"
#include "pipe_handler.h"
#include "response_handler.h"

namespace eCAL
{
  namespace rec
  {
    class Addon
    {
    ////////////////////////////////////////////
    // Constructor & destructor
    ////////////////////////////////////////////
    public:
      Addon(const std::string& executable_path, std::function<void(std::int64_t job_id, const std::string& addon_id, const RecAddonJobStatus& job_status)> set_job_status_function);
      ~Addon();

      Addon(const Addon&) = delete;
      Addon& operator=(const Addon&) = delete;

    ////////////////////////////////////////////
    // public API
    ////////////////////////////////////////////
    public:
      void Initialize();
      void Deinitialize();

      void SetPreBuffer(bool enabled, std::chrono::steady_clock::duration length);

      void StartRecording(const JobConfig& job_config);
      void StopRecording();
      void SaveBuffer(const JobConfig& job_config);

      std::string GetAddonId() const;
      RecorderAddonStatus GetStatus() const;

      bool IsRunning() const;

    ////////////////////////////////////////////
    // Member variables
    ////////////////////////////////////////////
    private:
      mutable std::mutex            status_mutex_;
      RecorderAddonStatus           last_status_;
      
      addon::ResponseHandler response_handler_;
      ConcurrentQueue<std::pair<addon::Request, std::function<void(const addon::Response&)>>> request_queue_;
      PipeHandler pipe_handler_;

      std::thread status_thread_;
      bool status_thread_enabled_ = false;

      bool pre_buffering_enabled_ = false;

      std::int64_t currently_recording_job_id_;

      std::function<void(std::int64_t job_id, const std::string& addon_id, const RecAddonJobStatus& job_status)> set_job_status_function_;
    };
  }
}
