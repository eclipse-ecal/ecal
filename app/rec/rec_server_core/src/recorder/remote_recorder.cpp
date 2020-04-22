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

#include "remote_recorder.h"

#include <rec_core/ecal_rec_logger.h>

#include <ecal_utils/string.h>

#include <algorithm>

namespace eCAL
{
  namespace rec
  {

    //////////////////////////////////////////
    // Constructor & Destructor
    //////////////////////////////////////////

    RemoteRecorder::RemoteRecorder(const std::string& hostname, const RecorderSettings& initial_settings, bool initially_connected_to_ecal)
      : AbstractRecorder(hostname)
      , InterruptibleThread()
      , next_ping_time_             (std::chrono::steady_clock::time_point(std::chrono::steady_clock::duration(0)))
      , currently_executing_action_ (false)
      , connected_pid_              (0)
      , client_in_sync_             (false)
      , recorder_alive_             (false)
      , last_response_              ({true, ""})
      , should_be_connected_to_ecal_(initially_connected_to_ecal)
      , complete_settings_          (initial_settings)
      , connection_shutting_down_   (true)
    {
      // Initial Ping => to perform auto recovery, which also sets the initial settings
      actions_to_perform_.push_back(std::move(Action()));
    }

    RemoteRecorder::~RemoteRecorder()
    {
      Interrupt();
      Join();
    }

    //////////////////////////////////////////
    // Public API
    //////////////////////////////////////////

    void RemoteRecorder::SetClientConnectionEnabled(bool connect)
    {
      if (connect && !IsRunning())
      {
        Join(); // Just in case an old thread hasn't finished properly, yet

        connection_shutting_down_ = false;
        connected_pid_            = 0;
        client_in_sync_           = false;
        recorder_alive_           = false;

        Start();
      }
      else if (!connect && IsRunning())
      {
        Interrupt();

        std::lock_guard<decltype(io_mutex_)> io_lock(io_mutex_);

        connection_shutting_down_ = true;
        connected_pid_            = 0;
        client_in_sync_           = false;
        recorder_alive_           = false;
      }
    }

    bool RemoteRecorder::IsClientConnectionEnabled() const
    {
      return IsRunning();
    }

    void RemoteRecorder::SetSettings(const RecorderSettings& settings)
    {
      std::lock_guard<decltype(io_mutex_)> io_lock (io_mutex_);

      // Add the settings to the complete settings. This is important in cases where we want to set everything, e.g. when connecting and syncing a new ecal_rec client instance
      complete_settings_.AddSettings(settings);

      if (connection_shutting_down_ || !IsRunning()) return;

      if ((actions_to_perform_.size() > 0)
        && (actions_to_perform_.back().IsSettings()))
      {
        // If there are settings that haven't been set, yet, we add the new ones, so they can all be set in one single call
        actions_to_perform_.back().settings_.AddSettings(settings);
      }
      else
      {
        // If there are no other settings that haven't been set, yet
        actions_to_perform_.push_back(std::move(Action(settings)));
      }

      // Notify the thread that we have more work to do
      io_cv_.notify_all();
    }

    void RemoteRecorder::SetCommand(const RecorderCommand& command)
    {
      std::lock_guard<decltype(io_mutex_)> io_lock(io_mutex_);

      if (connection_shutting_down_ || !IsRunning()) return;

      actions_to_perform_.push_back(std::move(Action(command)));

      // Notify the thread that we have more work to do
      io_cv_.notify_all();
    }

    void RemoteRecorder::InitiateConnectionShutdown(const RecorderCommand& last_command)
    {
      std::lock_guard<decltype(io_mutex_)> io_lock(io_mutex_);
      connection_shutting_down_ = true;
      actions_to_perform_.clear();
      actions_to_perform_.emplace_back(Action(last_command));

      // Notify the thread that we have more work to do
      io_cv_.notify_all();
    }

    bool RemoteRecorder::IsAlive() const
    {
      std::lock_guard<decltype(io_mutex_)> io_lock(io_mutex_);
      return recorder_alive_;
    }

    RecorderState RemoteRecorder::GetState() const
    {
      std::lock_guard<decltype(io_mutex_)> io_lock(io_mutex_);
      return last_state_;
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
        bool exit_after_execution;

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

          exit_after_execution = connection_shutting_down_;

          currently_executing_action_ = !this_loop_action.IsPing();
        }

        if (IsInterrupted()) return;

        ////////////////////////////////////////////
        // Set Settings
        ////////////////////////////////////////////
        if (this_loop_action.IsSettings())
        {
          auto set_config_pb = SettingsToSettingsPb(this_loop_action.settings_);
          eCAL::SServiceInfo      service_info;
          eCAL::pb::rec::Response response;

          bool call_successfull = recorder_service_.Call(hostname_, "SetConfig", set_config_pb, service_info, response);
          if (IsInterrupted()) return;

          {
            std::unique_lock<decltype(io_mutex_)> io_lock(io_mutex_);

            if (call_successfull)
            {
              bool set_settings_successfull = (response.result() == eCAL::pb::rec::eServiceResult::success);

              recorder_alive_ = true;
              client_in_sync_ = set_settings_successfull;

              if (set_settings_successfull)
              {
                EcalRecLogger::Instance()->info("Sucessfully set settings for recorder on " + hostname_);
              }
              else
              {
                EcalRecLogger::Instance()->error("Failed setting settings for recorder on " + hostname_ + ": " + response.error());
                actions_to_perform_.push_front(std::move(Action(true))); // Next just ping the client to see if we can recover
              }

              last_response_ = { set_settings_successfull, response.error() };
            }
            else
            {
              recorder_alive_ = false;
              client_in_sync_ = false;
              EcalRecLogger::Instance()->error("Failed setting settings for recorder on " + hostname_ + ": Unable to contact recorder");
              actions_to_perform_.push_front(std::move(Action(true))); // Next just ping the client to see if we can recover
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
          else if ((this_loop_action.command_.type_ == RecorderCommand::Type::DE_INITIALIZE))
          {
            should_be_connected_to_ecal_ = false;
          }

          eCAL::pb::rec::CommandRequest command_request_pb = RecorderCommandToCommandPb(this_loop_action.command_);

          eCAL::SServiceInfo      service_info;
          eCAL::pb::rec::Response response_pb;

          bool call_successfull = recorder_service_.Call(hostname_, "SetCommand", command_request_pb, service_info, response_pb);

          {
            std::unique_lock<decltype(io_mutex_)> io_lock(io_mutex_);

            if (call_successfull)
            {
              bool execute_command_successfull = (response_pb.result() == eCAL::pb::rec::eServiceResult::success);

              recorder_alive_ = true;

              if (execute_command_successfull)
              {
                EcalRecLogger::Instance()->info("Successfully sent command to " + hostname_);
              }
              else
              {
                EcalRecLogger::Instance()->error("Failed sending command to " + hostname_ + ": " + response_pb.error());
              }

              last_response_ = { execute_command_successfull, response_pb.error() };
            }
            else
            {
              recorder_alive_ = false;
              EcalRecLogger::Instance()->error("Failed sending command to " + hostname_ + ": Unable to contact recorder");
              actions_to_perform_.push_front(std::move(Action(true))); // Next just ping the client to see if we can recover
              last_response_ = { false, "Unable to contact recorder" };
            }
          }
        }
        ////////////////////////////////////////////
        // Get State / Ping
        ////////////////////////////////////////////
        else
        {
          eCAL::pb::rec::GetStateRequest request;

          eCAL::SServiceInfo   service_info;
          eCAL::pb::rec::State state_response_pb;

          // Retrieve the state from the client
          bool call_success = recorder_service_.Call(hostname_, "GetState", request, service_info, state_response_pb);
          if (IsInterrupted()) return;

          if (call_success)
          {
            RecorderState last_state = StatePbToRecorderState(state_response_pb);
            int32_t       client_pid = state_response_pb.pid();

            {
              std::lock_guard<decltype(io_mutex_)> io_lock(io_mutex_);

              // Correct the connected PID
              if (connected_pid_ != client_pid)
              {
                // Some recorder on the same host with a different PID has connected!
                if (!recorder_alive_)
                  EcalRecLogger::Instance()->info("New recorder on host " + hostname_ + " has connected");

                client_in_sync_ = false;
                connected_pid_ = client_pid;
              }
              else
              {
                // Log an info, if the recorder as not been alive, previously
                if (!recorder_alive_)
                  EcalRecLogger::Instance()->info("Recorder on host " + hostname_ + " has reconnected!");
              }

              last_state_     = std::move(last_state);
              recorder_alive_ = true;

              // Auto recovery
              if (recorder_alive_ && !client_in_sync_)
              {
                // Remove all old autorecovery actions. We don't need them anyways, as we are adding new ones.
                removeAutorecoveryActions_NoLock();

                // (3) If the recorder is not connected but should be, we connect it again
                if (should_be_connected_to_ecal_ && !last_state_.initialized_)
                {
                  RecorderCommand connect_to_ecal_command;
                  connect_to_ecal_command.type_ = RecorderCommand::Type::INITIALIZE;
                  actions_to_perform_.push_front(std::move(Action(connect_to_ecal_command, true)));
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
                  actions_to_perform_.push_front(std::move(Action(complete_settings_, true)));
                }

                // (1) Stop recording (or entirely disconnect from ecal)
                if (!should_be_connected_to_ecal_ && last_state_.initialized_)
                {
                  // If the recorder is connected to ecal but shouldn't, we disconnect it first
                  RecorderCommand disconnect_command;
                  disconnect_command.type_ = RecorderCommand::Type::DE_INITIALIZE;
                  actions_to_perform_.push_front(std::move(Action(disconnect_command, true)));
                }
                else if (last_state.main_recorder_state_.recording_ && !last_state.main_recorder_state_.flushing_)
                {
                  // If the recorder is recording, we need to stop it
                  RecorderCommand stop_command;
                  stop_command.type_ = RecorderCommand::Type::STOP_RECORDING;
                  actions_to_perform_.push_front(std::move(Action(stop_command, true)));
                }
              }
              else if (!recorder_alive_)
              {
                actions_to_perform_.push_front(std::move(Action(true)));
              }
            }
          }
          else
          {
            std::lock_guard<decltype(io_mutex_)> io_lock(io_mutex_);

            // Log an info, if the recorder has been alive, previously
            if (recorder_alive_)
              EcalRecLogger::Instance()->info("Recorder on host " + hostname_ + " is not alive any more.");

            recorder_alive_ = false;
            last_response_  = { false, "Unable to contact recorder" };
          }

          next_ping_time_ = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);
        }

        // Shut down the connection
        if (exit_after_execution)
        {
          std::lock_guard<decltype(io_mutex_)> io_lock(io_mutex_);

          connected_pid_            = 0;
          client_in_sync_           = false;
          recorder_alive_           = false;

          actions_to_perform_.clear();
          currently_executing_action_ = false;

          io_cv_.notify_all();

          return;
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

    eCAL::pb::rec::SetConfigRequest RemoteRecorder::SettingsToSettingsPb(const RecorderSettings& settings)
    {
      eCAL::pb::rec::SetConfigRequest request;

      auto config = request.mutable_config()->mutable_items();

      if (settings.IsMaxPreBufferLengthSet())
      {
        std::string serialized_string;
        serialized_string = std::to_string(std::chrono::duration_cast<std::chrono::duration<double>>(settings.GetMaxPreBufferLength()).count());
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
        serialized_string = (settings.GetRecordMode() == RecordMode::All ? "all" : (settings.GetRecordMode() == RecordMode::Blacklist ? "blacklist" : "whitelist"));
        (*config)["record_mode"] = serialized_string;
      }
      if (settings.IsListedTopicsSet())
      {
        std::string serialized_string;
        serialized_string = EcalUtils::String::Join("\n", settings.GetListedTopics());
        (*config)["listed_topics"] = serialized_string;
      }

      return request;
    }

    RecorderState RemoteRecorder::StatePbToRecorderState(const eCAL::pb::rec::State& recorder_state_pb)
    {
      RecorderState recorder_state;

      recorder_state.initialized_ = recorder_state_pb.initialized();
      recorder_state.main_recorder_state_ = WriterStatePbToWriterState(recorder_state_pb.main_recorder_state());
      recorder_state.pre_buffer_length_.first = recorder_state_pb.pre_buffer_length_frames_count();
      recorder_state.pre_buffer_length_.second = std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(recorder_state_pb.pre_buffer_length_secs()));

      for (const auto& buffer_writer_state_pb : recorder_state_pb.buffer_writer_states())
      {
        recorder_state.buffer_writers_.push_back(WriterStatePbToWriterState(buffer_writer_state_pb));
      }

      for (const auto& subscribed_topic : recorder_state_pb.subscribed_topics())
      {
        recorder_state.subscribed_topics_.emplace(subscribed_topic);
      }

      return recorder_state;
    }

    WriterState RemoteRecorder::WriterStatePbToWriterState(const eCAL::pb::rec::State::WriterState& writer_state_pb)
    {
      WriterState writer_state;

      writer_state.recording_               = writer_state_pb.recording();
      writer_state.flushing_                = writer_state_pb.flushing();
      writer_state.recording_length_.first  = writer_state_pb.recording_length_frame_count();
      writer_state.recording_length_.second = std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(writer_state_pb.recording_length_secs()));
      writer_state.recording_queue_.first   = writer_state_pb.queued_frames_count();
      writer_state.recording_queue_.second  = std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(writer_state_pb.queued_secs()));

      return writer_state;
    }

    eCAL::pb::rec::CommandRequest RemoteRecorder::RecorderCommandToCommandPb(const RecorderCommand& command)
    {
      eCAL::pb::rec::CommandRequest request;
      
      switch (command.type_)
      {
      case RecorderCommand::Type::NONE:
        request.set_command(eCAL::pb::rec::CommandRequest::none);
        break;
      case RecorderCommand::Type::INITIALIZE:
        request.set_command(eCAL::pb::rec::CommandRequest::initialize);
        break;
      case RecorderCommand::Type::DE_INITIALIZE:
        request.set_command(eCAL::pb::rec::CommandRequest::de_initialize);
        break;
      case RecorderCommand::Type::START_RECORDING:
        request.set_command(eCAL::pb::rec::CommandRequest::start_recording);
        SetJobConfig(request.mutable_command_params()->mutable_items(), command.job_config_);
        break;
      case RecorderCommand::Type::STOP_RECORDING:
        request.set_command(eCAL::pb::rec::CommandRequest::stop_recording);
        break;
      case RecorderCommand::Type::SAVE_PRE_BUFFER:
        request.set_command(eCAL::pb::rec::CommandRequest::save_pre_buffer);
        SetJobConfig(request.mutable_command_params()->mutable_items(), command.job_config_);
        break;
      case RecorderCommand::Type::ADD_SCENARIO:
        request.set_command(eCAL::pb::rec::CommandRequest::add_scenario);
        (*request.mutable_command_params()->mutable_items())["scenario_name"] = command.scenario_name_;
        break;
      case RecorderCommand::Type::EXIT:
        request.set_command(eCAL::pb::rec::CommandRequest::exit);
        break;
      default:
        break;
      }

      return request;
    }

    void RemoteRecorder::SetJobConfig(google::protobuf::Map<std::string, std::string>* job_config_pb, const JobConfig& job_config)
    {
      (*job_config_pb)["meas_root_dir"]     = job_config.GetMeasRootDir();
      (*job_config_pb)["meas_name"]         = job_config.GetMeasName();
      (*job_config_pb)["description"]       = job_config.GetDescription();
      (*job_config_pb)["max_file_size_mib"] = std::to_string(job_config.GetMaxFileSize());
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
  }
}