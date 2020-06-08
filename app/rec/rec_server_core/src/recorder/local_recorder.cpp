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

#include <rec_client_core/ecal_rec.h>

namespace eCAL
{
  namespace rec_server
  {
    //////////////////////////////////////
    // Constructor & Destructor
    //////////////////////////////////////

    LocalRecorder::LocalRecorder(const std::string& hostname
                                , const std::shared_ptr<eCAL::rec::EcalRec>& ecal_rec_instance
                                , const std::function<void(const std::string& hostname, const eCAL::rec::RecorderStatus& recorder_status)>& update_jobstatus_function
                                , const std::function<void(int64_t job_id, const std::string& hostname, const std::pair<bool, std::string>& info_command_response)>& report_job_command_response_callback
                                , const RecorderSettings& initial_settings)
      : AbstractRecorder                    (hostname, update_jobstatus_function, report_job_command_response_callback)
      , InterruptibleLoopThread             (std::chrono::milliseconds(100))
      , ecal_rec_instance_                  (ecal_rec_instance)
      , ever_participated_in_a_measurement_ (false)
      , recorder_enabled_                   (false)
      , is_in_sync_                         (false)
      , last_response_                      ({true, ""})
      , complete_settings_                  (initial_settings)
      , should_be_connected_to_ecal_        (false)
    {
      Start();
    }
    
    LocalRecorder::~LocalRecorder()
    {
      Interrupt();
      Join();
    }

    //////////////////////////////////////
    // Interruptible Thread overrrides
    //////////////////////////////////////

    void LocalRecorder::Loop()
    {
      std::lock_guard<decltype(ecal_rec_instance_and_status_mutex_)> ecal_rec_instance_and_status_lock(ecal_rec_instance_and_status_mutex_);
      last_status_ = ecal_rec_instance_->GetRecorderStatus();
      update_jobstatus_function_(hostname_, last_status_);
    }

    //////////////////////////////////////
    // Public API
    //////////////////////////////////////

    void LocalRecorder::SetRecorderEnabled(bool enabled, bool connect_to_ecal)
    {
      // ENABLE recorder
      if (!recorder_enabled_ && enabled)
      {
        recorder_enabled_ = true;
        SetSettings(complete_settings_);

        if (connect_to_ecal)
        {
          ecal_rec_instance_->ConnectToEcal();
          should_be_connected_to_ecal_ = true; 
        }
      }

      // DISABLE recorder
      else if (recorder_enabled_ && !enabled)
      {
        recorder_enabled_            = false;

        if (ecal_rec_instance_->IsConnectedToEcal())
          ecal_rec_instance_->DisconnectFromEcal();

        is_in_sync_ = false;
      }
    }

    bool LocalRecorder::IsRecorderEnabled() const
    {
      return recorder_enabled_;
    }

    bool LocalRecorder::EverParticipatedInAMeasurement() const
    {
      return ever_participated_in_a_measurement_;
    }

    void LocalRecorder::SetSettings(const RecorderSettings& settings)
    {
      complete_settings_.AddSettings(settings);

      if (!recorder_enabled_)
        return;

      const RecorderSettings& settings_to_set = (is_in_sync_ ? settings : complete_settings_);

      {
        std::lock_guard<decltype(ecal_rec_instance_and_status_mutex_)> ecal_rec_instance_and_status_lock(ecal_rec_instance_and_status_mutex_);

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

        if (settings_to_set.IsEnabledAddonsSet())
        {
          bool success = ecal_rec_instance_->SetEnabledAddons(settings_to_set.GetEnabledAddons());
          if (!success)
          {
            last_response_ = { false, "Unable to set enabled addons" };
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
      }
      
      is_in_sync_ = true;
      last_response_ = { true, "" };
    }

    void LocalRecorder::SetCommand(const RecorderCommand& command)
    {
      if (!recorder_enabled_
        && (command.type_ != RecorderCommand::Type::UPLOAD_MEASUREMENT)
        && (command.type_ != RecorderCommand::Type::ADD_COMMENT)
        && (command.type_ != RecorderCommand::Type::DELETE_MEASUREMENT)) // The UPLOAD_MEASURMENT, ADD_COMMENT and DELETE_MEASUREMENT command may always be sent
      {
        return;
      }

      if ((command.type_ == RecorderCommand::Type::SAVE_PRE_BUFFER)
        || (command.type_ == RecorderCommand::Type::START_RECORDING))
      {
        // Save whether we ever had a measurement started. Connections to
        // recorders that have a measurement started must not be cut, as the
        // measurement connection object is responsible for error message
        // handling
        ever_participated_in_a_measurement_ = true;
      }

      last_response_ = { true, "" };

      {
        std::lock_guard<decltype(ecal_rec_instance_and_status_mutex_)> ecal_rec_instance_and_status_lock(ecal_rec_instance_and_status_mutex_);

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
          report_job_command_response_callback_(command.job_config_.GetJobId(), hostname_, last_response_);
          break;
        case RecorderCommand::Type::STOP_RECORDING:
          if (!ecal_rec_instance_->StopRecording())
          {
            last_response_ = { false, "Unable to stop recording." };
          }
          report_job_command_response_callback_(command.job_config_.GetJobId(), hostname_, last_response_);
          break;
        case RecorderCommand::Type::SAVE_PRE_BUFFER:
          if (!ecal_rec_instance_->SavePreBufferedData(command.job_config_))
          {
            last_response_ = { false, "Unable to save pre-buffer." };
          }
          report_job_command_response_callback_(command.job_config_.GetJobId(), hostname_, last_response_);
          break;
        case RecorderCommand::Type::UPLOAD_MEASUREMENT:
        {
          eCAL::rec::Error error = ecal_rec_instance_->UploadMeasurement(command.upload_config_);
          if (error)
          {
            last_response_ = { true, "Unable to add comment: " + error.ToString() };
          }
          report_job_command_response_callback_(command.meas_id_add_delete, hostname_, last_response_);
          break;
        }
        case RecorderCommand::Type::ADD_COMMENT:
        {
          eCAL::rec::Error error = ecal_rec_instance_->AddComment(command.meas_id_add_delete, command.comment_);
          if (error)
          {
            last_response_ = { true, "Unable to add comment: " + error.ToString() };
          }
          report_job_command_response_callback_(command.meas_id_add_delete, hostname_, last_response_);
          break;
        }
        case RecorderCommand::Type::DELETE_MEASUREMENT:
        {
          eCAL::rec::Error error = ecal_rec_instance_->DeleteMeasurement(command.meas_id_add_delete);
          if (error)
          {
            last_response_ = { true, "Unable to delete measurement: " + error.ToString() };
          }
          report_job_command_response_callback_(command.meas_id_add_delete, hostname_, last_response_);
          break;
        }
        case RecorderCommand::Type::EXIT:
          // We cannot shut down the local recorder.
          break;
        default:
          break;
        }
      }
    }

    bool LocalRecorder::IsAlive() const
    {
      return true;
    }

    std::pair<eCAL::rec::RecorderStatus, eCAL::Time::ecal_clock::time_point> LocalRecorder::GetStatus() const
    {
      std::lock_guard<decltype(ecal_rec_instance_and_status_mutex_)> ecal_rec_instance_and_status_lock(ecal_rec_instance_and_status_mutex_);

      return { last_status_, last_status_.timestamp_ };

      //std::pair<eCAL::rec::RecorderStatus, eCAL::Time::ecal_clock::time_point> status = { ecal_rec_instance_->GetRecorderStatus(), eCAL::Time::ecal_clock::time_point(eCAL::Time::ecal_clock::duration(0))};
      //status.second = status.first.timestamp_;
      //return status;
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