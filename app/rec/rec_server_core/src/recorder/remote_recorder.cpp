/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include "remote_recorder.h"

#include <rec_client_core/proto_helpers.h>

#include <rec_client_core/ecal_rec_logger.h>

#include <ecal_utils/string.h>

#include <algorithm>
#include <clocale>

namespace eCAL
{
  namespace rec_server
  {

    //////////////////////////////////////////
    // Constructor & Destructor
    //////////////////////////////////////////

    RemoteRecorder::RemoteRecorder(const std::string& hostname
                                  , const std::function<void(const std::string& hostname, const eCAL::rec::RecorderStatus& recorder_status)>& update_jobstatus_function
                                  , const std::function<void(int64_t job_id, const std::string& hostname, const std::pair<bool, std::string>& info_command_response)>& report_job_command_response_callback
                                  , const RecorderSettings& initial_settings)
      : AbstractRecorder(hostname, update_jobstatus_function, report_job_command_response_callback)
      , InterruptibleThread()
      , next_ping_time_                     (std::chrono::steady_clock::time_point(std::chrono::steady_clock::duration(0)))
      , currently_executing_action_         (false)
      , recorder_enabled_                   (false)
      , connected_pid_                      (0)
      , client_in_sync_                     (false)
      , recorder_alive_                     (false)
      , ever_participated_in_a_measurement_ (false)
      , last_response_                      ({true, ""})
      , should_be_connected_to_ecal_        (false)
      , complete_settings_                  (initial_settings)
    {
      // Bind the recorder_service_ to the hostname
      recorder_service_.SetHostName(hostname);

      // Initial Ping => to perform auto recovery, which also sets the initial settings
      actions_to_perform_.emplace_back(Action());

      Start();
    }

    RemoteRecorder::~RemoteRecorder()
    {
      // call the function via its class becase it's a virtual function that is called in constructor/destructor,-
      // where the vtable is not created yet or it's destructed.
      RemoteRecorder::Interrupt();
      Join();
    }

    //////////////////////////////////////////
    // Public API
    //////////////////////////////////////////

    void RemoteRecorder::SetRecorderEnabled(bool enabled, bool connect_to_ecal)
    {
      std::lock_guard<decltype(io_mutex_)> io_lock(io_mutex_);

      // ENABLE recorder
      if (!recorder_enabled_ && enabled)
      {
        recorder_enabled_ = true;

        QueueSetSettings_NoLock(complete_settings_); // TODO: do I need this? The recorder is in un-synced state and will set all settings anyways

        if (connect_to_ecal)
        {
          // De-initialize the recorder
          RecorderCommand initialize_command;
          initialize_command.type_ = RecorderCommand::Type::INITIALIZE;

          QueueSetCommand_NoLock(initialize_command);
        }

        // Inform the thread about the settings AND the initialize command
        io_cv_.notify_all();
      }

      // DISABLE recorder
      else if (recorder_enabled_ && !enabled)
      {
        recorder_enabled_ = false;
        client_in_sync_   = false;

        // Remove all unfinished actions from the action queue
        actions_to_perform_.clear();

        if (should_be_connected_to_ecal_ && recorder_alive_)
        {
          // De-initialize the recorder
          RecorderCommand de_initialize_command;
          de_initialize_command.type_ = RecorderCommand::Type::DE_INITIALIZE;

          QueueSetCommand_NoLock(de_initialize_command);

          // Inform the thread about the De-Initialize command
          io_cv_.notify_all();
        }

      }
    }

    bool RemoteRecorder::IsRecorderEnabled() const
    {
      return recorder_enabled_;
    }

    bool RemoteRecorder::EverParticipatedInAMeasurement() const
    {
      return ever_participated_in_a_measurement_;
    }

    void RemoteRecorder::SetSettings(const RecorderSettings& settings)
    {
      std::lock_guard<decltype(io_mutex_)> io_lock (io_mutex_);

      // Add the settings to the complete settings. This is important in cases where we want to set everything, e.g. when connecting and syncing a new ecal_rec client instance
      complete_settings_.AddSettings(settings);

      // Only send the settings to the client if this recorder is enabled
      if (recorder_enabled_ && IsRunning())
      {
        // Add the settings to the action queue
        QueueSetSettings_NoLock(settings);

        // Notify the thread that there is something new in the queue
        io_cv_.notify_all();
      }
    }

    void RemoteRecorder::SetCommand(const RecorderCommand& command)
    {
      std::lock_guard<decltype(io_mutex_)> io_lock(io_mutex_);

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


      // Add command to the action queue
      QueueSetCommand_NoLock(command);

      // Notify the thread that we have more work to do
      io_cv_.notify_all();
    }

    bool RemoteRecorder::IsAlive() const
    {
      std::lock_guard<decltype(io_mutex_)> io_lock(io_mutex_);
      return recorder_alive_;
    }

    std::pair<eCAL::rec::RecorderStatus, eCAL::Time::ecal_clock::time_point> RemoteRecorder::GetStatus() const
    {
      std::lock_guard<decltype(io_mutex_)> io_lock(io_mutex_);
      return {last_status_, last_status_timestamp_};
    }

    bool RemoteRecorder::IsRequestPending() const
    {
      std::lock_guard<decltype(io_mutex_)> io_lock(io_mutex_);
      return IsRunning() && ((actions_to_perform_.size() > 0) || currently_executing_action_);
    }

    void RemoteRecorder::WaitForPendingRequests() const
    {
      std::unique_lock<decltype(io_mutex_)> io_lock(io_mutex_);

      if (IsRunning() && !IsInterrupted())
      {
        const auto pred = [this]() { return IsInterrupted() || ((actions_to_perform_.size() == 0) && !currently_executing_action_); };

        io_cv_.wait(io_lock, pred);
      }
    }

    std::pair<bool, std::string> RemoteRecorder::GetLastResponse() const
    {
      return last_response_;
    }

    //////////////////////////////////////////
    // InterruptibleThread overrides
    //////////////////////////////////////////

    void RemoteRecorder::Run()
    {
      while (!IsInterrupted())
      {
        Action this_loop_action;

        {
          std::unique_lock<decltype(io_mutex_)> io_lock(io_mutex_);

          currently_executing_action_ = false;

          // Notify the io_cv, as it is also used to wait for 
          if (actions_to_perform_.size() == 0)
          {
            io_cv_.notify_all();
          }

          // Wait until new settings / a new command is ready or until it is time to ping again
          io_cv_.wait_until(io_lock, next_ping_time_, [=] { return IsInterrupted() || ((actions_to_perform_.size() > 0) && (!actions_to_perform_.front().IsPing())); });
          if (IsInterrupted()) return;

          // The default action is to just ping the client and retrieve the current state
          if (actions_to_perform_.size() > 0)
          {
            this_loop_action = std::move(actions_to_perform_.front());
            actions_to_perform_.pop_front();
          }

          currently_executing_action_ = !this_loop_action.IsPing();
        }

        if (IsInterrupted()) return;

        ////////////////////////////////////////////
        // Set Settings
        ////////////////////////////////////////////
        if (this_loop_action.IsSettings())
        {
          auto set_config_pb = SettingsToSettingsPb(this_loop_action.settings_);
          eCAL::pb::rec_client::Response response;

          bool call_successfull = CallRecorderService("SetConfig", set_config_pb, response);

          if (IsInterrupted()) return;

          {
            std::unique_lock<decltype(io_mutex_)> io_lock(io_mutex_);

            if (call_successfull)
            {
              bool set_settings_successfull = (response.result() == eCAL::pb::rec_client::ServiceResult::success);

              recorder_alive_ = true;
              client_in_sync_ = set_settings_successfull;

              if (set_settings_successfull)
              {
                eCAL::rec::EcalRecLogger::Instance()->info("Sucessfully set settings for recorder on " + hostname_);
              }
              else
              {
                eCAL::rec::EcalRecLogger::Instance()->error("Failed setting settings for recorder on " + hostname_ + ": " + response.error());
                actions_to_perform_.emplace_front(Action(true)); // Next just ping the client to see if we can recover
              }

              last_response_ = { set_settings_successfull, response.error() };
            }
            else
            {
              recorder_alive_ = false;
              client_in_sync_ = false;
              eCAL::rec::EcalRecLogger::Instance()->error("Failed setting settings for recorder on " + hostname_ + ": Unable to contact recorder");
              actions_to_perform_.emplace_front(Action(true)); // Next just ping the client to see if we can recover
              last_response_ = { false, "Unable to contact recorder" };
            }
          }
        }
        ////////////////////////////////////////////
        // Send command
        ////////////////////////////////////////////
        else if (this_loop_action.IsCommand())
        {
          // Even if sending the command should be unsuccessfull, we save the connected to ecal state for later recovery
          if ((this_loop_action.command_.type_ == RecorderCommand::Type::INITIALIZE)
            || (this_loop_action.command_.type_ == RecorderCommand::Type::START_RECORDING))
          {
            should_be_connected_to_ecal_ = true;
          }
          else if (this_loop_action.command_.type_ == RecorderCommand::Type::DE_INITIALIZE)
          {
            should_be_connected_to_ecal_ = false;
          }

          eCAL::pb::rec_client::CommandRequest command_request_pb = RecorderCommandToCommandPb(this_loop_action.command_);
          eCAL::pb::rec_client::Response response_pb;

          bool call_successfull = CallRecorderService("SetCommand", command_request_pb, response_pb);

          {
            std::unique_lock<decltype(io_mutex_)> io_lock(io_mutex_);

            if (call_successfull)
            {
              bool execute_command_successfull = (response_pb.result() == eCAL::pb::rec_client::ServiceResult::success);

              recorder_alive_ = true;

              if (execute_command_successfull)
              {
                eCAL::rec::EcalRecLogger::Instance()->info("Successfully sent command to " + hostname_);
              }
              else
              {
                eCAL::rec::EcalRecLogger::Instance()->error("Failed sending command to " + hostname_ + ": " + response_pb.error());
              }

              last_response_ = { execute_command_successfull, response_pb.error() };
            }
            else
            {
              recorder_alive_ = false;
              eCAL::rec::EcalRecLogger::Instance()->error("Failed sending command to " + hostname_ + ": Unable to contact recorder");
              actions_to_perform_.emplace_front(Action(true)); // Next just ping the client to see if we can recover
              last_response_ = { false, "Unable to contact recorder" };
            }

            // Report the last response
            switch (this_loop_action.command_.type_)
            {
            case eCAL::rec_server::RecorderCommand::Type::START_RECORDING:
              // Same as SAVE_PRE_BUFFER
            case eCAL::rec_server::RecorderCommand::Type::SAVE_PRE_BUFFER:
              report_job_command_response_callback_(this_loop_action.command_.job_config_.GetJobId(), hostname_, last_response_);
              break;
            case eCAL::rec_server::RecorderCommand::Type::UPLOAD_MEASUREMENT:
              report_job_command_response_callback_(this_loop_action.command_.upload_config_.meas_id_, hostname_, last_response_);
              break;
            case eCAL::rec_server::RecorderCommand::Type::ADD_COMMENT:
              // Same as DELETE MEASUREMENT
            case eCAL::rec_server::RecorderCommand::Type::DELETE_MEASUREMENT:
              report_job_command_response_callback_(this_loop_action.command_.meas_id_add_delete, hostname_, last_response_);
              break;
            default:
              break;
            }
          }
        }
        ////////////////////////////////////////////
        // Get State / Ping
        ////////////////////////////////////////////
        else
        {
          eCAL::pb::rec_client::GetStateRequest request;
          eCAL::pb::rec_client::State           state_response_pb;

          // Retrieve the state from the client
          auto before_call_timestamp = eCAL::Time::ecal_clock::now();

          bool call_success = CallRecorderService("GetState", request, state_response_pb);

          auto after_call_timestamp = eCAL::Time::ecal_clock::now();
          if (IsInterrupted()) return;

          if (call_success)
          {
            eCAL::rec::RecorderStatus last_status;
            std::string               hostname;

            eCAL::rec::proto_helpers::FromProtobuf(state_response_pb, hostname, last_status);

            int32_t                   client_pid  = state_response_pb.process_id();

            {
              std::lock_guard<decltype(io_mutex_)> io_lock(io_mutex_);

              // Correct the connected PID
              if (connected_pid_ != client_pid)
              {
                // Some recorder on the same host with a different PID has connected!
                if (!recorder_alive_)
                  eCAL::rec::EcalRecLogger::Instance()->info("New recorder on host " + hostname_ + " has connected");

                client_in_sync_ = false;
                connected_pid_  = client_pid;
              }
              else
              {
                // Log an info, if the recorder as not been alive, previously
                if (!recorder_alive_)
                  eCAL::rec::EcalRecLogger::Instance()->info("Recorder on host " + hostname_ + " has reconnected!");
              }

              update_jobstatus_function_(hostname_, last_status);

              last_status_     = std::move(last_status);
              if (after_call_timestamp >= before_call_timestamp)
              {
                last_status_timestamp_ = before_call_timestamp + (after_call_timestamp - before_call_timestamp) / 2;
              }
              else
              {
                last_status_timestamp_ = after_call_timestamp;
              }
              recorder_alive_ = true;

              // Auto recovery
              if (recorder_enabled_ && recorder_alive_ && !client_in_sync_)
              {
                // Remove all old autorecovery actions. We don't need them anyways, as we are adding new ones.
                removeAutorecoveryActions_NoLock();

                // (3) If the recorder is not connected but should be, we connect it again
                if (should_be_connected_to_ecal_ && !last_status_.initialized_)
                {
                  RecorderCommand connect_to_ecal_command;
                  connect_to_ecal_command.type_ = RecorderCommand::Type::INITIALIZE;
                  actions_to_perform_.emplace_front(Action(connect_to_ecal_command, true));
                }

                // (2) Set all settings again
                if ((actions_to_perform_.size() > 0) && actions_to_perform_.front().IsSettings())
                {
                  // Merge settings
                  actions_to_perform_.front().settings_ = complete_settings_;
                  actions_to_perform_.front().SetIsAutorecoveryAction(true);
                }
                else
                {
                  actions_to_perform_.emplace_front(Action(complete_settings_, true));
                }

                // (1) Stop recording (or entirely disconnect from ecal)
                if (!should_be_connected_to_ecal_ && last_status_.initialized_)
                {
                  // If the recorder is connected to ecal but shouldn't, we disconnect it first
                  RecorderCommand disconnect_command;
                  disconnect_command.type_ = RecorderCommand::Type::DE_INITIALIZE;
                  actions_to_perform_.emplace_front(Action(disconnect_command, true));
                }
                else
                {
                  // If the recorder is recording, we need to stop it
                  bool is_recording = false;
                  for (const eCAL::rec::JobStatus& job_status : last_status_.job_statuses_)
                  {
                    if (job_status.state_ == eCAL::rec::JobState::Recording)
                    {
                      is_recording = true;
                      break;
                    }
                  }

                  if (is_recording)
                  {
                    RecorderCommand stop_command;
                    stop_command.type_ = RecorderCommand::Type::STOP_RECORDING;
                    actions_to_perform_.emplace_front(Action(stop_command, true));
                  }
                }
              }
              else if (!recorder_alive_)
              {
                actions_to_perform_.emplace_front(Action(true));
              }
            }
          }
          else
          {
            std::lock_guard<decltype(io_mutex_)> io_lock(io_mutex_);

            // Log an info, if the recorder has been alive, previously
            if (recorder_alive_)
              eCAL::rec::EcalRecLogger::Instance()->info("Recorder on host " + hostname_ + " is not alive any more.");

            recorder_alive_ = false;
            last_response_  = { false, "Unable to contact recorder" };
          }

          next_ping_time_ = std::chrono::steady_clock::now() + std::chrono::milliseconds(200);
        }
      }
    }

    void RemoteRecorder::Interrupt()
    {
      InterruptibleThread::Interrupt();
      io_cv_.notify_all();
    }

    //////////////////////////////////////////
    // Converter Functions
    //////////////////////////////////////////

    eCAL::pb::rec_client::SetConfigRequest RemoteRecorder::SettingsToSettingsPb(const RecorderSettings& settings)
    {
      const char decimal_point = std::localeconv()->decimal_point[0]; // Decimal point for std::to_string de-localization
      
      eCAL::pb::rec_client::SetConfigRequest request;

      auto config = request.mutable_config()->mutable_items();

      if (settings.IsMaxPreBufferLengthSet())
      {
        std::string serialized_string;
        serialized_string = std::to_string(std::chrono::duration_cast<std::chrono::duration<double>>(settings.GetMaxPreBufferLength()).count());
        std::replace(serialized_string.begin(), serialized_string.end(), decimal_point, '.');
        (*config)["max_pre_buffer_length_secs"] = serialized_string;
      }
      if (settings.IsPreBufferingEnabledSet())
      {
        std::string serialized_string;
        serialized_string = (settings.GetPreBufferingEnabled() ? "true" : "false");
        (*config)["pre_buffering_enabled"] = serialized_string;
      }
      if (settings.IsHostFilterSet())
      {
        std::string serialized_string;
        serialized_string = EcalUtils::String::Join("\n", settings.GetHostFilter());
        (*config)["host_filter"] = serialized_string;
      }
      if (settings.IsRecordModeSet())
      {
        std::string serialized_string;
        serialized_string = (settings.GetRecordMode() == eCAL::rec::RecordMode::All ? "all" : (settings.GetRecordMode() == eCAL::rec::RecordMode::Blacklist ? "blacklist" : "whitelist"));
        (*config)["record_mode"] = serialized_string;
      }
      if (settings.IsListedTopicsSet())
      {
        std::string serialized_string;
        serialized_string = EcalUtils::String::Join("\n", settings.GetListedTopics());
        (*config)["listed_topics"] = serialized_string;
      }
      if (settings.IsEnabledAddonsSet())
      {
        std::string serialized_string;
        serialized_string = EcalUtils::String::Join("\n", settings.GetEnabledAddons());
        (*config)["enabled_addons"] = serialized_string;
      }

      return request;
    }

    eCAL::pb::rec_client::CommandRequest RemoteRecorder::RecorderCommandToCommandPb(const RecorderCommand& command)
    {
      eCAL::pb::rec_client::CommandRequest request;
      
      switch (command.type_)
      {
      case RecorderCommand::Type::NONE:
        request.set_command(eCAL::pb::rec_client::CommandRequest::none);
        break;
      case RecorderCommand::Type::INITIALIZE:
        request.set_command(eCAL::pb::rec_client::CommandRequest::initialize);
        break;
      case RecorderCommand::Type::DE_INITIALIZE:
        request.set_command(eCAL::pb::rec_client::CommandRequest::de_initialize);
        break;
      case RecorderCommand::Type::START_RECORDING:
        request.set_command(eCAL::pb::rec_client::CommandRequest::start_recording);
        SetJobConfig(request.mutable_command_params()->mutable_items(), command.job_config_);
        break;
      case RecorderCommand::Type::STOP_RECORDING:
        request.set_command(eCAL::pb::rec_client::CommandRequest::stop_recording);
        break;
      case RecorderCommand::Type::SAVE_PRE_BUFFER:
        request.set_command(eCAL::pb::rec_client::CommandRequest::save_pre_buffer);
        SetJobConfig(request.mutable_command_params()->mutable_items(), command.job_config_);
        break;
      case RecorderCommand::Type::UPLOAD_MEASUREMENT:
        request.set_command(eCAL::pb::rec_client::CommandRequest::upload_measurement);
        SetUploadConfig(request.mutable_command_params()->mutable_items(), command.upload_config_);
        break;
      case RecorderCommand::Type::ADD_COMMENT:
        request.set_command(eCAL::pb::rec_client::CommandRequest::add_comment);
        (*request.mutable_command_params()->mutable_items())["meas_id"] = std::to_string(command.meas_id_add_delete);
        (*request.mutable_command_params()->mutable_items())["comment"] = command.comment_;
        break;
      case RecorderCommand::Type::DELETE_MEASUREMENT:
        request.set_command(eCAL::pb::rec_client::CommandRequest::delete_measurement);
        (*request.mutable_command_params()->mutable_items())["meas_id"] = std::to_string(command.meas_id_add_delete);
        break;
      case RecorderCommand::Type::EXIT:
        request.set_command(eCAL::pb::rec_client::CommandRequest::exit);
        break;
      default:
        break;
      }

      return request;
    }

    void RemoteRecorder::SetJobConfig(google::protobuf::Map<std::string, std::string>* job_config_pb, const eCAL::rec::JobConfig& job_config)
    {
      (*job_config_pb)["meas_id"]              = std::to_string(job_config.GetJobId());
      (*job_config_pb)["meas_root_dir"]        = job_config.GetMeasRootDir();
      (*job_config_pb)["meas_name"]            = job_config.GetMeasName();
      (*job_config_pb)["description"]          = job_config.GetDescription();
      (*job_config_pb)["max_file_size_mib"]    = std::to_string(job_config.GetMaxFileSize());
      (*job_config_pb)["one_file_per_topic"]   = job_config.GetOneFilePerTopicEnabled() ? "true" : "false";
    }

    void RemoteRecorder::SetUploadConfig(google::protobuf::Map<std::string, std::string>* upload_config_pb, const eCAL::rec::UploadConfig& upload_config)
    {
      (*upload_config_pb)["protocol"]              = "FTP";
      (*upload_config_pb)["meas_id"]               = std::to_string(upload_config.meas_id_);
      (*upload_config_pb)["username"]              = upload_config.username_;
      (*upload_config_pb)["password"]              = upload_config.password_;
      (*upload_config_pb)["host"]                  = upload_config.host_;
      (*upload_config_pb)["port"]                  = std::to_string(upload_config.port_);
      (*upload_config_pb)["upload_path"]           = upload_config.upload_path_;
      (*upload_config_pb)["upload_metadata_files"] = upload_config.upload_metadata_files_ ? "true" : "false";
      (*upload_config_pb)["delete_after_upload"]   = upload_config.delete_after_upload_ ? "true" : "false";
    }


    //////////////////////////////////////////
    // Auxiliary helper methods
    //////////////////////////////////////////
    void RemoteRecorder::removeAutorecoveryActions_NoLock()
    {
      auto new_end = std::remove_if(actions_to_perform_.begin(), actions_to_perform_.end(), [](const Action& action) {return action.IsAutorecoveryAction(); });
      actions_to_perform_.erase(new_end, actions_to_perform_.end());
      io_cv_.notify_all();
    }

    void RemoteRecorder::QueueSetSettings_NoLock(const RecorderSettings& settings)
    {
      if ((actions_to_perform_.size() > 0)
        && (actions_to_perform_.back().IsSettings()))
      {
        // If there are settings that haven't been set, yet, we add the new ones, so they can all be set in one single call
        actions_to_perform_.back().settings_.AddSettings(settings);
      }
      else
      {
        // If there are no other settings that haven't been set, yet
        actions_to_perform_.emplace_back(Action(settings));
      }
    }

    void RemoteRecorder::QueueSetCommand_NoLock(const RecorderCommand& command)
    {
      actions_to_perform_.emplace_back(Action(command));
    }

    bool RemoteRecorder::CallRecorderService(const std::string& method_name, const google::protobuf::Message& request, google::protobuf::Message& response)
    {
      // The target (i.e. the hostname) has already been set in the Constructor.

      eCAL::v5::ServiceResponseVecT service_response_vec;
      constexpr int timeout_ms = 1000;
      if (recorder_service_.Call(method_name, request.SerializeAsString(), timeout_ms, &service_response_vec))
      {
        if (service_response_vec.size() > 0)
        {
          response.ParseFromString(service_response_vec[0].response);
          return true;
        }
      }
      return false;
    }
  }
}
