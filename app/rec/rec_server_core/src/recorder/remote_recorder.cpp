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
      , next_ping_time_                     (std::chrono::steady_clock::time_point(std::chrono::steady_clock::duration(0)))
      , currently_executing_action_         (false)
      , recorder_enabled_                   (false)
      , connected_service_client_id_        {}
      , client_in_sync_                     (false)
      , ever_participated_in_a_measurement_ (false)
      , last_response_                      ({true, ""})
      , should_be_connected_to_ecal_        (false)
      , complete_settings_                  (initial_settings)
      , hostname_                           (hostname)
    {
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
      std::lock_guard<decltype(io_mutex_)> const io_lock(io_mutex_);

      // ENABLE recorder
      if (!recorder_enabled_ && enabled)
      {
        recorder_enabled_ = true;

        if (connected_service_client_id_.entity_id != 0)
        {
          // Make sure that the recorder is in sync
          actions_to_perform_.emplace_front(true); // Action(true) indicates a Ping followed by forced autorecovery actions
        }

        if (connect_to_ecal)
        {
          if (connected_service_client_id_.entity_id != 0)
          {
            // Initialize the recorder
            RecorderCommand initialize_command;
            initialize_command.type_ = RecorderCommand::Type::INITIALIZE;

            QueueSetCommand_NoLock(initialize_command);
          }
          else
          {
            should_be_connected_to_ecal_ = connect_to_ecal;
          }
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

        if (should_be_connected_to_ecal_ && (connected_service_client_id_.entity_id != 0))
        {
          // De-initialize the recorder
          RecorderCommand de_initialize_command;
          de_initialize_command.type_ = RecorderCommand::Type::DE_INITIALIZE;

          QueueSetCommand_NoLock(de_initialize_command);

        }
        // Inform the thread about the De-Initialize command
        // Also, the WaitForPendingRequests may be waiting on the actions_to_perform_ list, which has been cleared.
        io_cv_.notify_all();
      }
    }

    bool RemoteRecorder::IsRecorderEnabled() const
    {
      std::lock_guard<decltype(io_mutex_)> const io_lock(io_mutex_);
      return recorder_enabled_;
    }

    bool RemoteRecorder::EverParticipatedInAMeasurement() const
    {
      return ever_participated_in_a_measurement_;
    }

    void RemoteRecorder::SetSettings(const RecorderSettings& settings)
    {
      std::lock_guard<decltype(io_mutex_)> const io_lock(io_mutex_);

      // Add the settings to the complete settings. This is important in cases where we want to set everything, e.g. when connecting and syncing a new ecal_rec client instance
      complete_settings_.AddSettings(settings);

      // Only send the settings to the client if this recorder is enabled and connected.
      // If the recorder is not connected, the complete settings will be set via the auto-recovery upon successful connection.
      if (recorder_enabled_ && (connected_service_client_id_.entity_id != 0) && IsRunning())
      {
        // Add the settings to the action queue
        QueueSetSettings_NoLock(settings);

        // Notify the thread that there is something new in the queue
        io_cv_.notify_all();
      }
    }

    void RemoteRecorder::SetCommand(const RecorderCommand& command)
    {
      std::lock_guard<decltype(io_mutex_)> const io_lock(io_mutex_);

      if (!IsRunning() || IsInterrupted())
        return;

      // Return, if the recorder is not enabled.
      // The UPLOAD_MEASURMENT, ADD_COMMENT and DELETE_MEASUREMENT command may always be sent, even if the recorder is not enabled anymore
      if (!recorder_enabled_
        && (command.type_ != RecorderCommand::Type::UPLOAD_MEASUREMENT)
        && (command.type_ != RecorderCommand::Type::ADD_COMMENT)
        && (command.type_ != RecorderCommand::Type::DELETE_MEASUREMENT))
      {
        return;
      }

      // Safe the connected-to-ecal state for a potential autorecovery
      if ((command.type_ == RecorderCommand::Type::INITIALIZE)
        || (command.type_ == RecorderCommand::Type::START_RECORDING))
      {
        should_be_connected_to_ecal_ = true;
      }
      else if (command.type_ == RecorderCommand::Type::DE_INITIALIZE)
      {
        should_be_connected_to_ecal_ = false;
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
      std::lock_guard<decltype(io_mutex_)> const io_lock(io_mutex_);
      return (connected_service_client_id_.entity_id != 0);
    }

    std::pair<eCAL::rec::RecorderStatus, eCAL::Time::ecal_clock::time_point> RemoteRecorder::GetStatus() const
    {
      std::lock_guard<decltype(io_mutex_)> const io_lock(io_mutex_);
      return {last_status_, last_status_timestamp_};
    }

    bool RemoteRecorder::IsRequestPending() const
    {
      std::lock_guard<decltype(io_mutex_)> const io_lock(io_mutex_);
      return IsRunning()
            && (connected_service_client_id_.entity_id != 0)
            && ((!actions_to_perform_.empty()) || currently_executing_action_);
    }

    void RemoteRecorder::WaitForPendingRequests() const
    {
      std::unique_lock<decltype(io_mutex_)> io_lock(io_mutex_);
      
      // If the recorder is not connected, we instantly return
      if (connected_service_client_id_.entity_id == 0)
      {
        return;
      }
      
      // If the recorder is connected, we wait until the queue has been emptied
      if (IsRunning() && !IsInterrupted())
      {
        const auto pred = [this]()
                          {
                            return IsInterrupted() || ((actions_to_perform_.empty()) && !currently_executing_action_);
                          };

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
        // TODO: Only connect to the recorder, if it is enabled. Currently, we always try to connect.
        
        ////////////////////////////////////////////
        // Connect to free recorder
        ////////////////////////////////////////////
        bool is_connected = false;
        {
          std::lock_guard<decltype(io_mutex_)> const io_lock(io_mutex_);
          is_connected = (connected_service_client_id_.entity_id != 0);
        }
        
        if (!is_connected) // Try to connect to a new recorder and set the last status
        {
          is_connected = ConnectToFreeRecorderService();
          
          if (IsInterrupted()) return;
          
          if (is_connected) // Connection successful!
          {
            std::lock_guard<decltype(io_mutex_)> const io_lock(io_mutex_);
            
            // The new connection has also set a new status, so we need to call the update_jobstatus_function_
            update_jobstatus_function_(hostname_, last_status_);
            
            // Add auto-recovery actions to the Queue front, so that setting the settings and getting the client in sync is prioritized.
            client_in_sync_ = false; // We need to set all settings again, as we don't know which state the new recorder is in
            if (recorder_enabled_)
            {
              QueueAutoRecoveryCommandsBasedOnLastStatus_NoLock();
            }
          }
          else // Connection was not successful
          {
            // Wait a short time before trying to connect the next time
            std::unique_lock<decltype(io_mutex_)> io_lock(io_mutex_);
            io_cv_.wait_for(io_lock, std::chrono::milliseconds(200), [this] { return IsInterrupted(); });
            continue;
          }
        }
        
        
        ////////////////////////////////////////////
        // Get next action to perform
        ////////////////////////////////////////////

        Action this_loop_action;

        {
          std::unique_lock<decltype(io_mutex_)> io_lock(io_mutex_);

          // Wait until new settings / a new command is ready or until it is time to ping again
          io_cv_.wait_until(io_lock, next_ping_time_
                            , [this]
                              {
                                return IsInterrupted()
                                      || ((!actions_to_perform_.empty()) && (!actions_to_perform_.front().IsPing()));
                              });
          if (IsInterrupted()) return;

          // The default action is to just ping the client and retrieve the current state
          if (!actions_to_perform_.empty())
          {
            this_loop_action = std::move(actions_to_perform_.front());
            actions_to_perform_.pop_front();
          }

          currently_executing_action_ = !this_loop_action.IsPing();
          
          // Notify the io_cv_, as the WaitForPendingRequests function may be waiting on currently_executing_action_ and the length of actions_to_perform_
          io_cv_.notify_all();
        }

        if (IsInterrupted()) return;

        ////////////////////////////////////////////
        // Set Settings
        ////////////////////////////////////////////
        if (this_loop_action.IsSettings())
        {
          auto const set_config_pb = SettingsToSettingsPb(this_loop_action.settings_);
          eCAL::pb::rec_client::Response response;

          bool const call_successful = CallRecorderService("SetConfig", set_config_pb, response);

          if (IsInterrupted()) return;
          
          
          if (call_successful)
          {
            std::unique_lock<decltype(io_mutex_)> const io_lock(io_mutex_);
            bool const set_settings_successful = (response.result() == eCAL::pb::rec_client::ServiceResult::success);

            client_in_sync_ = set_settings_successful;

            if (set_settings_successful)
            {
              eCAL::rec::EcalRecLogger::Instance()->info("Successfully set settings for recorder on " + hostname_);
            }
            else
            {
              eCAL::rec::EcalRecLogger::Instance()->error("Failed setting settings for recorder on " + hostname_ + ": " + response.error());

              if (recorder_enabled_)
              {
                QueueAutoRecoveryCommandsBasedOnLastStatus_NoLock(); // The client is not in sync anymore, so we need to recover from that
              }
            }

            last_response_ = { set_settings_successful, response.error() };
          }
        }
        ////////////////////////////////////////////
        // Send command
        ////////////////////////////////////////////
        else if (this_loop_action.IsCommand())
        {
          eCAL::pb::rec_client::CommandRequest const command_request_pb = RecorderCommandToCommandPb(this_loop_action.command_);
          eCAL::pb::rec_client::Response             response_pb;

          bool const call_successful  = CallRecorderService("SetCommand", command_request_pb, response_pb);

          {
            std::unique_lock<decltype(io_mutex_)> const io_lock(io_mutex_);

            if (call_successful)
            {
              bool const execute_command_successful = (response_pb.result() == eCAL::pb::rec_client::ServiceResult::success);

              if (execute_command_successful)
              {
                eCAL::rec::EcalRecLogger::Instance()->info("Successfully sent command to " + hostname_);
              }
              else
              {
                eCAL::rec::EcalRecLogger::Instance()->error("Failed sending command to " + hostname_ + ": " + response_pb.error());
              }

              last_response_ = { execute_command_successful, response_pb.error() };
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
          eCAL::pb::rec_client::GetStateRequest const request;
          eCAL::pb::rec_client::State                 state_response_pb;

          // Retrieve the state from the client
          auto const before_call_timestamp = eCAL::Time::ecal_clock::now();
          bool const call_success = CallRecorderService("GetState", request, state_response_pb);
          auto const after_call_timestamp = eCAL::Time::ecal_clock::now();

          if (IsInterrupted()) return;

          if (call_success)
          {
            std::lock_guard<decltype(io_mutex_)> const io_lock(io_mutex_);

            {
              eCAL::rec::RecorderStatus last_status;
              std::string               hostname;
              eCAL::rec::proto_helpers::FromProtobuf(state_response_pb, hostname, last_status);
              last_status_ = std::move(last_status); // Necessary, as FromProtobuf expects a clean output-variable
            }

            update_jobstatus_function_(hostname_, last_status_);

            if (after_call_timestamp >= before_call_timestamp)
            {
              last_status_timestamp_ = before_call_timestamp + (after_call_timestamp - before_call_timestamp) / 2;
            }
            else
            {
              last_status_timestamp_ = after_call_timestamp;
            }

            // Check if we are supposed to force an autorecovery (idicated from a Ping Action that has autorecovery set to true)
            if (this_loop_action.IsAutorecoveryAction())
            {
              QueueAutoRecoveryCommandsBasedOnLastStatus_NoLock();
            }
          }
        }
        
        {
          // Reset the currently_executing_action_ to false
          std::lock_guard<decltype(io_mutex_)> const io_lock(io_mutex_);
          currently_executing_action_ = false;
          
          // Also notify the io_cv_, as the WaitForPendingRequests may depend on the currently_executing_action_ variable
          io_cv_.notify_all();
        }
        
        next_ping_time_ = std::chrono::steady_clock::now() + std::chrono::milliseconds(200);
      }
    }

    void RemoteRecorder::Interrupt()
    {
      std::unique_lock<decltype(io_mutex_)> const io_lock(io_mutex_);
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

      auto *config = request.mutable_config()->mutable_items();

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
        switch (settings.GetRecordMode())
        {
        case eCAL::rec::RecordMode::All:
          serialized_string = "all";
          break;
        case eCAL::rec::RecordMode::Blacklist:
          serialized_string = "blacklist";
          break;
        default:
          serialized_string = "whitelist";
        }
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

    void RemoteRecorder::QueueAutoRecoveryCommandsBasedOnLastStatus_NoLock()
    {
      // Remove all old autorecovery actions. We don't need them anyways, as we are adding new ones.
      auto new_end = std::remove_if(actions_to_perform_.begin(), actions_to_perform_.end(), [](const Action& action) {return action.IsAutorecoveryAction(); });
      actions_to_perform_.erase(new_end, actions_to_perform_.end());
      
      // (3) If the recorder is not connected but should be, we connect it again
      if (should_be_connected_to_ecal_ && !last_status_.initialized_)
      {
        RecorderCommand connect_to_ecal_command;
        connect_to_ecal_command.type_ = RecorderCommand::Type::INITIALIZE;
        actions_to_perform_.emplace_front(connect_to_ecal_command, true);
      }
      
      // (2) Set all settings again
      if ((!actions_to_perform_.empty()) && actions_to_perform_.front().IsSettings()) {
        // Merge settings
        actions_to_perform_.front().settings_ = complete_settings_;
        actions_to_perform_.front().SetIsAutorecoveryAction(true);
      } else {
        actions_to_perform_.emplace_front(complete_settings_, true);
      }

      // (1) Stop recording (or entirely disconnect from ecal)
      if (!should_be_connected_to_ecal_ && last_status_.initialized_)
      {
        // If the recorder is connected to ecal but shouldn't, we disconnect it first
        RecorderCommand disconnect_command;
        disconnect_command.type_ = RecorderCommand::Type::DE_INITIALIZE;
        actions_to_perform_.emplace_front(disconnect_command, true);
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
          actions_to_perform_.emplace_front(stop_command, true);
        }
      }
    }

    void RemoteRecorder::QueueSetSettings_NoLock(const RecorderSettings& settings)
    {
      if ((!actions_to_perform_.empty()) && (actions_to_perform_.back().IsSettings()))
      {
        // If there are settings that haven't been set, yet, we add the new ones, so they can all be set in one single call
        actions_to_perform_.back().settings_.AddSettings(settings);
      }
      else
      {
        // If there are no other settings that haven't been set, yet
        actions_to_perform_.emplace_back(settings);
      }
    }

    void RemoteRecorder::QueueSetCommand_NoLock(const RecorderCommand& command)
    {
      actions_to_perform_.emplace_back(command);
    }

    bool RemoteRecorder::ConnectToFreeRecorderService()
    {
      auto client_instances = recorder_service_.GetClientInstances();
      for (auto& client_instance : client_instances)
      {
        if (IsInterrupted())
          return false;

        if (client_instance.GetClientID().host_name == hostname_)
        {
          eCAL::pb::rec_client::GetStateRequest const request;
          eCAL::pb::rec_client::State                 state_response_pb;

          // TODO:  enrich the getstaterequest with a "desire to own the client" functionality

          auto before_call_timestamp = eCAL::Time::ecal_clock::now();
          auto client_instance_response_pair = client_instance.CallWithResponse("GetState", request, 0);
          auto after_call_timestamp = eCAL::Time::ecal_clock::now();

          if (!client_instance_response_pair.first)
          {
            // If contacting the recorder at all failed, continue to try the next one
            continue;
          }
          else
          {
            state_response_pb.ParseFromString(client_instance_response_pair.second.response);
            // TODO: Check if owning the rec client was successful

            {
              std::lock_guard<decltype(io_mutex_)> const io_lock(io_mutex_);

              // Set the connected client ID, which is used to see if the connection has been successful
              connected_service_client_id_ = client_instance.GetClientID();

              // Log the successful connection
              std::stringstream ss;
              ss << "Connected to recorder service for host " << hostname_
                 << " (PID: " << connected_service_client_id_.process_id
                 << ", EntityId: " << connected_service_client_id_.entity_id << ")";
              eCAL::rec::EcalRecLogger::Instance()->info(ss.str());

              // Set the last status of the connected client. The last status is used by the autorecovery, so it is important that we set it.
              {
                eCAL::rec::RecorderStatus last_status;
                std::string               hostname;
                eCAL::rec::proto_helpers::FromProtobuf(state_response_pb, hostname, last_status);
                last_status_ = std::move(last_status); // Necessary, as FromProtobuf expects a clean variable as output
              }
              if (after_call_timestamp >= before_call_timestamp)
              {
                last_status_timestamp_ = before_call_timestamp + (after_call_timestamp - before_call_timestamp) / 2;
              }
              else
              {
                last_status_timestamp_ = after_call_timestamp;
              }

            }

            return true;
          }
        }
      }
      return false;
    }

    bool RemoteRecorder::CallRecorderService(const std::string& method_name, const google::protobuf::Message& request, google::protobuf::Message& response)
    {
      auto client_instances = recorder_service_.GetClientInstances();
      for (auto& client_instance : client_instances)
      {
        if (client_instance.GetClientID() == connected_service_client_id_)
        {
          auto client_instance_response = client_instance.CallWithResponse(method_name, request);

          if (!client_instance_response.first)
          {
            // The Client was not reachable
            eCAL::rec::EcalRecLogger::Instance()->error("Failed sending message to " + hostname_
                                                      + ": Connected recorder is unreachable (PID: "
                                                      + std::to_string(connected_service_client_id_.process_id)
                                                      + ", Service ID: "
                                                      + std::to_string(connected_service_client_id_.entity_id)
                                                      + ")");

            {
              std::lock_guard<decltype(io_mutex_)> const io_lock(io_mutex_);

              connected_service_client_id_ = eCAL::SEntityId();
              last_response_ = { false, "Unable to contact recorder" };

              // Notify the io_cv, as the WaitForPendingRequests function also returns if the recorder is not connected, anymore
              io_cv_.notify_all();
            }

            return false;
          }
          else
          {
            // The Client was reachable. The response message is returned. It may still contain an error that is reported by the client.
            response.ParseFromString(client_instance_response.second.response);
            return true;
          }
        }
      }

      // The client that we had previously connected to does not appear in the list anymore.
      eCAL::rec::EcalRecLogger::Instance()->error("Failed sending message to " + hostname_
                                                + ": Previously connected recorder does not exist anymore (PID: "
                                                + std::to_string(connected_service_client_id_.process_id)
                                                + ", Service ID: "
                                                + std::to_string(connected_service_client_id_.entity_id)
                                                + ")");

      {
        std::lock_guard<decltype(io_mutex_)> const io_lock(io_mutex_);

        connected_service_client_id_ = eCAL::SEntityId();
        last_response_               = { false, "Unable to contact recorder" };

        // Notify the io_cv, as the WaitForPendingRequests function also returns if the recorder is not connected, anymore
        io_cv_.notify_all();
      }
      return false;
    }
  }
}
