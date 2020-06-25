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

#include "recorder_settings.h"
#include "recorder_command.h"
#include <rec_client_core/state.h>
#include <rec_client_core/job_config.h>
#include <rec_client_core/upload_config.h>


#include <memory>
#include <vector>
#include <functional>

namespace eCAL
{
  namespace rec_server
  {
    class AbstractRecorder
    {
    public:
      AbstractRecorder(const std::string& hostname
                      , const std::function<void(const std::string& hostname, const eCAL::rec::RecorderStatus& recorder_status)>& update_jobstatus_function
                      , const std::function<void(int64_t job_id, const std::string& hostname, const std::pair<bool, std::string>& info_command_response)>& report_job_command_response_callback)
        : hostname_(hostname)
        , update_jobstatus_function_(update_jobstatus_function)
        , report_job_command_response_callback_(report_job_command_response_callback)
      {}

      virtual ~AbstractRecorder() {}

      std::string GetHostname() const { return hostname_; }

      virtual void SetRecorderEnabled(bool enabled, bool connect_to_ecal = false) = 0;
      virtual bool IsRecorderEnabled() const = 0;

      virtual bool EverParticipatedInAMeasurement() const = 0;

      virtual void SetSettings(const RecorderSettings& settings) = 0;
      virtual void SetCommand(const RecorderCommand& command) = 0;

      virtual bool IsAlive() const = 0;
      virtual std::pair<eCAL::rec::RecorderStatus, eCAL::Time::ecal_clock::time_point> GetStatus() const = 0;
      
      virtual bool IsRequestPending() const = 0;
      virtual void WaitForPendingRequests() const = 0;

      virtual std::pair<bool, std::string> GetLastResponse() const = 0;

    protected:
      const std::string hostname_;
      const std::function<void(const std::string& hostname, const eCAL::rec::RecorderStatus& recorder_status)> update_jobstatus_function_;
      const std::function<void(int64_t job_id, const std::string& hostname, const std::pair<bool, std::string>& info_command_response)> report_job_command_response_callback_;
    };

  }
}