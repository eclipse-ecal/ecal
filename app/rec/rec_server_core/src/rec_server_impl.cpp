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

#include "rec_server_impl.h"

#include "recorder_settings.h"

#include <ecal/ecal.h>

#include "monitoring_thread.h"
#include "recorder/remote_recorder.h"
#include "recorder/local_recorder.h"

#include "config/config.h"

#include <ecal_utils/string.h>
#include <ecal_utils/filesystem.h>
#include <EcalParser/EcalParser.h>

#include <rec_client_core/ecal_rec_logger.h>
#include <rec_client_core/ecal_rec.h>

namespace eCAL
{
  namespace rec_server
  {
    ////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////

    RecServerImpl::RecServerImpl()
      : monitoring_thread_          (std::make_unique<MonitoringThread>([this]() { return GetRunningEnabledRecClients(); }))
      , client_connections_active_  (false)
      , connected_to_ecal_          (false)
      , recording_                  (false)
      , currently_recording_meas_id_(0)
      , use_built_in_recorder_      (true)
      , ecal_rec_instance_          (std::make_shared<eCAL::rec::EcalRec>())
      , ftp_server_                 (std::make_unique<fineftp::FtpServer>(static_cast<uint16_t>(0)))
      , loaded_config_version_      (-1)
    {
      // Initialize Recorder Settings
      settings_.SetAllToDefaults();
      settings_.ClearHostFilter(); // There is no global host filter

      // Initialize eCAL
      eCAL::Initialize("eCALRec-Server", eCAL::Init::Default | eCAL::Init::Monitoring);

      // Start FTP Server
      ftp_server_->start(5);

      // Start Monitoring Thread
      SetMonitoringUpdateCallback([](const TopicInfoMap_T&, const HostsRunningEcalRec_T&) -> void { return; });
      monitoring_thread_->Start();
    }

    RecServerImpl::~RecServerImpl()
    {
      ftp_server_->stop();

      monitoring_thread_->Interrupt();
      monitoring_thread_->Join();

      connected_rec_clients_.clear();
    }

    ////////////////////////////////////
    // Client management
    ////////////////////////////////////

    bool RecServerImpl::SetEnabledRecClients(const std::map<std::string, ClientConfig>& enabled_rec_clients)
    {
      // We cannot add / remove recorders or change the host filter while a recording is running
      if (recording_)
        return false;

      std::unique_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

      // CONNECT to new recorders and ENABLE and UPDATE those which we are already connected to 

      for (const auto& enabled_client_pair : enabled_rec_clients)
      {
        // Check if the recorder already exists
        auto existing_enabled_client_it = enabled_rec_clients_.find(enabled_client_pair.first);
        if (existing_enabled_client_it == enabled_rec_clients_.end())
        {
          // The recorder was NOT ENABLED, before

          // Check if we have already connected to that recorder
          auto connected_rec_it = connected_rec_clients_.find(enabled_client_pair.first);
          if (connected_rec_it == connected_rec_clients_.end())
          {
            // We are not connected. So we create a new connection. This
            // connection will probably fail, because if there was a client with
            // that name it should have shown up in the monitoring and we would
            // already have connected to it.

            RecorderSettings settings;
            settings.SetEnabledAddons(enabled_client_pair.second.enabled_addons_);
            settings.SetHostFilter   (enabled_client_pair.second.host_filter_);

            auto new_rec = CreateNewRecorder(enabled_client_pair.first, settings);
            if (client_connections_active_)
              new_rec->SetRecorderEnabled(true, connected_to_ecal_);

            connected_rec_clients_.emplace(enabled_client_pair.first, std::move(new_rec));
          }
          else
          {
            // The connection has already been established. We only need to enable it.

            RecorderSettings settings;
            settings.SetEnabledAddons(enabled_client_pair.second.enabled_addons_);
            settings.SetHostFilter   (enabled_client_pair.second.host_filter_);

            connected_rec_it->second->SetSettings(settings);
            if (client_connections_active_)
              connected_rec_it->second->SetRecorderEnabled(true, connected_to_ecal_);
          }
        }
        else
        {
          // The recorder was ENABLED before. We may only need to set some settings (if at all).

          RecorderSettings new_settings;

          // Enabled Addons changed?
          if (existing_enabled_client_it->second.enabled_addons_ != enabled_client_pair.second.enabled_addons_)
          {
            new_settings.SetEnabledAddons(enabled_client_pair.second.enabled_addons_);
          }

          // Host Filter Changed?
          if (existing_enabled_client_it->second.host_filter_ != enabled_client_pair.second.host_filter_)
          {
            new_settings.SetHostFilter(enabled_client_pair.second.host_filter_);
          }

          // Set Settings if anything has changed
          if (new_settings.IsHostFilterSet() || new_settings.IsEnabledAddonsSet())
          {
            auto recorder_instance_it = connected_rec_clients_.find(existing_enabled_client_it->first);
            recorder_instance_it->second->SetSettings(new_settings);
          }
        }
      }

      // DISABLE recorders that are not enabled any more
      {
        for (const auto& connected_client : connected_rec_clients_)
        {
          if (connected_client.second->IsRecorderEnabled()
            && (enabled_rec_clients.find(connected_client.first) == enabled_rec_clients.end()))
          {
            connected_client.second->SetRecorderEnabled(false);
            eCAL::rec::EcalRecLogger::Instance()->info("Removing recorder:        " + connected_client.first);
          }
        }
      }

      enabled_rec_clients_ = enabled_rec_clients;
      return true;
    }

    std::map<std::string, ClientConfig> RecServerImpl::GetEnabledRecClients() const
    {
      std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);
      return enabled_rec_clients_;
    }

    bool RecServerImpl::SetHostFilter(const std::string& hostname, const std::set<std::string>& host_filter)
    {
      // TODO: add addon variant of thei method

      // We cannot set the host filter while a recording is running
      if (recording_)
        return false;

      std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

      // Lookup the correct recorder
      auto recorder_instance_it = connected_rec_clients_.find(hostname);

      if (recorder_instance_it == connected_rec_clients_.end())
      {
        // Recorder not found
        return false;
      }

      if (enabled_rec_clients_.at(hostname).host_filter_ != host_filter)
      {
        enabled_rec_clients_.at(hostname).host_filter_ = host_filter;

        // Prepare settings containing the host filter
        RecorderSettings settings;
        settings.SetHostFilter(host_filter);

        // Send host filter to the recorder
        recorder_instance_it->second->SetSettings(settings);
      }

      return true;
    }

    std::set<std::string> RecServerImpl::GetHostFilter(const std::string& hostname) const
    {
      // TODO: add addon variant of this method
      std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

      auto enabled_rec_it = enabled_rec_clients_.find(hostname);
      if (enabled_rec_it != enabled_rec_clients_.end())
      {
        return enabled_rec_it->second.host_filter_;
      }
      else
      {
        return{};
      }
    }

    bool RecServerImpl::SetConnectionToClientsActive(bool active)
    {
      if (recording_)
        return false;
      
      std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

      SetConnectionToClientsActive_NoLock(active);

      return true;
    }

    bool RecServerImpl::IsConnectionToClientsActive() const
    {
      return client_connections_active_;
    }

    void RecServerImpl::UpdateRecorderConnections(const HostsRunningEcalRec_T& hosts_running_ecal_rec)
    {
      std::vector<std::unique_ptr<AbstractRecorder>> connections_to_destroy; // The destructor of the rec connections may be expensive, so we copy the connections to this list and destroy the list without having the mutex locked

      {
        std::unique_lock<decltype (connected_enabled_rec_clients_mutex_)> rec_clients_lock(connected_enabled_rec_clients_mutex_);

        // Connect to new recorders that are visible now in the eCAL monitoring
        for (const auto& host : hosts_running_ecal_rec)
        {
          if (host.second)
          {
            // The host has a running eCAL rec instance

            // Check if the host is already connected
            if ((host.first != eCAL::Process::GetHostName()) || (!use_built_in_recorder_))
            {
              auto connected_client_it = connected_rec_clients_.find(host.first);
              if (connected_client_it == connected_rec_clients_.end())
              {
                // The client is not connected. So we need to connect to it and start polling its status.
                connected_rec_clients_.emplace(host.first, CreateNewRecorder(host.first));
              }
            }
          }
        }

        // Cut the connection to dead recorders that the user didn't enable anyways
        {
          auto connected_client_it = connected_rec_clients_.begin();
          while (connected_client_it != connected_rec_clients_.end())
          {
            // Only kill dead connections
            if (!connected_client_it->second->IsAlive())
            {
              // Only kill connections to hosts that are not visible any more or don't have an ecal rec running
              auto hosts_running_ecal_rec_it = hosts_running_ecal_rec.find(connected_client_it->first);
              if (hosts_running_ecal_rec_it == hosts_running_ecal_rec.end() || !hosts_running_ecal_rec_it->second)
              {
                // Only kill connections that are not enabled by the user
                if (enabled_rec_clients_.find(connected_client_it->first) == enabled_rec_clients_.end())
                {
                  // Check if the connection has participated in a measurement.
                  // If it has, we keep it, as the connection object is responsible
                  // for holding error messages when the user e.g. tries to upload
                  // the measurement.
                  if (!connected_client_it->second->EverParticipatedInAMeasurement())
                  {
                    connections_to_destroy.push_back(std::move(connected_client_it->second));
                    connected_client_it = connected_rec_clients_.erase(connected_client_it);
                    continue;
                  }
                }
              }
            }
            connected_client_it++;
          }
        }
      }

      connections_to_destroy.clear();
    }

    std::unique_ptr<AbstractRecorder> RecServerImpl::CreateNewRecorder(const std::string& hostname, const RecorderSettings& initial_settings)
    {
      // Create new recorder instance
      std::unique_ptr<AbstractRecorder> new_instance;

      RecorderSettings new_instance_settings(settings_);
      new_instance_settings.AddSettings(initial_settings);

      if ((hostname == eCAL::Process::GetHostName()) && use_built_in_recorder_)
      {
        // Create local recorder instance
        new_instance = std::make_unique<LocalRecorder>(hostname
                                                     , ecal_rec_instance_
                                                     , std::bind(&RecServerImpl::UpdateJobstatusCallback, this, std::placeholders::_1, std::placeholders::_2)
                                                     , std::bind(&RecServerImpl::ReportJobCommandResponseCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
                                                     , new_instance_settings);
        eCAL::rec::EcalRecLogger::Instance()->info("Creating local recorder:  " + hostname);
      }
      else
      {
        // Create remote recorder instance controlled by the RPC interface
        new_instance = std::make_unique<RemoteRecorder>(hostname
                                                      , std::bind(&RecServerImpl::UpdateJobstatusCallback, this, std::placeholders::_1, std::placeholders::_2)
                                                      , std::bind(&RecServerImpl::ReportJobCommandResponseCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
                                                      , new_instance_settings);
        eCAL::rec::EcalRecLogger::Instance()->info("Creating remote recorder: " + hostname);
      }

      return new_instance;
    }

    bool RecServerImpl::IsRecClientEnabled(const std::string& hostname) const
    {
      std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

      return enabled_rec_clients_.find(hostname) != enabled_rec_clients_.end();
    }

    void RecServerImpl::SetConnectionToClientsActive_NoLock(bool active)
    {
      // ACTIVATE connections
      if (!client_connections_active_ && active)
      {
        for (const auto& rec_client : connected_rec_clients_)
        {
          bool is_enabled = (enabled_rec_clients_.find(rec_client.first) != enabled_rec_clients_.end());
          if (is_enabled)
            rec_client.second->SetRecorderEnabled(true, connected_to_ecal_);
        }

        client_connections_active_ = true;
      }

      // DEACTIVATE connections
      else if (client_connections_active_ && !active)
      {
        for (const auto& rec_client : connected_rec_clients_)
        {
          if (rec_client.second->IsRecorderEnabled())
            rec_client.second->SetRecorderEnabled(false);
        }

        client_connections_active_ = false;
        connected_to_ecal_         = false;
      }
    }

    ////////////////////////////////////
    // Recorder control
    ////////////////////////////////////

    bool RecServerImpl::ConnectToEcal()
    {
      std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

      if (!client_connections_active_)
      {
        // The SetConnectionToClientsActive_NoLock will also take care of initializing the recorders
        connected_to_ecal_ = true;
        SetConnectionToClientsActive_NoLock(true);
      }
      else
      {
        // Send Intialize command
        for (const auto& recorder : connected_rec_clients_)
        {
          RecorderCommand command;
          command.type_ = RecorderCommand::Type::INITIALIZE;

          recorder.second->SetCommand(command);
        }

        connected_to_ecal_ = true;
      }

      return true;
    }

    bool RecServerImpl::DisconnectFromEcal()
    {
      if (recording_)
        return false;

      std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

      for (const auto& recorder : connected_rec_clients_)
      {
        RecorderCommand command;
        command.type_ = RecorderCommand::Type::DE_INITIALIZE;

        recorder.second->SetCommand(command);
      }

      connected_to_ecal_ = false;

      return true;
    }

    bool RecServerImpl::SavePreBufferedData()
    {
      if (!connected_to_ecal_ || !settings_.GetPreBufferingEnabled())
        return false;

      auto now = std::chrono::system_clock::now();
      eCAL::rec::JobConfig host_evaluated_config = job_config_.CreateEvaluatedJobConfig(false, now);
      host_evaluated_config.GenerateNewJobId();

      // First add the job history entry, so once the job is started, the client connections can immediatelly modify the states
      JobHistoryEntry job_history_entry;
      {
        std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);
        job_history_entry = initializeJobHistoryEntry_NoLock(now, host_evaluated_config.CreateEvaluatedJobConfig(true, now));
      }
      {
        std::unique_lock<decltype (job_history_mutex_)>job_history_lock(job_history_mutex_);
        job_history_.push_back(std::move(job_history_entry));
      }

      // Create the commands for local and remote recorders
      // 
      // The local recorder gets a special role here: It will get a
      // pre-evaluated version of the job config. That is necessary, as we
      // need to make sure we know _exactly_ where it is recording to,
      // because we need that directory later for merging the measurement. The
      // alternative would be to let the local recorder to evaluate it itself,
      // but when we are not using the built-in recorder, the recording path
      // may differ due to other environment variables or a different
      // timestamp. In that case we would have to force the local recorder to
      // also upload its measurement via FTP. That however would usually lead
      // to the local recorder upload the measurement to its own folder, which
      // would lead to duplicate files.
      RecorderCommand remote_save_buffer_command;
      remote_save_buffer_command.type_       = RecorderCommand::Type::SAVE_PRE_BUFFER;
      remote_save_buffer_command.job_config_ = host_evaluated_config;

      RecorderCommand local_save_buffer_command;
      local_save_buffer_command.type_       = RecorderCommand::Type::SAVE_PRE_BUFFER;
      local_save_buffer_command.job_config_ = host_evaluated_config.CreateEvaluatedJobConfig(true, now);

      // Then start the job
      {
        std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);
        for (const auto& recorder : connected_rec_clients_)
        {
          if (recorder.first == eCAL::Process::GetHostName())
            recorder.second->SetCommand(local_save_buffer_command);
          else
            recorder.second->SetCommand(remote_save_buffer_command);
        }
      }

      return true;
    }

    bool RecServerImpl::StartRecording()
    {
      if (recording_)
        return false;

      auto now = std::chrono::system_clock::now();
      eCAL::rec::JobConfig host_evaluated_config = job_config_.CreateEvaluatedJobConfig(false);
      host_evaluated_config.GenerateNewJobId();

      // First add the job history entry, so once the job is started, the client connections can immediatelly modify the states
      JobHistoryEntry job_history_entry;
      {
        std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);
        job_history_entry = initializeJobHistoryEntry_NoLock(now, host_evaluated_config.CreateEvaluatedJobConfig(true, now));
      }
      {
        std::unique_lock<decltype (job_history_mutex_)>job_history_lock(job_history_mutex_);
        job_history_.push_back(std::move(job_history_entry));
      }

      // Then start the job
      {
        std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

        // Enable connection to clients 
        SetConnectionToClientsActive_NoLock(true);

        // Create the commands for local and remote recorders
        // 
        // The local recorder gets a special role here: It will get a
        // pre-evaluated version of the job config. That is necessary, as we
        // need to make sure we know _exactly_ where it is recording to,
        // because we need that directory later for merging the measurement. The
        // alternative would be to let the local recorder to evaluate it itself,
        // but when we are not using the built-in recorder, the recording path
        // may differ due to other environment variables or a different
        // timestamp. In that case we would have to force the local recorder to
        // also upload its measurement via FTP. That however would usually lead
        // to the local recorder upload the measurement to its own folder, which
        // would lead to duplicate files.
        RecorderCommand remote_recording_command;
        remote_recording_command.type_       = RecorderCommand::Type::START_RECORDING;
        remote_recording_command.job_config_ = host_evaluated_config;

        RecorderCommand local_recording_command;
        local_recording_command.type_       = RecorderCommand::Type::START_RECORDING;
        local_recording_command.job_config_ = host_evaluated_config.CreateEvaluatedJobConfig(true, now);

        for (const auto& recorder : connected_rec_clients_)
        {
          if (recorder.first == eCAL::Process::GetHostName())
            recorder.second->SetCommand(local_recording_command);
          else
            recorder.second->SetCommand(remote_recording_command);
        }
      }

      connected_to_ecal_           = true;
      recording_                   = true;
      currently_recording_meas_id_ = host_evaluated_config.GetJobId();

      return true;
    }

    bool RecServerImpl::StopRecording()
    {
      std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

      for (const auto& recorder : connected_rec_clients_)
      {
        RecorderCommand command;
        command.type_ = RecorderCommand::Type::STOP_RECORDING;

        recorder.second->SetCommand(command);
      }
      recording_                   = false;
      currently_recording_meas_id_ = 0;

      return true;
    }

    bool RecServerImpl::IsConnectedToEcal() const
    {
      return connected_to_ecal_;
    }

    bool RecServerImpl::IsRecording() const
    {
      return recording_;
    }

    int64_t RecServerImpl::GetCurrentlyRecordingMeasId() const
    {
      return currently_recording_meas_id_;
    }

    bool RecServerImpl::IsAnyRequestPending() const
    {
      std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

      for (const auto& recorder : connected_rec_clients_)
      {
        if (recorder.second->IsRequestPending()) return true;
      }
      return false;
    }

    std::set<std::string> RecServerImpl::GetHostsWithPendingRequests() const
    {
      std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

      std::set<std::string> pending_set;
      for (const auto& recorder : connected_rec_clients_)
      {
        if (recorder.second->IsRequestPending())
          pending_set.emplace(recorder.first);
      }
      return pending_set;
    }

    void RecServerImpl::WaitForPendingRequests() const
    {
      std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

      for (const auto& recorder : connected_rec_clients_)
      {
        recorder.second->WaitForPendingRequests();
      }
    }

    JobHistoryEntry RecServerImpl::initializeJobHistoryEntry_NoLock(std::chrono::system_clock::time_point now, const eCAL::rec::JobConfig& local_evaluated_job_config)
    {
      JobHistoryEntry job_history_entry;
      job_history_entry.local_start_time_           = now;
      job_history_entry.local_evaluated_job_config_ = local_evaluated_job_config;

      for (const auto& enabled_rec_client : enabled_rec_clients_)
      {

        eCAL::rec_server::ClientJobStatus client_job_status_;
        client_job_status_.job_status_.job_id_ = job_history_entry.local_evaluated_job_config_.GetJobId();

        for (const auto& addon_id : enabled_rec_client.second.enabled_addons_)
        {
          client_job_status_.job_status_.rec_addon_statuses_.emplace(addon_id, eCAL::rec::RecAddonJobStatus());
        }

        job_history_entry.client_statuses_.emplace(enabled_rec_client.first, std::move(client_job_status_));
      }
      return job_history_entry;
    }

    ////////////////////////////////////
    // Status
    ////////////////////////////////////

    eCAL::rec_server::RecorderStatusMap_T RecServerImpl::GetRecorderStatuses() const
    {
      std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

      eCAL::rec_server::RecorderStatusMap_T recorder_statuses;
      for (const auto& recorder_instance : connected_rec_clients_)
      {
        if (recorder_instance.second->IsAlive())
        {
          recorder_statuses.emplace(recorder_instance.first, recorder_instance.second->GetStatus());
        }
      }
      return recorder_statuses;
    }

    eCAL::rec::RecorderStatus RecServerImpl::GetBuiltInRecorderInstanceStatus() const
    {
      return ecal_rec_instance_->GetRecorderStatus();
    }

    TopicInfoMap_T RecServerImpl::GetTopicInfo() const
    {
      return monitoring_thread_->GetTopicInfoMap();
    }

    HostsRunningEcalRec_T RecServerImpl::GetHostsRunningEcalRec() const
    {
      return monitoring_thread_->GetHostsRunningEcalRec();
    }

    std::list<eCAL::rec_server::JobHistoryEntry> RecServerImpl::GetJobHistory() const
    {
      std::shared_lock<decltype(job_history_mutex_)> job_history_lock(job_history_mutex_);
      return job_history_;
    }

    RecServerStatus RecServerImpl::GetStatus() const
    {
      RecServerStatus status;

      status.config_path_       = loaded_config_path_;
      status.activated_         = connected_to_ecal_;
      status.recording_meas_id_ = currently_recording_meas_id_;
      
      {
        std::shared_lock<decltype(job_history_mutex_)> job_history_lock(job_history_mutex_);
        status.job_history_ = job_history_;
        for (auto& job : status.job_history_)
        {
          auto result = SimulateUploadMeasurement_NoLock(job);
          if (result == eCAL::rec::Error(eCAL::rec::Error::OK))
            job.can_upload_  = true;
          else
            job.can_upload_  = false;
          result = SimulateAddComment_NoLock(job, static_cast<int>(job.local_evaluated_job_config_.GetJobId()));
          if (result == eCAL::rec::Error(eCAL::rec::Error::OK))
            job.can_comment_ = true;
          else
            job.can_comment_ = false;
        }
      }

      {
        std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

        for (const auto& recorder_instance : connected_rec_clients_)
        {
          bool is_enabled = (enabled_rec_clients_.find(recorder_instance.first) != enabled_rec_clients_.end());
          if (is_enabled && recorder_instance.second->IsAlive())
          {
            status.client_statuses_.emplace(recorder_instance.first, recorder_instance.second->GetStatus());
          }
        }
      }

      return status;
    }

    // TODO: This function could be a general "reportRecorderStatus" callback
    void RecServerImpl::UpdateJobstatusCallback(const std::string& hostname, const eCAL::rec::RecorderStatus& recorder_status)
    {
      // Update Job history
      {
        std::unique_lock<decltype(job_history_mutex_)> job_history_lock(job_history_mutex_);

        for (const auto& job_status : recorder_status.job_statuses_)
        {
          auto job_history_it = std::find_if(job_history_.begin(), job_history_.end(),
            [&job_status](const eCAL::rec_server::JobHistoryEntry& entry) -> bool { return entry.local_evaluated_job_config_.GetJobId() == job_status.job_id_; });
          if (job_history_it == job_history_.end()) continue;

          auto client_status_it = job_history_it->client_statuses_.find(hostname);
          if (client_status_it == job_history_it->client_statuses_.end()) continue;

          // Update PID
          client_status_it->second.client_pid_ = recorder_status.pid_;

          // Update State 
          client_status_it->second.job_status_.state_ = job_status.state_;

          // Update upload status
          client_status_it->second.job_status_.upload_status_ = job_status.upload_status_;

          // Update is_deleted
          client_status_it->second.job_status_.is_deleted_ = job_status.is_deleted_;

          // Update HDF5 rec status
          client_status_it->second.job_status_.rec_hdf5_status_ = job_status.rec_hdf5_status_;

          // Update Addon statuses that are still running
          for (const auto& addon_status : job_status.rec_addon_statuses_)
          {
            client_status_it->second.job_status_.rec_addon_statuses_[addon_status.first] = addon_status.second;
          }

          // Update Addons that are not running
          for (auto& existing_addon_status : client_status_it->second.job_status_.rec_addon_statuses_)
          {
            // If the addon has not finished flushing and now doesn't report it's state, the measurement may corrupt!
            if (existing_addon_status.second.state_ != eCAL::rec::RecAddonJobStatus::State::FinishedFlushing)
            {
              auto addon_job_status_it = job_status.rec_addon_statuses_.find(existing_addon_status.first);
              auto addon_status_it = std::find_if(recorder_status.addon_statuses_.begin(), recorder_status.addon_statuses_.end(),
                                                  [&existing_addon_status](const eCAL::rec::RecorderAddonStatus& addon_status)
                                                  {
                                                    return addon_status.addon_id_ == existing_addon_status.first;
                                                  });
              if ((addon_job_status_it == job_status.rec_addon_statuses_.end())
                || (addon_status_it == recorder_status.addon_statuses_.end()))
              {
                existing_addon_status.second.info_ = { false, "The addon has failed" };
              }
            }
          }
        }
      }
    }
    
    void RecServerImpl::ReportJobCommandResponseCallback(int64_t job_id, const std::string& hostname, const std::pair<bool, std::string>& response_info)
    {
      std::unique_lock<decltype(job_history_mutex_)> job_history_lock(job_history_mutex_);

      auto job_history_it = std::find_if(job_history_.begin(), job_history_.end(),
        [job_id](const eCAL::rec_server::JobHistoryEntry& entry) -> bool { return entry.local_evaluated_job_config_.GetJobId() == job_id; });
      if (job_history_it == job_history_.end()) return;

      auto client_status_it = job_history_it->client_statuses_.find(hostname);
      if (client_status_it == job_history_it->client_statuses_.end()) return;

      client_status_it->second.info_last_command_response_ = response_info;
    }

    std::map<std::string, int32_t> RecServerImpl::GetRunningEnabledRecClients() const
    {
      std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

      std::map<std::string, int32_t> running_enabled_rec_clients;
      for (const auto& enabled_rec_client : enabled_rec_clients_)
      {
        auto connected_rec_client_it = connected_rec_clients_.find(enabled_rec_client.first);
        if (connected_rec_client_it != connected_rec_clients_.end())
        {
          int32_t process_id = connected_rec_client_it->second->GetStatus().first.pid_;
          if (process_id >= 0)
          {
            running_enabled_rec_clients[enabled_rec_client.first] = process_id;
          }
        }
      }

      return running_enabled_rec_clients;
    }

    ////////////////////////////////////
    // General Client Settings
    ////////////////////////////////////

    void RecServerImpl::SetMaxPreBufferLength(std::chrono::steady_clock::duration max_pre_buffer_length)
    {
      settings_.SetMaxPreBufferLength(max_pre_buffer_length);

      RecorderSettings diff_settings;
      diff_settings.SetMaxPreBufferLength(max_pre_buffer_length);

      std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

      for (const auto& recorder : connected_rec_clients_)
      {
        recorder.second->SetSettings(diff_settings);
      }
    }

    void RecServerImpl::SetPreBufferingEnabled(bool pre_buffering_enabled)
    {
      settings_.SetPreBufferingEnabled(pre_buffering_enabled);

      RecorderSettings diff_settings;
      diff_settings.SetPreBufferingEnabled(pre_buffering_enabled);

      std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

      for (const auto& recorder : connected_rec_clients_)
      {
        recorder.second->SetSettings(diff_settings);
      }
    }

    bool RecServerImpl::SetRecordMode(eCAL::rec::RecordMode record_mode)
    {
      if (recording_) return false;

      settings_.SetRecordMode(record_mode);

      RecorderSettings diff_settings;
      diff_settings.SetRecordMode(record_mode);

      std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

      for (const auto& recorder : connected_rec_clients_)
      {
        recorder.second->SetSettings(diff_settings);
      }

      return true;
    }

    bool RecServerImpl::SetRecordMode(eCAL::rec::RecordMode record_mode, const std::set<std::string>& topic_list)
    {
      if (recording_) return false;

      settings_.SetRecordMode  (record_mode);
      settings_.SetListedTopics(topic_list);

      RecorderSettings diff_settings;
      diff_settings.SetRecordMode  (record_mode);
      diff_settings.SetListedTopics(topic_list);

      std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

      for (const auto& recorder : connected_rec_clients_)
      {
        recorder.second->SetSettings(diff_settings);
      }

      return true;
    }

    bool RecServerImpl::SetListedTopics(std::set<std::string> listed_topics)
    {
      if (recording_) return false;

      settings_.SetListedTopics(listed_topics);

      RecorderSettings diff_settings;
      diff_settings.SetListedTopics(listed_topics);

      std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

      for (const auto& recorder : connected_rec_clients_)
      {
        recorder.second->SetSettings(diff_settings);
      }

      return true;
    }


    std::chrono::steady_clock::duration RecServerImpl::GetMaxPreBufferLength() const
    {
      return settings_.GetMaxPreBufferLength();
    }

    bool RecServerImpl::GetPreBufferingEnabled() const
    {
      return settings_.GetPreBufferingEnabled();
    }

    eCAL::rec::RecordMode RecServerImpl::GetRecordMode() const
    {
      return settings_.GetRecordMode();
    }

    std::set<std::string> RecServerImpl::GetListedTopics() const
    {
      return settings_.GetListedTopics();
    }

    ////////////////////////////////////
    // Job Settings
    ////////////////////////////////////

    void RecServerImpl::SetMeasRootDir(const std::string& meas_root_dir)
    {
      job_config_.SetMeasRootDir(meas_root_dir);
    }

    void RecServerImpl::SetMeasName(const std::string& meas_name)
    {
      job_config_.SetMeasName(meas_name);
    }

    void RecServerImpl::SetMaxFileSizeMib(int64_t max_file_size_mib)
    {
      job_config_.SetMaxFileSize(max_file_size_mib);
    }

    void RecServerImpl::SetOneFilePerTopicEnabled(bool enabled)
    {
      job_config_.SetOneFilePerTopicEnabled(enabled);
    }

    void RecServerImpl::SetDescription(const std::string& description)
    {
      job_config_.SetDescription(description);
    }


    std::string RecServerImpl::GetMeasRootDir() const
    {
      return job_config_.GetMeasRootDir();
    }

    std::string RecServerImpl::GetMeasName() const
    {
      return job_config_.GetMeasName();
    }

    int64_t RecServerImpl::GetMaxFileSizeMib() const
    {
      return job_config_.GetMaxFileSize();
    }

    bool RecServerImpl::GetOneFilePerTopicEnabled() const
    {
      return job_config_.GetOneFilePerTopicEnabled();
    }

    std::string RecServerImpl::GetDescription() const
    {
      return job_config_.GetDescription();
    }

    ////////////////////////////////////
    // Server Settings
    ////////////////////////////////////

    void RecServerImpl::SetMonitoringUpdateCallback(PostUpdateCallback_T post_update_callback)
    {
      monitoring_thread_->SetPostUpdateCallbacks
      (
        {
          // 1. Update the internal lists and connections
          [this](const TopicInfoMap_T&, const HostsRunningEcalRec_T& hosts_running_ecal_rec) { UpdateRecorderConnections(hosts_running_ecal_rec); }

          // 2. User-defined callback
          , post_update_callback
        }
      );
    }

    bool RecServerImpl::SetUsingBuiltInRecorderEnabled(bool enabled)
    {
      if (recording_)
        return false;

      // DISABLE built in recorder
      if (use_built_in_recorder_ && !enabled)
      {
        std::unique_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

        use_built_in_recorder_ = false;

        // Look for the connection to the local recorder. If there is one, replace it with a remote connection
        auto local_rec_connection_it = connected_rec_clients_.find(eCAL::Process::GetHostName());
        if (local_rec_connection_it != connected_rec_clients_.end())
        {
          // Terminate the connection to the local recorder, if there was one
          local_rec_connection_it->second->SetRecorderEnabled(false);
          connected_rec_clients_.erase(local_rec_connection_it);

          // Create a new connection to an external recorder, if required.
          // The CreateNewRecorder() method will automatically create a remote connection, as the use_built_in_recorder_ is set to false.

          auto enabled_client = enabled_rec_clients_.find(eCAL::Process::GetHostName());
          RecorderSettings initial_settings;
          if (enabled_client != enabled_rec_clients_.end())
          {
            // Create initial settings, only if the the client is enabled
            initial_settings.SetEnabledAddons(enabled_client->second.enabled_addons_);
            initial_settings.SetHostFilter   (enabled_client->second.host_filter_);
          }

          auto new_recorder_connection = CreateNewRecorder(eCAL::Process::GetHostName(), initial_settings);
          if (client_connections_active_)
            new_recorder_connection->SetRecorderEnabled(true, connected_to_ecal_);

          connected_rec_clients_.emplace(eCAL::Process::GetHostName(), std::move(new_recorder_connection));
        }
      }

      // ENABLE built in recorder
      else
      {
        std::unique_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

        use_built_in_recorder_ = true;

        // Look for the connection to the local recorder. If there is one, remove it
        auto local_rec_connection_it = connected_rec_clients_.find(eCAL::Process::GetHostName());
        if (local_rec_connection_it != connected_rec_clients_.end())
        {
          // Terminate the connection to the local recorder, if there was one
          local_rec_connection_it->second->SetRecorderEnabled(false);
          local_rec_connection_it->second->WaitForPendingRequests();
          connected_rec_clients_.erase(local_rec_connection_it);
        }

        // Create a new connection to the built in recorder. We always do that,
        // as the built in recorder will always be present, no matter if the
        // local external recorder instance was running and connected.

        auto enabled_client = enabled_rec_clients_.find(eCAL::Process::GetHostName());
        RecorderSettings initial_settings;
        if (enabled_client != enabled_rec_clients_.end())
        {
          // Create initial settings, only if the the client is enabled
          initial_settings.SetEnabledAddons(enabled_client->second.enabled_addons_);
          initial_settings.SetHostFilter   (enabled_client->second.host_filter_);
        }

        auto new_recorder_connection = CreateNewRecorder(eCAL::Process::GetHostName(), initial_settings);
        if (client_connections_active_)
          new_recorder_connection->SetRecorderEnabled(true, connected_to_ecal_);

        connected_rec_clients_.emplace(eCAL::Process::GetHostName(), std::move(new_recorder_connection));

      }

      return true;
    }

    bool RecServerImpl::IsUsingBuiltInRecorderEnabled() const
    {
      return use_built_in_recorder_;
    }

    ////////////////////////////////////
    // Measurement Upload
    ////////////////////////////////////

    void RecServerImpl::SetUploadConfig(const UploadConfig& upload_config)
    {
      upload_config_ = upload_config;
    }

    UploadConfig RecServerImpl::GetUploadConfig() const
    {
      return upload_config_;
    }

    int RecServerImpl::GetInternalFtpServerOpenConnectionCount() const
    {
      return ftp_server_->getOpenConnectionCount();
    }

    uint16_t RecServerImpl::GetInternalFtpServerPort() const
    {
      return ftp_server_->getPort();
    }

    eCAL::rec::Error RecServerImpl::UploadMeasurement(int64_t meas_id)
    {
      JobHistoryEntry job_history_entry;

      eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::OK);

      {
        std::shared_lock<decltype(job_history_mutex_)> job_history_lock(job_history_mutex_);

        // Search for the correct job
        auto job_it = std::find_if(job_history_.begin(), job_history_.end(), [meas_id](const JobHistoryEntry& job_history_entry) { return job_history_entry.local_evaluated_job_config_.GetJobId() == meas_id; });
        if (job_it == job_history_.end())
        {
          error = eCAL::rec::Error(eCAL::rec::Error::ErrorCode::MEAS_ID_NOT_FOUND, std::to_string(meas_id));
        }
        else
        {
          job_history_entry = *job_it;
        }
      }

      if (!error)
        error = SimulateUploadMeasurement_NoLock(job_history_entry);

      if (!error)
        error = UploadMeasurement(job_history_entry);

      if (!error)
      {
        std::unique_lock<decltype(job_history_mutex_)> job_history_lock(job_history_mutex_);

        // Search for the correct job
        auto job_it = std::find_if(job_history_.begin(), job_history_.end(), [meas_id](const JobHistoryEntry& job_history_entry) { return job_history_entry.local_evaluated_job_config_.GetJobId() == meas_id; });
        if (job_it != job_history_.end())
        {
          job_it->is_uploaded_   = true;
          job_it->upload_config_ = upload_config_;
        }
      }
      else
      {
        eCAL::rec::EcalRecLogger::Instance()->error("Failed to upload measurement " + std::to_string(meas_id) + ": " + error.ToString());
      }

      return error;
    }

    bool RecServerImpl::CanUploadMeasurement(int64_t meas_id) const
    {
      return !SimulateUploadMeasurement(meas_id);
    }

    eCAL::rec::Error RecServerImpl::SimulateUploadMeasurement(int64_t meas_id) const
    {
      std::shared_lock<decltype (job_history_mutex_)>job_history_lock(job_history_mutex_);
      return SimulateUploadMeasurement_NoLock(meas_id);
    }

    int RecServerImpl::UploadNonUploadedMeasurements()
    {
      std::list<JobHistoryEntry> job_history_copy;

      {
        std::shared_lock<decltype (job_history_mutex_)>job_history_lock(job_history_mutex_);
        job_history_copy = job_history_;
      }

      // Remove all elements that cannot be uploaded
      job_history_copy.remove_if([this] (const JobHistoryEntry& job_history_entry) { return !CanUploadMeasurement(job_history_entry.local_evaluated_job_config_.GetJobId()); });

      // Upload all uploadable entries!
      for (const JobHistoryEntry& job_history_entry : job_history_copy)
      {
        UploadMeasurement(job_history_entry.local_evaluated_job_config_.GetJobId());
      }

      return static_cast<int>(job_history_copy.size());
    }

    bool RecServerImpl::HasAnyUploadError(int64_t meas_id) const
    {
      std::shared_lock<decltype (job_history_mutex_)>job_history_lock(job_history_mutex_);
      auto job_it = std::find_if(job_history_.begin(), job_history_.end(), [meas_id](const JobHistoryEntry& job_history_entry) { return job_history_entry.local_evaluated_job_config_.GetJobId() == meas_id; });

      if (job_it == job_history_.end())
      {
        return false;
      }
      else
      {
        for (const auto& client_status : job_it->client_statuses_)
        {
          if (((client_status.second.job_status_.state_ == eCAL::rec::JobState::Uploading)
                || (client_status.second.job_status_.state_ == eCAL::rec::JobState::FinishedUploading))
            && !client_status.second.job_status_.upload_status_.info_.first)
          {
            return true;
          }
        }
        return false;
      }

    }

    eCAL::rec::Error RecServerImpl::UploadMeasurement(const JobHistoryEntry& job_history_entry)
    {
      if (upload_config_.type_ == UploadConfig::Type::INTERNAL_FTP)
      {
        // Evaluate the local measurement path
        std::string local_measurement_path = EcalUtils::Filesystem::ToNativeSeperators(job_history_entry.local_evaluated_job_config_.GetCompleteMeasurementPath());
        if (local_measurement_path.size() == 0)
        {
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Measurement path is empty");
        }
        eCAL::rec::EcalRecLogger::Instance()->info("Start merging measurement " + std::to_string(job_history_entry.local_evaluated_job_config_.GetJobId()));

        if (local_measurement_path.back() != EcalUtils::Filesystem::NativeSeparator())
        {
          local_measurement_path.push_back(EcalUtils::Filesystem::NativeSeparator());
        }

        // The local measurement path does not exist, if the local recorder has not been selected. Thus we have to create it.
        auto dir_status = EcalUtils::Filesystem::FileStatus(local_measurement_path);

        bool path_ok = false;

        if (dir_status.IsOk() && (dir_status.GetType() == EcalUtils::Filesystem::Dir))
        {
          path_ok = true;
        }
        else if (!dir_status.IsOk())
        {
          path_ok = EcalUtils::Filesystem::MkPath(local_measurement_path);
        }

        if (!path_ok)
        {
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::RESOURCE_UNAVAILABLE, "Failed creating path \"" + local_measurement_path + "\"");
        }

        // Add a user with the measurement_path as home folder
        ftp_server_->addUser(std::to_string(job_history_entry.local_evaluated_job_config_.GetJobId())
                           , std::to_string(job_history_entry.local_evaluated_job_config_.GetJobId())
                           , local_measurement_path
                           , fineftp::Permission::FileWrite
                              | fineftp::Permission::FileAppend
                              | fineftp::Permission::FileRename
                              | fineftp::Permission::DirList
                              | fineftp::Permission::DirCreate
                              | fineftp::Permission::DirRename);

        eCAL::rec::UploadConfig upload_config;
        upload_config.meas_id_               = job_history_entry.local_evaluated_job_config_.GetJobId();
        upload_config.protocol_              = eCAL::rec::UploadConfig::Type::FTP;
        upload_config.host_                  = eCAL::Process::GetHostName();
        upload_config.port_                  = ftp_server_->getPort();
        upload_config.username_              = std::to_string(job_history_entry.local_evaluated_job_config_.GetJobId());
        upload_config.password_              = std::to_string(job_history_entry.local_evaluated_job_config_.GetJobId());
        upload_config.upload_path_           = "/";
        upload_config.upload_metadata_files_ = false;
        upload_config.delete_after_upload_   = upload_config_.delete_after_upload_;

        RecorderCommand upload_command;
        upload_command.type_          = RecorderCommand::Type::UPLOAD_MEASUREMENT;
        upload_command.upload_config_ = std::move(upload_config);

        {
          std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

          auto clients_that_need_to_upload = GetClientsThatNeedToUpload_NoLock(job_history_entry);
          for (auto& client : clients_that_need_to_upload)
          {
            if (client.second.second)
            {
              // Upload metadata!
              RecorderCommand also_upload_metadata_command(upload_command);
              also_upload_metadata_command.upload_config_.upload_metadata_files_ = true;

              client.second.first->SetCommand(also_upload_metadata_command);
            }
            else
            {
              client.second.first->SetCommand(upload_command);
            }
          }
        }

        return eCAL::rec::Error::ErrorCode::OK;
      }
      else if (upload_config_.type_ == UploadConfig::Type::FTP)
      {
        // Evaluate the local measurement name
        std::string local_measurement_name = EcalUtils::Filesystem::ToUnixSeperators(job_history_entry.local_evaluated_job_config_.GetMeasName());
        if (local_measurement_name.size() == 0)
        {
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Measurement name is empty");
        }
        eCAL::rec::EcalRecLogger::Instance()->info("Start merging measurement " + std::to_string(job_history_entry.local_evaluated_job_config_.GetJobId()));

        std::string ftp_root_dir = EcalUtils::Filesystem::ToUnixSeperators(upload_config_.root_path_);
        if (ftp_root_dir.empty() || (ftp_root_dir.front() != '/'))
        {
          ftp_root_dir = "/" + ftp_root_dir;
        }

        if (ftp_root_dir.back() != '/')
        {
          ftp_root_dir += "/";
        }

        std::string ftp_measurement_path = EcalUtils::Filesystem::CleanPath(ftp_root_dir + local_measurement_name, EcalUtils::Filesystem::OsStyle::Unix);
        
        if (ftp_measurement_path.back() != '/')
        {
          ftp_measurement_path.push_back('/');
        }

        // Create generic upload config
        eCAL::rec::UploadConfig upload_config;
        upload_config.meas_id_               = job_history_entry.local_evaluated_job_config_.GetJobId();
        upload_config.protocol_              = eCAL::rec::UploadConfig::Type::FTP;
        upload_config.host_                  = upload_config_.host_;
        upload_config.port_                  = upload_config_.port_;
        upload_config.username_              = upload_config_.username_;
        upload_config.password_              = upload_config_.password_;
        upload_config.upload_path_           = ftp_measurement_path;
        upload_config.upload_metadata_files_ = false;
        upload_config.delete_after_upload_   = upload_config_.delete_after_upload_;

        RecorderCommand upload_command;
        upload_command.type_          = RecorderCommand::Type::UPLOAD_MEASUREMENT;
        upload_command.upload_config_ = std::move(upload_config);

        {
          std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

          auto clients_that_need_to_upload = GetClientsThatNeedToUpload_NoLock(job_history_entry);
          for (auto& client : clients_that_need_to_upload)
          {
            if (client.second.second)
            {
              // Upload metadata!
              RecorderCommand also_upload_metadata_command(upload_command);
              also_upload_metadata_command.upload_config_.upload_metadata_files_ = true;

              client.second.first->SetCommand(also_upload_metadata_command);
            }
            else
            {
              client.second.first->SetCommand(upload_command);
            }
          }
        }
        return eCAL::rec::Error::ErrorCode::OK;
      }
      else
      {
        return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::UNSUPPORTED_ACTION, "Unsupported Protocol");
      }
    }

    eCAL::rec::Error RecServerImpl::SimulateUploadMeasurement_NoLock(int64_t meas_id) const
    {
      eCAL::rec::Error error(eCAL::rec::Error::OK);

      auto job_it = std::find_if(job_history_.begin(), job_history_.end(), [meas_id](const JobHistoryEntry& job_history_entry) { return job_history_entry.local_evaluated_job_config_.GetJobId() == meas_id; });
      if (job_it == job_history_.end())
      {
        return eCAL::rec::Error(eCAL::rec::Error::MEAS_ID_NOT_FOUND, std::to_string(meas_id));
      }
      else
      {
        return SimulateUploadMeasurement_NoLock(*job_it);
      }
    }

    eCAL::rec::Error RecServerImpl::SimulateUploadMeasurement_NoLock(const JobHistoryEntry& job_history_entry) const
    {
      if (job_history_entry.is_deleted_)
        return eCAL::rec::Error::ErrorCode::MEAS_IS_DELETED;

      // Check if it would even make sense to upload this measurement
      if (upload_config_.type_ == UploadConfig::Type::INTERNAL_FTP)
      {
        bool any_other_host = false;

        for (const auto& client_status : job_history_entry.client_statuses_)
        {
          if (client_status.first != eCAL::Process::GetHostName())
          {
            any_other_host = true;
            continue;
          }
        }

        if (!any_other_host)
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::ACTION_SUPERFLOUS);
      }
      else if (upload_config_.type_ == UploadConfig::Type::FTP)
      {
        if (upload_config_.host_.empty())
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Hostname is empty");
      }
      else
      {
        return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::UNSUPPORTED_ACTION, "Unsupported Protocol");
      }

      // Check if the rec server has a valid measurement name. We don't allow
      // uploading without a measurement name.
      if (job_history_entry.local_evaluated_job_config_.GetMeasName().empty())
      {
        return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Meas name is empty");
      }

      eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::OK);

      // Check if all clients have finished flushing
      std::set<std::string> hosts_recording;
      std::set<std::string> hosts_flushing;
      std::set<std::string> hosts_uploading;

      std::set<std::string> hosts_upload_finished_but_failed;

      for (const auto& client : job_history_entry.client_statuses_)
      {
        if ((client.second.job_status_.state_ == eCAL::rec::JobState::Recording)
          || anyAddonStateIs_NoLock(eCAL::rec::RecAddonJobStatus::State::Recording, client.second.job_status_))
        {
          hosts_recording.emplace(client.first);
        }
        else if ((client.second.job_status_.state_ == eCAL::rec::JobState::Flushing)
          || anyAddonStateIs_NoLock(eCAL::rec::RecAddonJobStatus::State::Flushing, client.second.job_status_))
        {
          hosts_flushing.emplace(client.first);
        }
        else if (client.second.job_status_.state_ == eCAL::rec::JobState::Uploading)
        {
          hosts_uploading.emplace(client.first);
        }
        else if ((client.second.job_status_.state_ == eCAL::rec::JobState::FinishedUploading)
          && !client.second.job_status_.upload_status_.info_.first)
        {
          hosts_upload_finished_but_failed.emplace(client.first);
        }
      }

      if (!hosts_recording.empty())
        error = eCAL::rec::Error(eCAL::rec::Error::CURRENTLY_RECORDING, EcalUtils::String::Join(", ", hosts_recording));
      else if (!hosts_flushing.empty())
        error = eCAL::rec::Error(eCAL::rec::Error::CURRENTLY_FLUSHING, EcalUtils::String::Join(", ", hosts_flushing));
      else if (!hosts_uploading.empty())
        error = eCAL::rec::Error(eCAL::rec::Error::CURRENTLY_UPLOADING, EcalUtils::String::Join(", ", hosts_uploading));
      else if (job_history_entry.is_uploaded_ && hosts_upload_finished_but_failed.empty())
        error = eCAL::rec::Error(eCAL::rec::Error::ALREADY_UPLOADED);

      return error;
    }

    std::map<std::string, std::pair<AbstractRecorder*, bool>> RecServerImpl::GetClientsThatNeedToUpload_NoLock(const JobHistoryEntry& job_history_entry) const
    {
      std::set<std::string>                                     all_involved_hosts;
      std::map<std::string, std::pair<AbstractRecorder*, bool>> hosts_still_need_to_upload;

      for (const auto& client_status : job_history_entry.client_statuses_)
      {
        // Create a set of involved client names. The set is always sorted. Thus
        // we can use it to determine which client has to upload metadata:
        //   - When the local recorder participated: The local recorder
        //   - When the local recorder did not participate: The alphabetically first
        all_involved_hosts.emplace(client_status.first);

        // The local recorder does not need to upload the measurement to its own FTP Server
        if ((upload_config_.type_ == eCAL::rec_server::UploadConfig::Type::INTERNAL_FTP)
          && (client_status.first == eCAL::Process::GetHostName()))
        {
          continue;
        }

        // Collect pointers to the client connections that we need to send the upload command to
        if (!job_history_entry.is_uploaded_ 
            || (job_history_entry.is_uploaded_ 
                && (client_status.second.job_status_.state_ == eCAL::rec::JobState::FinishedUploading)
                && !client_status.second.job_status_.upload_status_.info_.first
                && !client_status.second.job_status_.is_deleted_)
            || (job_history_entry.is_uploaded_ && (client_status.second.job_status_.state_ == eCAL::rec::JobState::FinishedFlushing)))
        {
          auto connected_client_it = connected_rec_clients_.find(client_status.first);
          if (connected_client_it != connected_rec_clients_.end())
          {
            hosts_still_need_to_upload.emplace(client_status.first, std::make_pair(connected_client_it->second.get(), false));
          }
        }

      }

      // Now let's see who is responsible for metadata upload!
      std::string metadata_uploader;
      if (all_involved_hosts.find(eCAL::Process::GetHostName()) != all_involved_hosts.end())
        metadata_uploader = eCAL::Process::GetHostName();
      else if (!all_involved_hosts.empty())
        metadata_uploader = *all_involved_hosts.begin();

      auto host_need_to_upload_it = hosts_still_need_to_upload.find(metadata_uploader);
      if (host_need_to_upload_it != hosts_still_need_to_upload.end())
        host_need_to_upload_it->second.second = true;

      return hosts_still_need_to_upload;
    }

    ////////////////////////////////////
    // Comments
    ////////////////////////////////////
    eCAL::rec::Error RecServerImpl::AddComment(int64_t meas_id, const std::string& comment)
    {
      std::set<std::string> hosts_to_send_command_to;

      eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::OK);

      // Check if it should be possible to add a comment to this measurement
      int local_rec_connection_pid = -1;
      {
        std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);
        local_rec_connection_pid = GetLocalRecConnectionPid_NoLock();
      }
      {
        std::shared_lock<decltype (job_history_mutex_)>job_history_lock(job_history_mutex_);

        auto job_it = std::find_if(job_history_.begin(), job_history_.end(), [meas_id](const JobHistoryEntry& job_history_entry) { return job_history_entry.local_evaluated_job_config_.GetJobId() == meas_id; });
        if (job_it == job_history_.end())
        {
          error = eCAL::rec::Error(eCAL::rec::Error::MEAS_ID_NOT_FOUND, std::to_string(meas_id));
        }
        else
        {
          error = SimulateAddComment_NoLock(*job_it, local_rec_connection_pid);
        }

        if (!error)
        {
          for (const auto& client_status : job_it->client_statuses_)
          {
            // Create a lists of all involved hosts that haven't deleted the measurement, yet
            if (!client_status.second.job_status_.is_deleted_)
              hosts_to_send_command_to.emplace(client_status.first);
          }
        }
      }

      // Send the add comment statement to all hosts
      if (!error)
      {
        std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

        for (const std::string& host : hosts_to_send_command_to)
        {
          auto rec_connection_it = connected_rec_clients_.find(host);
          if (rec_connection_it != connected_rec_clients_.end())
          {
            // Send AddComment 
            RecorderCommand add_comment_command;
            add_comment_command.type_              = RecorderCommand::Type::ADD_COMMENT;
            add_comment_command.meas_id_add_delete = meas_id;

            auto now = std::chrono::system_clock::now();
            time_t now_time_t = std::chrono::system_clock::to_time_t(now);
            std::tm now_timeinfo {};
  #if defined(__unix__)
            localtime_r(&now_time_t,            &now_timeinfo);
  #elif defined(_MSC_VER)
            localtime_s(&now_timeinfo,  &now_time_t);
  #else
            static std::mutex mtx;
            {
              std::lock_guard<std::mutex> lock(mtx);
              now_timeinfo = *std::localtime(&now_time_t);
            }
  #endif

            char time_string_char_p[64];
            size_t strftime_bytes = strftime(time_string_char_p, 64, "%F, %R", &now_timeinfo);

            add_comment_command.comment_             = "\n------------------------------------\n>> Comment added " + std::string(time_string_char_p, strftime_bytes) + ":\n\n" + comment;
          
            rec_connection_it->second->SetCommand(add_comment_command);
          }
        }
      }

      if (error)
        eCAL::rec::EcalRecLogger::Instance()->error("Failed to add comment to measurement " + std::to_string(meas_id) + ": " + error.ToString());

      return error;
    }

    bool RecServerImpl::CanAddComment(int64_t meas_id) const
    {
      return !SimulateAddComment(meas_id);
    }

    eCAL::rec::Error RecServerImpl::SimulateAddComment(int64_t meas_id) const
    {
      int local_rec_connection_pid = -1;

      {
        std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)> rec_clients_lock(connected_enabled_rec_clients_mutex_);
        local_rec_connection_pid = GetLocalRecConnectionPid_NoLock();
      }

      {
        std::shared_lock<decltype (job_history_mutex_)>job_history_lock(job_history_mutex_);
        return SimulateAddComment_NoLock(meas_id, local_rec_connection_pid);
      }
    }

    int RecServerImpl::GetLocalRecConnectionPid_NoLock() const
    {
      auto local_rec_connection_it = connected_rec_clients_.find(eCAL::Process::GetHostName());

      if (local_rec_connection_it != connected_rec_clients_.end())
      {
        return local_rec_connection_it->second->GetStatus().first.pid_;
      }
      else
      {
        return -1;
      }
    }

    eCAL::rec::Error RecServerImpl::SimulateAddComment_NoLock(int64_t meas_id, int local_rec_connection_pid) const
    {
      eCAL::rec::Error error(eCAL::rec::Error::OK);

      auto job_it = std::find_if(job_history_.begin(), job_history_.end(), [meas_id](const JobHistoryEntry& job_history_entry) { return job_history_entry.local_evaluated_job_config_.GetJobId() == meas_id; });
      if (job_it == job_history_.end())
      {
        return eCAL::rec::Error(eCAL::rec::Error::MEAS_ID_NOT_FOUND, std::to_string(meas_id));
      }
      else
      {
        return SimulateAddComment_NoLock(*job_it, local_rec_connection_pid);
      }

    }

    eCAL::rec::Error RecServerImpl::SimulateAddComment_NoLock(const JobHistoryEntry& job_history_entry, int local_rec_connection_pid) const
    {
      if (job_history_entry.is_deleted_)
        return eCAL::rec::Error::ErrorCode::MEAS_IS_DELETED;

      if (!job_history_entry.is_uploaded_)
      {
        // We can always add comments to measurements that haven't been uploaded, yet
        return eCAL::rec::Error::ErrorCode::OK;
      }
      else
      {
        // We have to check:
        //   - If the measurement is currently being uploaded => Error!
        //   - If the measurement has been uploaded successfully...
        //        - ... to an external FTP => Error!
        //        - ... to the built-in FTP ...
        //              - And the local recorder participated and is still online => OK
        //              - Or the local recorder did not participate or is not online any more => Error!

        std::set<std::string> hosts_uploading;
        for (const auto& client : job_history_entry.client_statuses_)
        {
          if (client.second.job_status_.state_ == eCAL::rec::JobState::Uploading)
            hosts_uploading.emplace(client.first);
        }

        if (!hosts_uploading.empty())
        {
          // If any host is currently uploading, we must not write data to the measurement
          return eCAL::rec::Error(eCAL::rec::Error::CURRENTLY_UPLOADING, EcalUtils::String::Join(", ", hosts_uploading));
        }

        if (job_history_entry.upload_config_.type_ != eCAL::rec_server::UploadConfig::Type::INTERNAL_FTP) 
        {
          // If the measurement has been uploaded to an external server we cannot modify it any more
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::ALREADY_UPLOADED, "Unable to change measurement uploaded to an external server");
        }

        // If the measurement has already been uploaded, we must check if the 
        // host that the measurement has been uploaded to (--> this host) can
        // still write data to the measurement.

        auto this_host_status_it = job_history_entry.client_statuses_.find(eCAL::Process::GetHostName());

        if ((this_host_status_it != job_history_entry.client_statuses_.end())                     // => The local recorder has participated
          && (local_rec_connection_pid > 0)                                                       // => there is a local recorder connection
          && (this_host_status_it->second.client_pid_ == local_rec_connection_pid)                // => the local recorder instance ist still the one from the measurement
          && (this_host_status_it->second.job_status_.state_ != eCAL::rec::JobState::NotStarted)) // => it has not failed to start the recording
        {
          return eCAL::rec::Error::ErrorCode::OK;
        }
        else
        {
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::ALREADY_UPLOADED, "Original local recorder instance is not reachable any more or has not participated in the recording");
        }
      }
    }

    ////////////////////////////////////
    // Delete measurement
    ////////////////////////////////////
    bool RecServerImpl::CanDeleteMeasurement(int64_t meas_id) const
    {
      return !SimulateDeleteMeasurement(meas_id);
    }

    eCAL::rec::Error RecServerImpl::SimulateDeleteMeasurement(int64_t meas_id) const
    {
      std::shared_lock<decltype (job_history_mutex_)>job_history_lock(job_history_mutex_);
      return SimulateDeleteMeasurement_NoLock(meas_id);
    }

    eCAL::rec::Error RecServerImpl::DeleteMeasurement(int64_t meas_id)
    {
      std::set<std::string> hosts_to_delete_on;

      eCAL::rec::Error error(eCAL::rec::Error::OK);

      // Check if it should be possible to delete this measurement
      {
        std::shared_lock<decltype (job_history_mutex_)>job_history_lock(job_history_mutex_);

        auto job_it = std::find_if(job_history_.begin(), job_history_.end(), [meas_id](const JobHistoryEntry& job_history_entry) { return job_history_entry.local_evaluated_job_config_.GetJobId() == meas_id; });
        if (job_it == job_history_.end())
        {
          error = eCAL::rec::Error(eCAL::rec::Error::MEAS_ID_NOT_FOUND, std::to_string(meas_id));
        }
        else
        {
          error = SimulateDeleteMeasurement_NoLock(*job_it);
        }

        if (!error)
        {
          for (const auto& client_status : job_it->client_statuses_)
          {
            if (!client_status.second.job_status_.is_deleted_)
              hosts_to_delete_on.emplace(client_status.first);
          }
          job_it->is_deleted_ = true;
        }
      }

      // Send delete commands
      if (!error)
      {
        eCAL::rec::EcalRecLogger::Instance()->info("Deleting measurement " + std::to_string(meas_id) + " on " + EcalUtils::String::Join(", ", hosts_to_delete_on));
        std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);

        for (const auto& recorder : connected_rec_clients_)
        {
          // Only let external recorders upload their data that were involved in the measurement
          if (hosts_to_delete_on.find(recorder.first) != hosts_to_delete_on.end())
          {
            RecorderCommand delete_command;
            delete_command.type_              = RecorderCommand::Type::DELETE_MEASUREMENT;
            delete_command.meas_id_add_delete = meas_id;

            recorder.second->SetCommand(delete_command);
          }
        }
      }

      if (error)
        eCAL::rec::EcalRecLogger::Instance()->error("Failed to delete measurement " + std::to_string(meas_id) + ": " + error.ToString());

      return error;
    }

    eCAL::rec::Error RecServerImpl::SimulateDeleteMeasurement_NoLock(int64_t meas_id) const
    {
      eCAL::rec::Error error(eCAL::rec::Error::OK);

      auto job_it = std::find_if(job_history_.begin(), job_history_.end(), [meas_id](const JobHistoryEntry& job_history_entry) { return job_history_entry.local_evaluated_job_config_.GetJobId() == meas_id; });
      if (job_it == job_history_.end())
      {
        return eCAL::rec::Error(eCAL::rec::Error::MEAS_ID_NOT_FOUND, std::to_string(meas_id));
      }
      else
      {
        return SimulateDeleteMeasurement_NoLock(*job_it);
      }
    }

    eCAL::rec::Error RecServerImpl::SimulateDeleteMeasurement_NoLock(const JobHistoryEntry& job_history_entry) const
    {
      if (job_history_entry.is_deleted_)
        return eCAL::rec::Error::ErrorCode::MEAS_IS_DELETED;

      eCAL::rec::Error error(eCAL::rec::Error::OK);

      std::set<std::string> hosts_recording;
      std::set<std::string> hosts_flushing;
      std::set<std::string> hosts_uploading;

      for (const auto& client : job_history_entry.client_statuses_)
      {
        if ((client.second.job_status_.state_ == eCAL::rec::JobState::Recording)
          || anyAddonStateIs_NoLock(eCAL::rec::RecAddonJobStatus::State::Recording, client.second.job_status_))
        {
          hosts_recording.emplace(client.first);
        }
        else if ((client.second.job_status_.state_ == eCAL::rec::JobState::Flushing)
          || anyAddonStateIs_NoLock(eCAL::rec::RecAddonJobStatus::State::Flushing, client.second.job_status_))
        {
          hosts_flushing.emplace(client.first);
        }
        else if (client.second.job_status_.state_ == eCAL::rec::JobState::Uploading)
        {
          hosts_uploading.emplace(client.first);
        }
      }

      if (!hosts_recording.empty())
        error = eCAL::rec::Error(eCAL::rec::Error::CURRENTLY_RECORDING, EcalUtils::String::Join(", ", hosts_recording));
      else if (!hosts_flushing.empty())
        error = eCAL::rec::Error(eCAL::rec::Error::CURRENTLY_FLUSHING, EcalUtils::String::Join(", ", hosts_flushing));
      else if (!hosts_uploading.empty())
        error = eCAL::rec::Error(eCAL::rec::Error::CURRENTLY_UPLOADING, EcalUtils::String::Join(", ", hosts_uploading));

      return error;
    }

    bool RecServerImpl::anyAddonStateIs_NoLock(eCAL::rec::RecAddonJobStatus::State state, const eCAL::rec::JobStatus& job_status)
    {
      for (const auto& addon_status : job_status.rec_addon_statuses_)
      {
        if (addon_status.second.state_ == state)
          return true;
      }
      return false;
    }

    ////////////////////////////////////
    // Config Save / Load / Get
    ////////////////////////////////////
    
    RecServerConfig RecServerImpl::GetConfig() const
    {
      eCAL::rec_server::RecServerConfig config;

      config.root_dir_                  = GetMeasRootDir();
      config.meas_name_                 = GetMeasName();
      config.max_file_size_             = GetMaxFileSizeMib();
      config.one_file_per_topic_        = GetOneFilePerTopicEnabled();
      config.description_               = GetDescription();
      config.enabled_clients_config_    = GetEnabledRecClients();
      config.pre_buffer_enabled_        = GetPreBufferingEnabled();
      config.pre_buffer_length_         = GetMaxPreBufferLength();
      config.built_in_recorder_enabled_ = IsUsingBuiltInRecorderEnabled();
      config.record_mode_               = GetRecordMode();
      config.upload_config_             = GetUploadConfig();

      if (config.record_mode_ != eCAL::rec::RecordMode::All)
        config.listed_topics_ = GetListedTopics();

      return config;
    }

    eCAL::rec::Error RecServerImpl::SetConfig(const RecServerConfig& config)
    {
      // Set settings that can fail
      {
        const auto current_enabled_clients_config = GetEnabledRecClients();
        if (current_enabled_clients_config != config.enabled_clients_config_)
        {
          if (recording_)
            return eCAL::rec::Error(eCAL::rec::Error::CURRENTLY_RECORDING, "Cannot enable / disable rec clients while recording");

          if (!SetEnabledRecClients(config.enabled_clients_config_))
            return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR, "Failed enabling / disabling rec clients");
        }
      }

      {
        const auto current_built_in_recorder_enabled_ = IsUsingBuiltInRecorderEnabled();
        if (current_built_in_recorder_enabled_ != config.built_in_recorder_enabled_)
        {
          if (recording_)
            return eCAL::rec::Error(eCAL::rec::Error::CURRENTLY_RECORDING, "Cannot switch between built-in and external local recorder while recording");

          if (!SetUsingBuiltInRecorderEnabled(config.built_in_recorder_enabled_))
            return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR, "Failed switch between built-in and external local recorder while recording");
        }
      }

      {
        const auto current_record_mode   = GetRecordMode();
        const auto current_listed_topics = GetListedTopics();

        if ((current_record_mode != config.record_mode_)
          || (current_listed_topics != config.listed_topics_))
        {
          if (recording_)
            return eCAL::rec::Error(eCAL::rec::Error::CURRENTLY_RECORDING, "Cannot set the record mode while recording");

          if (!SetRecordMode(config.record_mode_, config.listed_topics_))
            return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR, "Failed setting the record mode");
        }
      }

      // Set settings that cannot fail
      SetMeasRootDir             (config.root_dir_);
      SetMeasName                (config.meas_name_);
      SetMaxFileSizeMib          (config.max_file_size_);
      SetDescription             (config.description_);
      SetOneFilePerTopicEnabled  (config.one_file_per_topic_);
      SetPreBufferingEnabled     (config.pre_buffer_enabled_);
      SetMaxPreBufferLength      (config.pre_buffer_length_);
      SetUploadConfig            (config.upload_config_);

      return eCAL::rec::Error::OK;
    }

    bool RecServerImpl::ClearConfig()
    {
      if (recording_)
      {
        eCAL::rec::EcalRecLogger::Instance()->error("Error clearing config: A recording is running.");
        return false;
      }

      SetEnabledRecClients  (std::map<std::string, ClientConfig>());
      SetMaxPreBufferLength (std::chrono::steady_clock::duration(0));
      SetPreBufferingEnabled(false);
      SetRecordMode         (eCAL::rec::RecordMode::All);
      SetMeasRootDir        ("");
      SetMeasName           ("");
      SetMaxFileSizeMib     (100);
      SetOneFilePerTopicEnabled(false);
      SetDescription        ("");
      
      loaded_config_path_    = "";
      loaded_config_version_ = -1;

      return true;
    }

    bool RecServerImpl::SaveConfigToFile(const std::string& path)
    {
      bool success = config::writeConfigToFile(*this, path);
      if (success)
      {
        loaded_config_path_    = path;
        loaded_config_version_ = config::NATIVE_CONFIG_VERSION;
      }
      return success;
    }

    bool RecServerImpl::LoadConfigFromFile(const std::string& path)
    {
      if (recording_)
      {
        eCAL::rec::EcalRecLogger::Instance()->error("Error loading config: A recording is running.");
        return false;
      }

      {
        // Disable the connections to all clients for safety measures.
        // The user has to hit "Activate" for the connections becoming enabled
        // again.
        std::shared_lock<decltype (connected_enabled_rec_clients_mutex_)>rec_clients_lock(connected_enabled_rec_clients_mutex_);
        if (client_connections_active_)
        {
          SetConnectionToClientsActive_NoLock(false);
        }
      }

      bool success = config::readConfigFromFile(*this, path, &loaded_config_version_);
      if (success)
      {
        loaded_config_path_ = path;
      }
      return success;
    }

    std::string RecServerImpl::GetLoadedConfigPath() const
    {
      return loaded_config_path_;
    }

    int RecServerImpl::GetLoadedConfigVersion() const
    {
      return loaded_config_version_;
    }

    int RecServerImpl::GetNativeConfigVersion() const
    {
      return config::NATIVE_CONFIG_VERSION;
    }
  }
}
