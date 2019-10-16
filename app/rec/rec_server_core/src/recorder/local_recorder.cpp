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

#include "local_recorder.h"

#include <rec_core/ecal_rec.h>

namespace eCAL
{
  namespace rec
  {
    LocalRecorder::LocalRecorder(const std::string& hostname, const std::shared_ptr<eCAL::rec::EcalRec>& ecal_rec_instance, const RecorderSettings& initial_settings, bool initially_connected_to_ecal)
      : AbstractRecorder(hostname)
      , ecal_rec_instance_          (ecal_rec_instance)
      , connection_activated_       (false)
      , is_in_sync_                 (false)
      , last_response_              ({true, ""})
      , complete_settings_          (initial_settings)
      , should_be_connected_to_ecal_(initially_connected_to_ecal)
    {}
    
    LocalRecorder::~LocalRecorder()
    {}

    void LocalRecorder::SetClientConnectionEnabled(bool connect)
    {
      if (!connection_activated_ && connect)
      {
        connection_activated_ = connect;

        SetSettings(complete_settings_);
        if (should_be_connected_to_ecal_)
          ecal_rec_instance_->ConnectToEcal();
        else if (ecal_rec_instance_->IsConnectedToEcal())
          ecal_rec_instance_->DisconnectFromEcal();
      }
      else if (connection_activated_ && !connect)
      {
        connection_activated_ = connect;

        is_in_sync_ = false;
      }
    }

    bool LocalRecorder::IsClientConnectionEnabled() const
    {
      return connection_activated_;
    }

    void LocalRecorder::SetSettings(const RecorderSettings& settings)
    {
      complete_settings_.AddSettings(settings);

      if (!connection_activated_)
        return;

      const RecorderSettings& settings_to_set = (is_in_sync_ ? settings : complete_settings_);

      if (settings_to_set.IsMaxPreBufferLengthSet())
      {
        ecal_rec_instance_->SetMaxPreBufferLength(settings_to_set.GetMaxPreBufferLength());
      }
      
      if (settings_to_set.IsPreBufferingEnabledSet())
      {
        ecal_rec_instance_->SetPreBufferingEnabled(settings_to_set.GetPreBufferingEnabled());
      }
      
      if (settings_to_set.IsHostFilterSet())
      {
        bool success = ecal_rec_instance_->SetHostFilter(settings_to_set.GetHostFilter());
        if (!success)
        {
          last_response_ = { false, "Unable to set host filter" };
          is_in_sync_ = false;
          return;
        }
      }
      
      if (settings_to_set.IsRecordModeSet() && settings_to_set.IsListedTopicsSet())
      {
        bool success = ecal_rec_instance_->SetRecordMode(settings_to_set.GetRecordMode(), settings_to_set.GetListedTopics());
        if (!success)
        {
          last_response_ = { false, "Unable to set record mode and listed topics" };
          is_in_sync_ = false;
          return;
        }
      }
      else
      {
        if (settings_to_set.IsRecordModeSet())
        {
          bool success = ecal_rec_instance_->SetRecordMode(settings_to_set.GetRecordMode());
          if (!success)
          {
            last_response_ = { false, "Unable to set record mode" };
            is_in_sync_ = false;
            return;
          }
        }
      
        if (settings_to_set.IsListedTopicsSet())
        {
          bool success = ecal_rec_instance_->SetListedTopics(settings_to_set.GetListedTopics());
          if (!success)
          {
            last_response_ = { false, "Unable to set topic list" };
            is_in_sync_ = false;
            return;
          }
        }
      }
      
      is_in_sync_ = true;
      last_response_ = { true, "" };
    }

    void LocalRecorder::SetCommand(const RecorderCommand& command)
    {
      if (!connection_activated_)
        return;

      last_response_ = { true, "" };

      switch (command.type_)
      {
      case RecorderCommand::Type::INITIALIZE:
        ecal_rec_instance_->ConnectToEcal();
        should_be_connected_to_ecal_ = true;
        break;
      case RecorderCommand::Type::DE_INITIALIZE:
        ecal_rec_instance_->DisconnectFromEcal();
        should_be_connected_to_ecal_ = false;
        break;
      case RecorderCommand::Type::START_RECORDING:
        if (!ecal_rec_instance_->StartRecording(command.job_config_))
        {
          should_be_connected_to_ecal_ = true;
          last_response_ = { false, "Unable to start recording." };
        }
        break;
      case RecorderCommand::Type::STOP_RECORDING:
        if (!ecal_rec_instance_->StopRecording())
        {
          last_response_ = { false, "Unable to stop recording." };
        }
        break;
      case RecorderCommand::Type::SAVE_PRE_BUFFER:
        if (!ecal_rec_instance_->SavePreBufferedData(command.job_config_))
        {
          last_response_ = { false, "Unable to save pre-buffer." };
        }
        break;
      case RecorderCommand::Type::ADD_SCENARIO:
        // TODO: implement
        break;
      case RecorderCommand::Type::EXIT:
        // TODO: remove or implement
        break;
      default:
        break;
      }
    }

    void LocalRecorder::InitiateConnectionShutdown(const RecorderCommand& last_command)
    {
      SetCommand(last_command);
      connection_activated_ = false;
      is_in_sync_           = false;
    }

    bool LocalRecorder::IsAlive() const
    {
      return connection_activated_;
    }

    RecorderState LocalRecorder::GetState() const
    {
      return ecal_rec_instance_->GetRecorderState();
    }

    bool LocalRecorder::IsRequestPending() const
    {
      return false;
    }

    void LocalRecorder::WaitForPendingRequests() const
    {}

    std::pair<bool, std::string> LocalRecorder::GetLastResponse() const
    {
      return last_response_;
    }

  }
}