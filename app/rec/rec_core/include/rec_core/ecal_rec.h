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

#include <memory>
#include <vector>
#include <chrono>
#include <map>
#include <set>
#include <thread>

#include <rec_core/recorder_state.h>
#include <rec_core/topic_info.h>

#include <rec_core/record_mode.h>
#include <rec_core/job_config.h>


namespace eCAL
{
  namespace rec
  {

    class Recorder;

    class EcalRec
    {
    public:
      EcalRec();
      ~EcalRec();

      //////////////////////////////////////
      /// Recorder control              ////
      //////////////////////////////////////

      void SetPreBufferingEnabled(bool enabled);

      void SetMaxPreBufferLength(std::chrono::steady_clock::duration max_pre_buffer_length);

      std::chrono::steady_clock::duration GetMaxPreBufferLength() const;

      bool IsPreBufferingEnabled() const;

      std::pair<size_t, std::chrono::steady_clock::duration> GetCurrentPreBufferLength() const;

      bool SavePreBufferedData(const JobConfig& job_config);

      bool StartRecording(const JobConfig& job_config);

      bool StopRecording();

      WriterState GetMainRecorderState() const;

      RecorderState GetRecorderState() const;

      //////////////////////////////////////
      /// Only record certain messages  ////
      //////////////////////////////////////

      bool SetHostFilter(const std::set<std::string>& hosts);

      bool SetRecordMode(RecordMode mode, std::set<std::string> listed_topics = {});

      bool SetListedTopics(const std::set<std::string>& listed_topics);

      std::set<std::string> GetHostsFilter() const;

      RecordMode GetRecordMode() const;

      RecordMode GetRecordMode(std::set<std::string>& listed_topics) const;

      std::set<std::string> GetListedTopics() const;

      //////////////////////////////////////
      /// eCAL                          ////
      //////////////////////////////////////

      void ConnectToEcal();

      void DisconnectFromEcal();

      bool IsConnectedToEcal() const;

      std::map<std::string, bool> GetHostsRunningEcalRpcService() const;

      std::set<std::string> GetSubscribedTopics() const;

    private:
      std::unique_ptr<Recorder> recorder_;
    };
  }
}
