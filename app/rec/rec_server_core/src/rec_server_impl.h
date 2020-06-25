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

#include <cstdint>
#include <string>
#include <map>
#include <functional>
#include <chrono>
#include <set>
#include <memory>
#include <shared_mutex>
#include <list>

#include <rec_client_core/topic_info.h>
#include <rec_client_core/record_mode.h>
#include <rec_client_core/state.h>
#include <rec_client_core/job_config.h>

#include <rec_server_core/rec_server.h>

#include <fineftp/server.h>

#include "recorder_settings.h"

namespace eCAL
{
  namespace rec
  {
    class EcalRec;
  }

  namespace rec_server
  {
    ////////////////////////////////////
    // Forward declarations
    ////////////////////////////////////
    class MonitoringThread;
    class AbstractRecorder;

    class RecServerImpl
    {
    ////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////
    public:
      RecServerImpl();
      ~RecServerImpl();

    ////////////////////////////////////
    // Client management
    ////////////////////////////////////
    public:
      bool SetEnabledRecClients(const std::map<std::string, ClientConfig>& enabled_rec_clients);
      std::map<std::string, ClientConfig> GetEnabledRecClients() const;

      bool SetHostFilter (const std::string& hostname, const std::set<std::string>& host_filter); // TODO: evaluate if I really need that
      std::set<std::string> GetHostFilter (const std::string& hostname) const;

      bool SetConnectionToClientsActive(bool active);
      bool IsConnectionToClientsActive() const;

    private:
      void UpdateRecorderConnections(const HostsRunningEcalRec_T& hosts_running_ecal_rec);
      std::unique_ptr<AbstractRecorder> CreateNewRecorder(const std::string& hostname, const RecorderSettings& initial_settings = RecorderSettings());

      bool IsRecClientEnabled(const std::string& hostname) const;

      void SetConnectionToClientsActive_NoLock(bool active);

    ////////////////////////////////////
    // Recorder control
    ////////////////////////////////////
    public:
      bool ConnectToEcal      ();
      bool DisconnectFromEcal ();
      bool SavePreBufferedData();
      bool StartRecording     ();
      bool StopRecording      ();

      bool IsConnectedToEcal             () const;
      bool IsRecording                   () const;
      int64_t GetCurrentlyRecordingMeasId() const;

      bool                  IsAnyRequestPending        () const;
      std::set<std::string> GetHostsWithPendingRequests() const;
      void                  WaitForPendingRequests     () const;

    private:
      JobHistoryEntry initializeJobHistoryEntry_NoLock(std::chrono::system_clock::time_point now, const eCAL::rec::JobConfig& local_evaluated_job_config);

    ////////////////////////////////////
    // Status
    ////////////////////////////////////
    public:
      eCAL::rec_server::RecorderStatusMap_T GetRecorderStatuses() const;
      eCAL::rec::RecorderStatus GetBuiltInRecorderInstanceStatus() const;

      TopicInfoMap_T GetTopicInfo() const;
      HostsRunningEcalRec_T GetHostsRunningEcalRec() const;

      std::list<eCAL::rec_server::JobHistoryEntry> GetJobHistory() const;

      RecServerStatus GetStatus() const;

    private:
      void UpdateJobstatusCallback(const std::string& hostname, const eCAL::rec::RecorderStatus& recorder_status);
      void ReportJobCommandResponseCallback(int64_t job_id, const std::string& hostname, const std::pair<bool, std::string>& response_info);

      std::map<std::string, int32_t> GetRunningEnabledRecClients() const;

    ////////////////////////////////////
    // General Client Settings        //
    ////////////////////////////////////
    public:
      void SetMaxPreBufferLength (std::chrono::steady_clock::duration max_pre_buffer_length);
      void SetPreBufferingEnabled(bool pre_buffering_enabled);
      bool SetRecordMode         (eCAL::rec::RecordMode record_mode);
      bool SetRecordMode         (eCAL::rec::RecordMode record_mode, const std::set<std::string>& listed_topics);
      bool SetListedTopics       (std::set<std::string> listed_topics);

      std::chrono::steady_clock::duration GetMaxPreBufferLength () const;
      bool                                GetPreBufferingEnabled() const;
      eCAL::rec::RecordMode               GetRecordMode         () const;
      std::set<std::string>               GetListedTopics       () const;

    ////////////////////////////////////
    // Job Settings                   //
    ////////////////////////////////////
    public:
      void SetMeasRootDir   (const std::string& meas_root_dir);
      void SetMeasName      (const std::string& meas_name);
      void SetMaxFileSizeMib(unsigned int max_file_size_mib);
      void SetDescription   (const std::string& description);

      std::string  GetMeasRootDir   () const;
      std::string  GetMeasName      () const;
      unsigned int GetMaxFileSizeMib() const;
      std::string  GetDescription   () const;

    ////////////////////////////////////
    // Server Settings                //
    ////////////////////////////////////
    public:
      void SetMonitoringUpdateCallback(PostUpdateCallback_T post_update_callback);

      bool SetUsingBuiltInRecorderEnabled(bool enabled);
      bool IsUsingBuiltInRecorderEnabled() const;

    ////////////////////////////////////
    // Measurement Upload             //
    ////////////////////////////////////
    public:
      void SetUploadConfig(const UploadConfig& upload_config);
      UploadConfig GetUploadConfig() const;
      int GetInternalFtpServerOpenConnectionCount() const;
      uint16_t GetInternalFtpServerPort() const;

      eCAL::rec::Error UploadMeasurement(int64_t meas_id);
      bool CanUploadMeasurement(int64_t meas_id) const;
      eCAL::rec::Error SimulateUploadMeasurement(int64_t meas_id) const;
      int  UploadNonUploadedMeasurements();

    private:
      eCAL::rec::Error UploadMeasurement(const JobHistoryEntry& job_history_entry);
      eCAL::rec::Error SimulateUploadMeasurement_NoLock(int64_t meas_id) const;
      eCAL::rec::Error SimulateUploadMeasurement_NoLock(const JobHistoryEntry& job_history_entry) const;

    ////////////////////////////////////
    // Comments
    ////////////////////////////////////
    public:
      eCAL::rec::Error AddComment(int64_t meas_id, const std::string& comment);
      bool CanAddComment(int64_t meas_id) const;
      eCAL::rec::Error SimulateAddComment(int64_t meas_id) const;

    private:
      int GetLocalRecConnectionPid_NoLock() const;
      eCAL::rec::Error SimulateAddComment_NoLock(int64_t meas_id, int local_rec_connection_pid) const;
      eCAL::rec::Error SimulateAddComment_NoLock(const JobHistoryEntry& job_history_entry, int local_rec_connection_pid) const;

    ////////////////////////////////////
    // Delete measurement
    ////////////////////////////////////
    public:
      bool CanDeleteMeasurement(int64_t meas_id) const;
      eCAL::rec::Error SimulateDeleteMeasurement(int64_t meas_id) const;
      eCAL::rec::Error DeleteMeasurement(int64_t meas_id);

    private:
      eCAL::rec::Error SimulateDeleteMeasurement_NoLock(int64_t meas_id) const;
      eCAL::rec::Error SimulateDeleteMeasurement_NoLock(const JobHistoryEntry& job_history_entry) const;
      
      static bool anyAddonStateIs_NoLock(eCAL::rec::RecAddonJobStatus::State state, const eCAL::rec::JobStatus& job_status);

    ////////////////////////////////////
    // Config Save / Load
    ////////////////////////////////////
    public:
      bool ClearConfig();
      bool SaveConfigToFile(const std::string& path);
      bool LoadConfigFromFile(const std::string& path);

      std::string GetLoadedConfigPath() const;
      int GetLoadedConfigVersion() const;
      int GetNativeConfigVersion() const;

    ////////////////////////////////////
    // Member Variables               //
    ////////////////////////////////////
    private:
      std::unique_ptr<MonitoringThread>                        monitoring_thread_;

      mutable std::shared_timed_mutex                          connected_enabled_rec_clients_mutex_; // We need to use the _timed_ variant here to be C++14 compatible
      std::map<std::string, std::unique_ptr<AbstractRecorder>> connected_rec_clients_;
      std::map<std::string, ClientConfig>                      enabled_rec_clients_;

      bool                                                     client_connections_active_;

      RecorderSettings settings_;
      eCAL::rec::JobConfig job_config_;

      bool      connected_to_ecal_;
      bool      recording_;
      int64_t   currently_recording_meas_id_;

      mutable std::shared_timed_mutex job_history_mutex_;
      std::list<JobHistoryEntry>      job_history_;

      // Options
      bool                                use_built_in_recorder_;
      std::shared_ptr<eCAL::rec::EcalRec> ecal_rec_instance_;

      // FTP
      UploadConfig                        upload_config_;
      std::unique_ptr<fineftp::FtpServer> ftp_server_;

      std::string                         loaded_config_path_;
      int                                 loaded_config_version_;
    };
  }
}
