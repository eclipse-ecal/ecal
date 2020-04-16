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

#include <rec_server_core/rec_server.h>

#include "recorder_settings.h"

#include <ecal/ecal.h>

#include "recorder/remote_recorder.h"
#include "recorder/local_recorder.h"

#include <ecal_utils/string.h>
#include <EcalParser/EcalParser.h>

#include <rec_core/ecal_rec_logger.h>
#include <rec_core/ecal_rec.h>

namespace eCAL
{
  namespace rec
  {
    RecServer::RecServer()
      : client_connections_enabled_(false)
      , connected_to_ecal_    (false)
      , recording_            (false)
      , use_built_in_recorder_(true)
      , ecal_rec_instance_(std::make_shared<eCAL::rec::EcalRec>())
    {
      settings_ = std::make_unique<RecorderSettings>();
      settings_->SetAllToDefaults();
      settings_->ClearHostFilter(); // There is no global host filter
      eCAL::Initialize(0, nullptr, "", eCAL::Init::Default);
    }

    RecServer::~RecServer()
    {
      recorder_instances_.clear();
    }

    ////////////////////////////////////
    // Instance management            //
    ////////////////////////////////////

    bool RecServer::SetRecorderInstances(const std::vector<std::pair<std::string, std::set<std::string>>>& host_hostfilter_list)
    {
      // We cannot add new recorders while a recording is running
      if (recording_)
        return false;

      // Add new instances and update existing ones
      for (const auto& host_hostfilter_pair : host_hostfilter_list)
      {
        // Check if the recorder already exists
        auto existing_rec_it = std::find_if(recorder_instances_.begin(), recorder_instances_.end(),
          [&host_hostfilter_pair](const auto& existing_rec) {return existing_rec.first->GetHostname() == host_hostfilter_pair.first; });

        if (existing_rec_it == recorder_instances_.end())
        {
          // Create new recorder instance
          std::unique_ptr<AbstractRecorder> new_instance;

          RecorderSettings new_instance_settings = *settings_;
          new_instance_settings.SetHostFilter(host_hostfilter_pair.second);

          if ((host_hostfilter_pair.first == eCAL::Process::GetHostName()) && use_built_in_recorder_)
          {
            // Create local recorder instance
            new_instance = std::make_unique<LocalRecorder>(host_hostfilter_pair.first, ecal_rec_instance_, new_instance_settings, connected_to_ecal_);
            EcalRecLogger::Instance()->info("Adding local recorder:  " + host_hostfilter_pair.first);
          }
          else
          {
            // Create remote recorder instance controlled by the RPC interface
            new_instance = std::make_unique<RemoteRecorder>(host_hostfilter_pair.first, new_instance_settings, connected_to_ecal_);
            EcalRecLogger::Instance()->info("Adding remote recorder: " + host_hostfilter_pair.first);
          }

          if (client_connections_enabled_)
          {
            // Connect to the recorder, if all the others have been connected to, already
            new_instance->SetClientConnectionEnabled(true);

            // Connect to ecal
            if (connected_to_ecal_)
            {
              RecorderCommand initialize_command;
              initialize_command.type_ = RecorderCommand::Type::INITIALIZE;
              new_instance->SetCommand(initialize_command);
            }
          }

          recorder_instances_.push_back(std::make_pair(std::move(new_instance), host_hostfilter_pair.second));
        }
        else
        {
          // Update the existing recorder
          if (existing_rec_it->second != host_hostfilter_pair.second)
          {
            existing_rec_it->second = host_hostfilter_pair.second;

            RecorderSettings hostfilter_settings;
            hostfilter_settings.SetHostFilter(host_hostfilter_pair.second);
            existing_rec_it->first->SetSettings(hostfilter_settings);
          }
        }
      }

      // Remove obsolete instances
      {
        std::vector<std::unique_ptr<AbstractRecorder>> removed_recorders;
        auto exsting_rec_it = recorder_instances_.begin();
        while (exsting_rec_it != recorder_instances_.end())
        {
          // Check if the existing recorder is in the list of the set recorders
          auto host_hostfilter_pair_it = std::find_if(host_hostfilter_list.begin(), host_hostfilter_list.end(),
            [exsting_rec_it](const std::pair<std::string, std::set<std::string>>& host_hostfilter_pair) { return exsting_rec_it->first->GetHostname() == host_hostfilter_pair.first; });

          if (host_hostfilter_pair_it == host_hostfilter_list.end())
          {
            // If it is not in that list, remove it!
            EcalRecLogger::Instance()->info("Removing recorder:      " + exsting_rec_it->first->GetHostname());
            removed_recorders.push_back(std::move(exsting_rec_it->first));
            exsting_rec_it = recorder_instances_.erase(exsting_rec_it);
          }
          else
          {
            exsting_rec_it++;
          }
        }


        // Intiate Shutdown
        for (auto& recorder : removed_recorders)
        {
          RecorderCommand disconnect_command;
          disconnect_command.type_ = RecorderCommand::Type::DE_INITIALIZE;
          recorder->InitiateConnectionShutdown(disconnect_command);
        }
        // Wait for recorders to complete the shutdown
        for (auto& recorder : removed_recorders)
        {
          recorder->WaitForPendingRequests();
        }
        // The recorders will now implicitely get destroyed as their pointer goes out of scope
      }

      return true;
    }


    std::vector<std::pair<std::string, std::set<std::string>>> RecServer::GetRecorderInstances() const
    {
      std::vector<std::pair<std::string, std::set<std::string>>> instances;
      instances.reserve(recorder_instances_.size());

      for (const auto& recorder : recorder_instances_)
      {
        instances.emplace_back(std::make_pair(recorder.first->GetHostname(), recorder.second));
      }
      return instances;
    }

    bool RecServer::SetClientConnectionsEnabled(bool enabled)
    {
      if (recording_)
        return false;

      client_connections_enabled_ = enabled;

      for (const auto& recorder : recorder_instances_)
      {
        recorder.first->SetClientConnectionEnabled(enabled);
      }

      return true;
    }

    void RecServer::InitiateConnectionShutdown()
    {
      client_connections_enabled_ = false;

      RecorderCommand disconnect_command;
      disconnect_command.type_ = RecorderCommand::Type::DE_INITIALIZE;

      for (const auto& recorder : recorder_instances_)
      {
        recorder.first->InitiateConnectionShutdown(disconnect_command);
      }

      recording_         = false;
      connected_to_ecal_ = false;
    }

    bool RecServer::AreClientConnectionsEnabled() const
    {
      return client_connections_enabled_;
    }

    std::map<std::string, std::pair<bool, std::string>> RecServer::GetLastResponses()
    {
      std::map<std::string, std::pair<bool, std::string>> response_map;
      for (const auto& recorder : recorder_instances_)
      {
        response_map.emplace(recorder.first->GetHostname(), recorder.first->GetLastResponse());
      }
      return response_map;
    }

    void RecServer::WaitForPendingRequests() const
    {
      for (const auto& recorder : recorder_instances_)
      {
        recorder.first->WaitForPendingRequests();
      }
    }

    std::map<std::string, bool> RecServer::AreRequestsPending() const
    {
      std::map<std::string, bool> pending_map;
      for (const auto& recorder : recorder_instances_)
      {
        pending_map.emplace(recorder.first->GetHostname(), recorder.first->IsRequestPending());
      }
      return pending_map;
    }

    bool RecServer::IsAnyRequestPending() const
    {
      for (const auto& recorder : recorder_instances_)
      {
        if (recorder.first->IsRequestPending()) return true;
      }
      return false;
    }

    ////////////////////////////////////
    // Settings                       //
    ////////////////////////////////////

    bool RecServer::SetMaxPreBufferLength(std::chrono::steady_clock::duration max_pre_buffer_length)
    {
      settings_->SetMaxPreBufferLength(max_pre_buffer_length);

      RecorderSettings diff_settings;
      diff_settings.SetMaxPreBufferLength(max_pre_buffer_length);

      for (const auto& recorder : recorder_instances_)
      {
        recorder.first->SetSettings(diff_settings);
      }

      return true; //Always succeeds
    }

    bool RecServer::SetPreBufferingEnabled(bool pre_buffering_enabled)
    {
      settings_->SetPreBufferingEnabled(pre_buffering_enabled);

      RecorderSettings diff_settings;
      diff_settings.SetPreBufferingEnabled(pre_buffering_enabled);

      for (const auto& recorder : recorder_instances_)
      {
        recorder.first->SetSettings(diff_settings);
      }

      return true; //Always succeeds
    }

    bool RecServer::SetRecordMode(RecordMode record_mode)
    {
      if (recording_) return false;

      settings_->SetRecordMode(record_mode);

      RecorderSettings diff_settings;
      diff_settings.SetRecordMode(record_mode);

      for (const auto& recorder : recorder_instances_)
      {
        recorder.first->SetSettings(diff_settings);
      }

      return true;
    }

    bool RecServer::SetRecordMode(RecordMode record_mode, const std::set<std::string>& topic_list)
    {
      if (recording_) return false;

      settings_->SetRecordMode(record_mode);
      settings_->SetListedTopics(topic_list);

      RecorderSettings diff_settings;
      diff_settings.SetRecordMode(record_mode);
      diff_settings.SetListedTopics(topic_list);

      for (const auto& recorder : recorder_instances_)
      {
        recorder.first->SetSettings(diff_settings);
      }

      return true;
    }

    bool RecServer::SetListedTopics(std::set<std::string> listed_topics)
    {
      if (recording_) return false;

      settings_->SetListedTopics(listed_topics);

      RecorderSettings diff_settings;
      diff_settings.SetListedTopics(listed_topics);

      for (const auto& recorder : recorder_instances_)
      {
        recorder.first->SetSettings(diff_settings);
      }

      return true;
    }

    bool RecServer::SetHostFilter(const std::string& hostname, const std::set<std::string>& host_filter)
    {
      // We cannot set the host filter while a recording is running
      if (recording_)
        return false;

      // Lookup the correct recorder
      for (auto& recorder : recorder_instances_)
      {
        if (recorder.first->GetHostname() == hostname)
        {
          if (recorder.second != host_filter)
          {
            recorder.second = host_filter;

            // Prepare settings containing the host filter
            RecorderSettings settings;
            settings.SetHostFilter(host_filter);

            // Send host filter to the recorder
            recorder.first->SetSettings(settings);
          }

          return true;
        }
      }

      // Recorder not found
      return false;
    }


    std::chrono::steady_clock::duration RecServer::GetMaxPreBufferLength() const
    {
      return settings_->GetMaxPreBufferLength();
    }

    bool RecServer::GetPreBufferingEnabled() const
    {
      return settings_->GetPreBufferingEnabled();
    }

    RecordMode RecServer::GetRecordMode() const
    {
      return settings_->GetRecordMode();
    }

    std::set<std::string> RecServer::GetListedTopics() const
    {
      return settings_->GetListedTopics();
    }

    std::set<std::string> RecServer::GetHostFilter(const std::string& hostname) const
    {
      for (auto& recorder : recorder_instances_)
      {
        if (recorder.first->GetHostname() == hostname)
          return recorder.second;
      }
      return {};
    }


    ////////////////////////////////////
    // Shared job config              //
    ////////////////////////////////////

    void RecServer::SetMeasRootDir(std::string meas_root_dir)
    {
      job_config_.SetMeasRootDir(meas_root_dir);
    }

    void RecServer::SetMeasName(std::string meas_name)
    {
      job_config_.SetMeasName(meas_name);
    }

    void RecServer::SetMaxFileSizeMib(size_t max_file_size_mib)
    {
      job_config_.SetMaxFileSize(max_file_size_mib);
    }

    void RecServer::SetDescription(std::string description)
    {
      job_config_.SetDescription(description);
    }


    std::string RecServer::GetMeasRootDir() const
    {
      return job_config_.GetMeasRootDir();
    }

    std::string RecServer::GetMeasName() const
    {
      return job_config_.GetMeasName();
    }

    size_t RecServer::GetMaxFileSizeMib() const
    {
      return job_config_.GetMaxFileSize();
    }

    std::string RecServer::GetDescription() const
    {
      return job_config_.GetDescription();
    }

    JobConfig RecServer::GetHostEvaluatedJobConfig() const
    {
      // Copy input config
      JobConfig evaluated_config = job_config_;

      // Save current time, so all elements will be evaluated with the same time
      auto now = std::chrono::system_clock::now();

      // Evaluate elements
      evaluated_config.SetMeasRootDir(EcalParser::Evaluate(evaluated_config.GetMeasRootDir(), false, now));
      evaluated_config.SetMeasName   (EcalParser::Evaluate(evaluated_config.GetMeasName(),    false, now));
      evaluated_config.SetDescription(EcalParser::Evaluate(evaluated_config.GetDescription(), false, now));

      return evaluated_config;
    }


    ////////////////////////////////////
    // Commands                       //
    ////////////////////////////////////

    bool RecServer::SendRequestConnectToEcal()
    {
      // Connect to recorder Clients
      if (!client_connections_enabled_)
      {
        for (const auto& recorder : recorder_instances_)
        {
          recorder.first->SetClientConnectionEnabled(true);
        }
        client_connections_enabled_ = true;
      }

      // Send Intialize command
      for (const auto& recorder : recorder_instances_)
      {
        RecorderCommand command;
        command.type_ = RecorderCommand::Type::INITIALIZE;

        recorder.first->SetCommand(command);
      }

      connected_to_ecal_           = true;

      return true;
    }

    bool RecServer::SendRequestDisconnectFromEcal()
    {
      if (recording_ || !client_connections_enabled_)
        return false;

      for (const auto& recorder : recorder_instances_)
      {
        RecorderCommand command;
        command.type_ = RecorderCommand::Type::DE_INITIALIZE;

        recorder.first->SetCommand(command);
      }

      connected_to_ecal_ = false;

      return true;
    }

    bool RecServer::SendRequestSavePreBufferedData()
    {
      if (!client_connections_enabled_ || !connected_to_ecal_ || !settings_->GetPreBufferingEnabled())
        return false;

      JobConfig evaluated_config = GetHostEvaluatedJobConfig();

      for (const auto& recorder : recorder_instances_)
      {
        RecorderCommand command;
        command.type_       = RecorderCommand::Type::SAVE_PRE_BUFFER;
        command.job_config_ = evaluated_config;

        recorder.first->SetCommand(command);
      }

      return true;
    }


    bool RecServer::SendRequestStartRecording()
    {

      host_evaluated_config_ = GetHostEvaluatedJobConfig();

      // Enable connection to clients
      if (!client_connections_enabled_)
      {
        for (const auto& recorder : recorder_instances_)
        {
          recorder.first->SetClientConnectionEnabled(true);
        }
      }
      client_connections_enabled_ = true;

      // Send record command
      for (const auto& recorder : recorder_instances_)
      {
        //std::vector<RecorderCommand> commands;

        RecorderCommand recording_command;
        recording_command.type_       = RecorderCommand::Type::START_RECORDING;
        recording_command.job_config_ = host_evaluated_config_;

        recorder.first->SetCommand(recording_command);
      }

      connected_to_ecal_ = true;
      recording_         = true;

      return true;
    }

    bool RecServer::SendRequestStopRecording()
    {
      for (const auto& recorder : recorder_instances_)
      {
        RecorderCommand command;
        command.type_ = RecorderCommand::Type::STOP_RECORDING;

        recorder.first->SetCommand(command);
      }
      recording_ = false;

      return true;
    }

    ////////////////////////////////////
    // State                          //
    ////////////////////////////////////

    bool RecServer::RecordersConnectedToEcal() const
    {
      return connected_to_ecal_;
    }

    bool RecServer::RecordersRecording() const
    {
      return recording_;
    }

    std::map<std::string, RecorderState> RecServer::GetRecorderStates() const
    {
      std::map<std::string, RecorderState> recorder_states;
      for (const auto& recorder_instance : recorder_instances_)
      {
        if (recorder_instance.first->IsAlive())
        {
          recorder_states.emplace(recorder_instance.first->GetHostname(), recorder_instance.first->GetState());
        }
      }
      return recorder_states;
    }

    bool RecServer::IsLocalRecorderInstanceBusy() const
    {
      auto local_recorder_state = ecal_rec_instance_->GetRecorderState();

      bool recorder_busy = (local_recorder_state.main_recorder_state_.recording_);
      if (!recorder_busy)
      {
        for (const auto& buffer_writer_state : local_recorder_state.buffer_writers_)
        {
          if (buffer_writer_state.recording_)
          {
            recorder_busy = true;
            continue;
          }
        }
      }
      return recorder_busy;
    }

    RecorderState RecServer::GetLocalRecorderInstanceState() const
    {
      return ecal_rec_instance_->GetRecorderState();
    }

    ////////////////////////////////////
    // Options                        //
    ////////////////////////////////////
    void RecServer::SetUsingBuiltInRecorderEnabled(bool enabled)
    {
      use_built_in_recorder_ = enabled;
    }

    bool RecServer::IsUsingBuiltInRecorderEnabled() const
    {
      return use_built_in_recorder_;
    }
  }
}