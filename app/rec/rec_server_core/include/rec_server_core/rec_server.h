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

#include <list>
#include <vector>
#include <memory>
#include <chrono>
#include <set>
#include <map>
#include <string>

#include <rec_core/recorder_state.h>
#include <rec_core/record_mode.h>
#include <rec_core/job_config.h>

namespace eCAL
{
  namespace rec
  {
    class AbstractRecorder;
    class RecorderSettings;
    class EcalRec;

    class RecServer
    {
    public:
      RecServer();
      ~RecServer();

      ////////////////////////////////////
      // Instance management            //
      ////////////////////////////////////

      bool SetRecorderInstances(const std::vector<std::pair<std::string, std::set<std::string>>>& host_hostfilter_list);

      std::vector<std::pair<std::string, std::set<std::string>>> GetRecorderInstances() const;

      bool SetClientConnectionsEnabled(bool enabled);
      void InitiateConnectionShutdown();

      bool AreClientConnectionsEnabled() const;

      ////////////////////////////////////
      // Requests                       //
      ////////////////////////////////////

      std::map<std::string, std::pair<bool, std::string>> GetLastResponses();

      void WaitForPendingRequests() const;

      std::map<std::string, bool> AreRequestsPending() const;

      bool IsAnyRequestPending() const;

      ////////////////////////////////////
      // Settings                       //
      ////////////////////////////////////

      bool SetMaxPreBufferLength(std::chrono::steady_clock::duration max_pre_buffer_length);
      bool SetPreBufferingEnabled(bool pre_buffering_enabled);
      bool SetRecordMode(RecordMode record_mode);
      bool SetRecordMode(RecordMode record_mode, const std::set<std::string>& listed_topics);
      bool SetListedTopics(std::set<std::string> listed_topics);
      bool SetHostFilter(const std::string& hostname, const std::set<std::string>& host_filter);

      std::chrono::steady_clock::duration GetMaxPreBufferLength() const;
      bool GetPreBufferingEnabled() const;
      RecordMode GetRecordMode() const;
      std::set<std::string> GetListedTopics() const;
      std::set<std::string> GetHostFilter(const std::string& hostname) const;

      ////////////////////////////////////
      // Shared job config              //
      ////////////////////////////////////

      void SetMeasRootDir(std::string meas_root_dir);
      void SetMeasName(std::string meas_name);
      void SetMaxFileSizeMib(size_t max_file_size_mib);
      void SetDescription(std::string description);

      std::string GetMeasRootDir() const;
      std::string GetMeasName() const;
      size_t GetMaxFileSizeMib() const;
      std::string GetDescription() const;

      JobConfig GetHostEvaluatedJobConfig() const;

      ////////////////////////////////////
      // Commands                       //
      ////////////////////////////////////

      bool SendRequestConnectToEcal();

      bool SendRequestDisconnectFromEcal();

      bool SendRequestSavePreBufferedData();

      bool SendRequestStartRecording();

      bool SendRequestStopRecording();

      ////////////////////////////////////
      // State                          //
      ////////////////////////////////////

      bool RecordersConnectedToEcal() const;
      bool RecordersRecording() const;

      std::map<std::string, RecorderState> GetRecorderStates() const;

      bool IsLocalRecorderInstanceBusy() const;
      RecorderState GetLocalRecorderInstanceState() const;

      ////////////////////////////////////
      // Options                        //
      ////////////////////////////////////
      void  SetUsingBuiltInRecorderEnabled(bool enabled);
      bool IsUsingBuiltInRecorderEnabled() const;

    ////////////////////////////////////
    // Member Variables               //
    ////////////////////////////////////
    private:
      std::list<std::pair<std::unique_ptr<AbstractRecorder>, std::set<std::string>>> recorder_instances_;
      bool client_connections_enabled_;

      std::unique_ptr<RecorderSettings> settings_;
      JobConfig job_config_;

      bool      connected_to_ecal_;
      bool      recording_;
      JobConfig host_evaluated_config_;

      // Options
      bool use_built_in_recorder_;
      std::shared_ptr<eCAL::rec::EcalRec> ecal_rec_instance_;
    };
  }
}