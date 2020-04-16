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

#include "frame.h"

#include <chrono>
#include <deque>
#include <mutex>
#include <list>
#include <map>
#include <set>
#include <thread>

#include <ecal_utils/ecal_utils.h>

#include "rec_core/recorder_state.h"
#include "rec_core/topic_info.h"
#include "rec_core/record_mode.h"
#include "rec_core/job_config.h"

namespace eCAL
{
  namespace rec
  {
    class Hdf5WriterThread;
    class GarbageCollectorTriggerThread;
    class MonitoringThread;

    class Recorder
    {
    public:
      Recorder();
      ~Recorder();

      //////////////////////////////////////
      //// Recorder control             ////
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
      //// Topic filters                ////
      //////////////////////////////////////

      bool SetHostFilter(const std::set<std::string>& hosts);

      bool SetRecordMode(RecordMode mode, std::set<std::string> listed_topics = {});

      bool SetListedTopics(const std::set<std::string>& listed_topics);

      std::set<std::string> GetHostsFilter() const;

      RecordMode GetRecordMode() const;

      RecordMode GetRecordMode(std::set<std::string>& listed_topics) const;

      std::set<std::string> GetListedTopics() const;

      //////////////////////////////////////
      //// eCAL                         ////
      //////////////////////////////////////

      void ConnectToEcal();

      void DisconnectFromEcal();

      bool IsConnectedToEcal() const;

      std::set<std::string> GetSubscribedTopics() const;

      std::map<std::string, bool> GetHostsRunningEcalRpcService() const;

      void EcalMessageReceived(const char* topic_name, const eCAL::SReceiveCallbackData* callback_data);

      //////////////////////////////////////
      //// API for external threads     ////
      //////////////////////////////////////
      void GarbageCollect();

      void SetTopicInfo(const std::map<std::string, TopicInfo>& topic_info_map);

    private:
      mutable std::mutex                    recorder_mutex_;

      std::unique_ptr<Hdf5WriterThread>            main_recorder_thread_;
      std::list<std::unique_ptr<Hdf5WriterThread>> buffer_writer_threads_;

      std::deque<std::shared_ptr<Frame>>    frame_buffer_;
      bool                                  pre_buffering_enabled_;
      std::chrono::steady_clock::duration   max_pre_buffer_length_;

      // Threads
      std::unique_ptr<GarbageCollectorTriggerThread> garbage_collector_trigger_thread_; /** frame_buffer_, buffer_writer_threads_, max_pre_buffer_length_ */
      std::unique_ptr<MonitoringThread>              monitoring_thread_;                /** connected_to_ecal_, FilterAvailableTopics_NoLock(hosts_filter_, topic_whitelist_, topic_blacklist_), CreateNewSubscribers_NoLock(subscriber_map_), main_writer_thread_, buffer_writer_threads_ */

      // eCAL subscribers
      mutable std::mutex                                        ecal_mutex_;
      bool                                                      connected_to_ecal_;
      std::map<std::string, std::unique_ptr<eCAL::CSubscriber>> subscriber_map_;    /**< buffering_enabled_, frame_buffer_, main_writer_thread_*/
      RecordMode                                                record_mode_;       /**< All / Blacklist / Whitelist */
      std::set<std::string>                                     listed_topics_;     /**< When in Blacklist or Whitelist mode, this list holds the according topic list */
      std::set<std::string>                                     hosts_filter_;      /**< Only subscribe to topics published by there hosts*/

      //////////////////////////////////////////////////////////////////////////////
      //// Private functions                                                    ////
      //////////////////////////////////////////////////////////////////////////////

      void UpdateAndCleanSubscribers();

      std::set<std::string> FilterAvailableTopics_NoLock(const std::map<std::string, TopicInfo>& topic_info_map) const;

      void CreateNewSubscribers_NoLock(const std::set<std::string>& topic_set);
      void RemoveOldSubscribers_NoLock(const std::set<std::string>& topic_set);

      bool StopRecording_NoLock();
      std::pair<size_t, std::chrono::steady_clock::duration> GetCurrentPreBufferLength_NoLock() const;
      bool IsAnythingSavingToPath_NoLock(const std::string& path) const;
    };
  }
}