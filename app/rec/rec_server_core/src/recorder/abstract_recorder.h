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
#include <rec_core/recorder_state.h>
#include <rec_core/job_config.h>

#include <memory>
#include <vector>

namespace eCAL
{
  namespace rec
  {
    class AbstractRecorder
    {
    public:
      AbstractRecorder(const std::string& hostname)
        : hostname_(hostname)
      {}

      virtual ~AbstractRecorder() {}

      std::string GetHostname() const { return hostname_; }

      virtual void SetClientConnectionEnabled(bool connect) = 0;
      virtual bool IsClientConnectionEnabled() const = 0;

      virtual void SetSettings(const RecorderSettings& settings) = 0;
      virtual void SetCommand(const RecorderCommand& command) = 0;

      virtual void InitiateConnectionShutdown(const RecorderCommand& last_command) = 0;

      virtual bool IsAlive() const = 0;
      virtual RecorderState GetState() const = 0;
      
      virtual bool IsRequestPending() const = 0;
      virtual void WaitForPendingRequests() const = 0;

      virtual std::pair<bool, std::string> GetLastResponse() const = 0;

    protected:
      const std::string hostname_;
    };

  }
}