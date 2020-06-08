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
#include <list>

#include <rec_client_core/topic_info.h>
#include <rec_client_core/record_mode.h>
#include <rec_client_core/state.h>
#include <rec_client_core/rec_error.h>

#include <rec_server_core/rec_server_config.h>
#include <rec_server_core/rec_server_types.h>
#include <rec_server_core/status.h>

namespace eCAL
{
  namespace rec_server
  {
    ////////////////////////////////////
    // Forward declarations
    ////////////////////////////////////
    class RecServerImpl;

    class RecServer
    {
    ////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////
    public:
      RecServer();
      ~RecServer();

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

    ////////////////////////////////////
    // General Client Settings
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
    // Job Settings
    ////////////////////////////////////
    public:
      void SetMeasRootDir   (std::string  meas_root_dir);
      void SetMeasName      (std::string  meas_name);
      void SetMaxFileSizeMib(unsigned int max_file_size_mib);
      void SetDescription   (std::string  description);

      std::string  GetMeasRootDir   () const;
      std::string  GetMeasName      () const;
      unsigned int GetMaxFileSizeMib() const;
      std::string  GetDescription   () const;

    ////////////////////////////////////
    // Server Settings
    ////////////////////////////////////
    public:
      void SetMonitoringUpdateCallback(PostUpdateCallback_T post_update_callback);

      bool SetUsingBuiltInRecorderEnabled(bool enabled);
      bool IsUsingBuiltInRecorderEnabled() const;

    ////////////////////////////////////
    // Measurement Upload
    ////////////////////////////////////
    public:
      void SetUploadConfig(const UploadConfig& upload_config);
      UploadConfig GetUploadConfig() const;
      int GetInternalFtpServerOpenConnectionCount() const;
      uint16_t GetInternalFtpServerPort() const;

      eCAL::rec::Error UploadMeasurement(int64_t meas_id);
      bool CanUploadMeasurement(int64_t meas_id) const;
      eCAL::rec::Error SimulateUploadMeasurement(int64_t meas_id) const;
      int UploadNonUploadedMeasurements();

    ////////////////////////////////////
    // Comments
    ////////////////////////////////////
    public:
      eCAL::rec::Error AddComment(int64_t meas_id, const std::string& comment);
      bool CanAddComment(int64_t meas_id) const;
      eCAL::rec::Error SimulateAddComment(int64_t meas_id) const;

    ////////////////////////////////////
    // Delete measurement
    ////////////////////////////////////
    public:
      bool CanDeleteMeasurement(int64_t meas_id) const;
      eCAL::rec::Error SimulateDeleteMeasurement(int64_t meas_id) const;
      eCAL::rec::Error DeleteMeasurement(int64_t meas_id);

    ////////////////////////////////////
    // Config Save / Load
    ////////////////////////////////////
    public:
      bool ClearConfig();
      bool SaveConfigToFile(const std::string& path) const;
      bool LoadConfigFromFile(const std::string& path);

      std::string GetLoadedConfigPath() const;
      int GetLoadedConfigVersion() const;
      int GetNativeConfigVersion() const;

    ////////////////////////////////////
    // Private implementation
    ////////////////////////////////////
    private:
      std::unique_ptr<RecServerImpl> rec_server_impl_;
    };
  }
}