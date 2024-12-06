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

#include "rec_client_core/ecal_rec.h"

#include "ecal_rec_impl.h"
#include "rec_client_core/ecal_rec_logger.h"

#include "EcalParser/EcalParser.h"

#include <iostream>
#include <fstream>

#include <ecal_utils/filesystem.h>

namespace eCAL
{
  namespace rec
  {

    EcalRec::EcalRec()
    {
      eCAL::Initialize(0, nullptr, "eCALRecClient", eCAL::Init::Default | eCAL::Init::Monitoring);
      eCAL::Monitoring::SetFilterState(false);

      recorder_ = std::make_unique<EcalRecImpl>();
    }

    EcalRec::~EcalRec()
    {
      eCAL::Finalize();
    }

    //////////////////////////////////////
    /// Recorder control              ////
    //////////////////////////////////////

    void EcalRec::SetPreBufferingEnabled(bool enabled)
    {
      recorder_->SetPreBufferingEnabled(enabled);
    }

    void EcalRec::SetMaxPreBufferLength(std::chrono::steady_clock::duration max_pre_buffer_length)
    {
      recorder_->SetMaxPreBufferLength(max_pre_buffer_length);
    }

    std::chrono::steady_clock::duration EcalRec::GetMaxPreBufferLength() const
    {
      return recorder_->GetMaxPreBufferLength();
    }

    bool EcalRec::IsPreBufferingEnabled() const
    {
      return recorder_->IsPreBufferingEnabled();
    }

    std::pair<size_t, std::chrono::steady_clock::duration> EcalRec::GetCurrentPreBufferLength() const
    {
      return recorder_->GetCurrentPreBufferLength();
    }

    bool EcalRec::SavePreBufferedData(const JobConfig& job_config)
    {
      return recorder_->SavePreBufferedData(job_config);
    }

    bool EcalRec::StartRecording(const JobConfig& job_config)
    {
      return recorder_->StartRecording(job_config);
    }

    bool EcalRec::StopRecording()
    {
      return recorder_->StopRecording();
    }

    RecorderStatus EcalRec::GetRecorderStatus() const
    {
      return recorder_->GetRecorderStatus();
    }

    eCAL::rec::Error EcalRec::UploadMeasurement(const UploadConfig& upload_config)
    {
      return recorder_->UploadMeasurement(upload_config);
    }

    bool EcalRec::SetEnabledAddons(const std::set<std::string>& addon_ids)
    {
      return recorder_->SetEnabledAddons(addon_ids);
    }

    std::set<std::string> EcalRec::GetEnabledAddons() const
    {
      return recorder_->GetEnabledAddons();
    }

    eCAL::rec::Error EcalRec::AddComment(int64_t job_id, const std::string& comment)
    {
      return recorder_->AddComment(job_id, comment);
    }

    Error EcalRec::DeleteMeasurement(int64_t job_id)
    {
      return recorder_->DeleteMeasurement(job_id);
    }

    //////////////////////////////////////
    /// Only record certain messages  ////
    //////////////////////////////////////

    bool EcalRec::SetHostFilter(const std::set<std::string>& hosts)
    {
      return recorder_->SetHostFilter(hosts);
    }

    bool EcalRec::SetRecordMode(RecordMode mode, const std::set<std::string>& listed_topics)
    {
      return recorder_->SetRecordMode(mode, listed_topics);
    }
    
    bool EcalRec::SetListedTopics(const std::set<std::string>& listed_topics)
    {
      return recorder_->SetListedTopics(listed_topics);
    }

    std::set<std::string> EcalRec::GetHostsFilter() const
    {
      return recorder_->GetHostsFilter();
    }

    RecordMode EcalRec::GetRecordMode() const
    {
      return recorder_->GetRecordMode();
    }

    RecordMode EcalRec::GetRecordMode(std::set<std::string>& listed_topics) const
    {
      return recorder_->GetRecordMode(listed_topics);
    }

    std::set<std::string> EcalRec::GetListedTopics() const
    {
      return recorder_->GetListedTopics();
    }

    //////////////////////////////////////
    /// eCAL                          ////
    //////////////////////////////////////

    void EcalRec::ConnectToEcal()
    {
      recorder_->ConnectToEcal();
    }

    void EcalRec::DisconnectFromEcal()
    {
      recorder_->DisconnectFromEcal();
    }

    bool EcalRec::IsConnectedToEcal() const
    {
      return recorder_->IsConnectedToEcal();
    }

    std::set<std::string> EcalRec::GetSubscribedTopics() const
    {
      return recorder_->GetSubscribedTopics();
    }
  }
}