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
#include <ThreadingUtils/InterruptibleThread.h>

#include "recorder_settings.h"
#include "recorder_command.h"
#include <rec_client_core/state.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100 4505 4800)
#endif
#include <ecal/msg/protobuf/client.h>
#include <ecal/pb/rec/client_service.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <mutex>
#include <deque>

namespace eCAL
{
  namespace rec_server
  {
    class RemoteRecorder : public AbstractRecorder, protected InterruptibleThread
    {

    //////////////////////////////////////////
    // Nested types
    //////////////////////////////////////////
    private:
      class Action
      {
      public:
        Action(bool is_autorecovery = true)
          : is_settings_(false)
          , is_command_(false)
          , is_autorecovery_action_(is_autorecovery)
        {}

        Action(const RecorderSettings& settings, bool is_autorecovery = false)
          : settings_(settings)
          , is_settings_(true)
          , is_command_(false)
          , is_autorecovery_action_(is_autorecovery)
        {}

        Action(const RecorderCommand&  command, bool is_autorecovery = false)
          : command_(command)
          , is_settings_(false)
          , is_command_(true)
          , is_autorecovery_action_(is_autorecovery)
        {}

        ~Action() {}

        RecorderSettings settings_;
        RecorderCommand  command_;

        bool IsSettings()           const { return is_settings_; }
        bool IsCommand()            const { return is_command_; }
        bool IsPing()               const { return !is_settings_ && !is_command_; }
        bool IsAutorecoveryAction() const { return is_autorecovery_action_; }

        void SetIsAutorecoveryAction(bool is_autorecovery) { is_autorecovery_action_ = is_autorecovery; }

      private:
        bool is_settings_;
        bool is_command_;
        bool is_autorecovery_action_;
      };

    //////////////////////////////////////////
    // Constructor & Destructor
    //////////////////////////////////////////
    public:
      explicit RemoteRecorder(const std::string& hostname
                              , const std::function<void(const std::string& hostname, const eCAL::rec::RecorderStatus& recorder_status)>& update_jobstatus_function
                              , const std::function<void(int64_t job_id, const std::string& hostname, const std::pair<bool, std::string>& info_command_response)>& report_job_command_response_callback
                              , const RecorderSettings& initial_settings);
      
      ~RemoteRecorder();

    //////////////////////////////////////////
    // Public API
    //////////////////////////////////////////
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

    //////////////////////////////////////////
    // InterruptibleThread overrides
    //////////////////////////////////////////
    protected:
      virtual void Run() override;
      virtual void Interrupt() override;

    //////////////////////////////////////////
    // Converter Functions
    //////////////////////////////////////////
    private:
      static eCAL::pb::rec_client::SetConfigRequest SettingsToSettingsPb(const RecorderSettings& settings);
      static eCAL::rec::RecorderStatus StatusPbToRecorderStatus(const eCAL::pb::rec_client::State& recorder_state_pb);
      static eCAL::pb::rec_client::CommandRequest RecorderCommandToCommandPb(const RecorderCommand& command);
      static void SetJobConfig(google::protobuf::Map<std::string, std::string>* job_config_pb, const eCAL::rec::JobConfig& job_config);
      static void SetUploadConfig(google::protobuf::Map<std::string, std::string>* upload_config_pb, const eCAL::rec::UploadConfig& upload_config);

    //////////////////////////////////////////
    // Auxiliary helper methods
    //////////////////////////////////////////
    private:
      void removeAutorecoveryActions_NoLock();
      void QueueSetSettings_NoLock(const RecorderSettings& settings);
      void QueueSetCommand_NoLock(const RecorderCommand& command);

    //////////////////////////////////////////
    // Member Variables
    //////////////////////////////////////////
    private:
      mutable std::mutex              io_mutex_;
      mutable std::condition_variable io_cv_;

      eCAL::protobuf::CServiceClient<eCAL::pb::rec_client::EcalRecClientService> recorder_service_;

      std::chrono::steady_clock::time_point next_ping_time_;
      bool currently_executing_action_;

      bool    recorder_enabled_;
      int32_t connected_pid_;
      bool    client_in_sync_;
      bool    recorder_alive_;

      std::atomic<bool> ever_participated_in_a_measurement_;

      eCAL::rec::RecorderStatus                     last_status_;
      eCAL::Time::ecal_clock::time_point last_status_timestamp_;
      std::pair<bool, std::string>       last_response_;

      bool should_be_connected_to_ecal_;

      std::deque<Action> actions_to_perform_;

      RecorderSettings complete_settings_;
    };
  }
}
