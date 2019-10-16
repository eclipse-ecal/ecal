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
#include <rec_core/recorder_state.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100 4505 4800)
#endif
#include <ecal/msg/protobuf/client.h>
#include <ecal/pb/rec/service.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <mutex>
#include <deque>

namespace eCAL
{
  namespace rec
  {
    class RemoteRecorder : public AbstractRecorder, protected InterruptibleThread
    {
    //////////////////////////////////////////
    // Constructor & Destructor
    //////////////////////////////////////////
    public:
      explicit RemoteRecorder(const std::string& hostname, const RecorderSettings& initial_settings, bool initially_connected_to_ecal);
      
      ~RemoteRecorder();

    //////////////////////////////////////////
    // Public API
    //////////////////////////////////////////
    public:
      virtual void SetClientConnectionEnabled(bool connect) override;
      virtual bool IsClientConnectionEnabled() const override;

      virtual void SetSettings(const RecorderSettings& settings) override;
      virtual void SetCommand(const RecorderCommand& command) override;

      virtual void InitiateConnectionShutdown(const RecorderCommand& last_command) override;

      virtual bool IsAlive() const override;
      virtual RecorderState GetState() const override;

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
      static eCAL::pb::rec::SetConfigRequest SettingsToSettingsPb(const RecorderSettings& settings);
      static RecorderState StatePbToRecorderState(const eCAL::pb::rec::State& recorder_state_pb);
      static WriterState WriterStatePbToWriterState(const eCAL::pb::rec::State::WriterState& writer_state_pb);
      static eCAL::pb::rec::CommandRequest RecorderCommandToCommandPb(const RecorderCommand& command);
      static void SetJobConfig(google::protobuf::Map<std::string, std::string>* job_config_pb, const JobConfig& job_config);


    //////////////////////////////////////////
    // Auxiliary helper methods
    //////////////////////////////////////////
    private:
      void removeAutorecoveryActions_NoLock();

    //////////////////////////////////////////
    // Member Variables
    //////////////////////////////////////////
    private:
      mutable std::mutex              io_mutex_;
      mutable std::condition_variable io_cv_;

      eCAL::protobuf::CServiceClient<eCAL::pb::rec::EcalRecService> recorder_service_;

      std::chrono::steady_clock::time_point next_ping_time_;
      bool currently_executing_action_;

      int32_t  connected_pid_;
      bool     client_in_sync_;
      bool     recorder_alive_;

      RecorderState                last_state_;
      std::pair<bool, std::string> last_response_;

      bool should_be_connected_to_ecal_;

      class Action
      {
      public:
        Action(bool is_autorecovery = true)
          : is_settings_           (false)
          , is_command_            (false)
          , is_autorecovery_action_(is_autorecovery)
        {}

        Action(const RecorderSettings& settings, bool is_autorecovery = false)
          : settings_              (settings)
          , is_settings_           (true)
          , is_command_            (false)
          , is_autorecovery_action_(is_autorecovery)
        {}

        Action(const RecorderCommand&  command, bool is_autorecovery = false)
          : command_               (command)
          , is_settings_           (false)
          , is_command_            (true)
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

      std::deque<Action> actions_to_perform_;

      RecorderSettings complete_settings_;

      bool connection_shutting_down_;
    };
  }
}
