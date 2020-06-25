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

#include "rec_server_impl.h"

namespace eCAL
{
  namespace rec_server
  {
    ////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////
    RecServer::RecServer() : rec_server_impl_(std::make_unique<RecServerImpl>()) {}
    RecServer::~RecServer()                                                      {}

    ////////////////////////////////////
    // Client management
    ////////////////////////////////////
    bool RecServer::SetEnabledRecClients(const std::map<std::string, ClientConfig>& enabled_rec_clients) { return rec_server_impl_->SetEnabledRecClients(enabled_rec_clients); }
    std::map<std::string, ClientConfig> RecServer::GetEnabledRecClients() const                          { return rec_server_impl_->GetEnabledRecClients(); }

    bool RecServer::SetHostFilter(const std::string& hostname, const std::set<std::string>& host_filter) { return rec_server_impl_->SetHostFilter(hostname, host_filter); }
    std::set<std::string> RecServer::GetHostFilter(const std::string& hostname) const                    { return rec_server_impl_->GetHostFilter(hostname); }

    bool RecServer::SetConnectionToClientsActive(bool active)                                            { return rec_server_impl_->SetConnectionToClientsActive(active); }
    bool RecServer::IsConnectionToClientsActive() const                                                  { return rec_server_impl_->IsConnectionToClientsActive(); }

    ////////////////////////////////////
    // Recorder control
    ////////////////////////////////////
    bool RecServer::ConnectToEcal      ()                                { return rec_server_impl_->ConnectToEcal(); }
    bool RecServer::DisconnectFromEcal ()                                { return rec_server_impl_->DisconnectFromEcal(); }
    bool RecServer::SavePreBufferedData()                                { return rec_server_impl_->SavePreBufferedData(); }
    bool RecServer::StartRecording     ()                                { return rec_server_impl_->StartRecording(); }
    bool RecServer::StopRecording      ()                                { return rec_server_impl_->StopRecording(); }

    bool RecServer::IsConnectedToEcal             () const               { return rec_server_impl_->IsConnectedToEcal(); }
    bool RecServer::IsRecording                   () const               { return rec_server_impl_->IsRecording(); }
    int64_t RecServer::GetCurrentlyRecordingMeasId() const               { return rec_server_impl_->GetCurrentlyRecordingMeasId(); }

    bool                  RecServer::IsAnyRequestPending        () const { return rec_server_impl_->IsAnyRequestPending(); }
    std::set<std::string> RecServer::GetHostsWithPendingRequests() const { return rec_server_impl_->GetHostsWithPendingRequests(); }
    void                  RecServer::WaitForPendingRequests     () const { rec_server_impl_->WaitForPendingRequests(); }

    ////////////////////////////////////
    // Status
    ////////////////////////////////////
    eCAL::rec_server::RecorderStatusMap_T RecServer::GetRecorderStatuses() const  { return rec_server_impl_->GetRecorderStatuses(); }
    eCAL::rec::RecorderStatus RecServer::GetBuiltInRecorderInstanceStatus() const { return rec_server_impl_->GetBuiltInRecorderInstanceStatus(); }

    TopicInfoMap_T RecServer::GetTopicInfo() const                                { return rec_server_impl_->GetTopicInfo(); }
    HostsRunningEcalRec_T RecServer::GetHostsRunningEcalRec() const               { return rec_server_impl_->GetHostsRunningEcalRec(); }

    std::list<eCAL::rec_server::JobHistoryEntry> RecServer::GetJobHistory() const { return rec_server_impl_->GetJobHistory(); }

    RecServerStatus RecServer::GetStatus() const                                  { return rec_server_impl_->GetStatus(); }

    ////////////////////////////////////
    // General Client Settings
    ////////////////////////////////////
    void RecServer::SetMaxPreBufferLength (std::chrono::steady_clock::duration max_pre_buffer_length)                     { rec_server_impl_->SetMaxPreBufferLength(max_pre_buffer_length); }
    void RecServer::SetPreBufferingEnabled(bool pre_buffering_enabled)                                                    { rec_server_impl_->SetPreBufferingEnabled(pre_buffering_enabled); }
    bool RecServer::SetRecordMode         (eCAL::rec::RecordMode record_mode)                                             { return rec_server_impl_->SetRecordMode(record_mode); }
    bool RecServer::SetRecordMode         (eCAL::rec::RecordMode record_mode, const std::set<std::string>& listed_topics) { return rec_server_impl_->SetRecordMode(record_mode, listed_topics); }
    bool RecServer::SetListedTopics       (std::set<std::string> listed_topics)                                           { return rec_server_impl_->SetListedTopics(listed_topics); }

    std::chrono::steady_clock::duration RecServer::GetMaxPreBufferLength () const                                         { return rec_server_impl_->GetMaxPreBufferLength(); }
    bool                                RecServer::GetPreBufferingEnabled() const                                         { return rec_server_impl_->GetPreBufferingEnabled(); }
    eCAL::rec::RecordMode               RecServer::GetRecordMode         () const                                         { return rec_server_impl_->GetRecordMode(); }
    std::set<std::string>               RecServer::GetListedTopics       () const                                         { return rec_server_impl_->GetListedTopics(); }

    ////////////////////////////////////
    // Job Settings
    ////////////////////////////////////
    void RecServer::SetMeasRootDir    (std::string meas_root_dir)       { rec_server_impl_->SetMeasRootDir(meas_root_dir); }
    void RecServer::SetMeasName       (std::string meas_name)           { rec_server_impl_->SetMeasName(meas_name); }
    void RecServer::SetMaxFileSizeMib (unsigned int max_file_size_mib)  { rec_server_impl_->SetMaxFileSizeMib(max_file_size_mib); }
    void RecServer::SetDescription    (std::string description)         { rec_server_impl_->SetDescription(description); }

    std::string  RecServer::GetMeasRootDir   () const                   { return rec_server_impl_->GetMeasRootDir(); }
    std::string  RecServer::GetMeasName      () const                   { return rec_server_impl_->GetMeasName(); }
    unsigned int RecServer::GetMaxFileSizeMib() const                   { return rec_server_impl_->GetMaxFileSizeMib(); }
    std::string  RecServer::GetDescription   () const                   { return rec_server_impl_->GetDescription(); }

    ////////////////////////////////////
    // Server Settings
    ////////////////////////////////////
    void RecServer::SetMonitoringUpdateCallback(PostUpdateCallback_T post_update_callback) { rec_server_impl_->SetMonitoringUpdateCallback(post_update_callback); }

    bool RecServer::SetUsingBuiltInRecorderEnabled(bool enabled)                           { return rec_server_impl_->SetUsingBuiltInRecorderEnabled(enabled); }
    bool RecServer::IsUsingBuiltInRecorderEnabled() const                                  { return rec_server_impl_->IsUsingBuiltInRecorderEnabled(); }

    ////////////////////////////////////
    // Measurement Upload
    ////////////////////////////////////
    void RecServer::SetUploadConfig(const UploadConfig& upload_config)           { rec_server_impl_->SetUploadConfig(upload_config); }
    UploadConfig RecServer::GetUploadConfig()  const                             { return rec_server_impl_->GetUploadConfig(); }
    int RecServer::GetInternalFtpServerOpenConnectionCount() const               { return rec_server_impl_->GetInternalFtpServerOpenConnectionCount(); }
    uint16_t RecServer::GetInternalFtpServerPort() const                         { return rec_server_impl_->GetInternalFtpServerPort(); }

    eCAL::rec::Error RecServer::UploadMeasurement(int64_t meas_id)               { return rec_server_impl_->UploadMeasurement(meas_id); }
    bool RecServer::CanUploadMeasurement(int64_t meas_id) const                  { return rec_server_impl_->CanUploadMeasurement(meas_id); };
    eCAL::rec::Error RecServer::SimulateUploadMeasurement(int64_t meas_id) const { return rec_server_impl_->SimulateUploadMeasurement(meas_id); };

    int RecServer::UploadNonUploadedMeasurements()                               { return rec_server_impl_->UploadNonUploadedMeasurements(); };

    ////////////////////////////////////
    // Comments
    ////////////////////////////////////
    eCAL::rec::Error RecServer::AddComment(int64_t meas_id, const std::string& comment) { return rec_server_impl_->AddComment(meas_id, comment); }
    bool RecServer::CanAddComment(int64_t meas_id) const                                { return rec_server_impl_->CanAddComment(meas_id); }
    eCAL::rec::Error RecServer::SimulateAddComment(int64_t meas_id) const               { return rec_server_impl_->SimulateAddComment(meas_id); }

    ////////////////////////////////////
    // Delete measurement
    ////////////////////////////////////
    bool RecServer::CanDeleteMeasurement(int64_t meas_id) const                   { return rec_server_impl_->CanDeleteMeasurement(meas_id); }
    eCAL::rec::Error RecServer::SimulateDeleteMeasurement(int64_t meas_id) const  { return rec_server_impl_->SimulateDeleteMeasurement(meas_id); }
    eCAL::rec::Error RecServer::DeleteMeasurement(int64_t meas_id)                { return rec_server_impl_->DeleteMeasurement(meas_id); }

    ////////////////////////////////////
    // Config Save / Load
    ////////////////////////////////////
    bool RecServer::ClearConfig       ()                              { return rec_server_impl_->ClearConfig(); }
    bool RecServer::SaveConfigToFile  (const std::string& path) const { return rec_server_impl_->SaveConfigToFile(path); }
    bool RecServer::LoadConfigFromFile(const std::string& path)       { return rec_server_impl_->LoadConfigFromFile(path); }

    std::string RecServer::GetLoadedConfigPath() const                { return rec_server_impl_->GetLoadedConfigPath(); }
    int RecServer::GetLoadedConfigVersion() const                     { return rec_server_impl_->GetLoadedConfigVersion(); }
    int RecServer::GetNativeConfigVersion() const                     { return rec_server_impl_->GetNativeConfigVersion(); }
  }
}