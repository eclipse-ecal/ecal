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

#include "abstract_recorder.h"

#include <ThreadingUtils/InterruptibleLoopThread.h>

#include <memory>

namespace eCAL
{
  namespace rec
  {
    class EcalRec;
  }

  namespace rec_server
  {

    class LocalRecorder : public AbstractRecorder, protected InterruptibleLoopThread
    {
    //////////////////////////////////////
    // Constructor & Destructor
    //////////////////////////////////////
    public:
      explicit LocalRecorder(const std::string& hostname
                            , const std::shared_ptr<eCAL::rec::EcalRec>& ecal_rec_instance
                            , const std::function<void(const std::string& hostname, const eCAL::rec::RecorderStatus& recorder_status)>& update_jobstatus_function
                            , const std::function<void(int64_t job_id, const std::string& hostname, const std::pair<bool, std::string>& info_command_response)>& report_job_command_response_callback
                            , const RecorderSettings& initial_settings);
      ~LocalRecorder();

    //////////////////////////////////////
    // Interruptible Thread overrrides
    //////////////////////////////////////
    protected:
      void Loop() override;

    //////////////////////////////////////
    // Public API
    //////////////////////////////////////
    public:
      virtual void SetRecorderEnabled(bool enabled, bool connect_to_ecal = false) override;
      virtual bool IsRecorderEnabled() const override;

      virtual bool EverParticipatedInAMeasurement() const override;

      virtual void SetSettings(const RecorderSettings& settings) override;
      virtual void SetCommand(const RecorderCommand& command) override;

      virtual bool IsAlive() const override;
      virtual std::pair<eCAL::rec::RecorderStatus, eCAL::Time::ecal_clock::time_point> GetStatus() const override;

      virtual bool IsRequestPending() const override;
      virtual void WaitForPendingRequests() const override;

      virtual std::pair<bool, std::string> GetLastResponse() const override;

    //////////////////////////////////////
    // Member Variables
    //////////////////////////////////////
    private:
      mutable std::mutex                  ecal_rec_instance_and_status_mutex_;
      std::shared_ptr<eCAL::rec::EcalRec> ecal_rec_instance_;
      eCAL::rec::RecorderStatus           last_status_;

      std::atomic<bool> ever_participated_in_a_measurement_;

      bool             recorder_enabled_;
      bool             is_in_sync_;

      std::pair<bool, std::string> last_response_;

      RecorderSettings complete_settings_;
      bool             should_be_connected_to_ecal_;
    };
  }
}