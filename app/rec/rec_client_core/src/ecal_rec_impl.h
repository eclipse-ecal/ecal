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

#include <rec_client_core/state.h>
#include <rec_client_core/topic_info.h>
#include <rec_client_core/record_mode.h>
#include <rec_client_core/job_config.h>
#include <rec_client_core/upload_config.h>

#include "job/record_job.h"

#include <ecal/ecal_callback.h>
#include <ecal/ecal_subscriber.h>

namespace eCAL
{
  namespace rec
  {
    class GarbageCollectorTriggerThread;
    class MonitoringThread;
    class AddonManager;

    class EcalRecImpl
    {
    public:
      EcalRecImpl();
      ~EcalRecImpl();

      //////////////////////////////////////
      //// Recorder control             ////
      //////////////////////////////////////

      void SetPreBufferingEnabled(bool enabled);

      void SetMaxPreBufferLength(std::chrono::steady_clock::duration max_pre_buffer_length);
      std::chrono::steady_clock::duration GetMaxPreBufferLength() const;

      bool IsPreBufferingEnabled() const;
      std::pair<int64_t, std::chrono::steady_clock::duration> GetCurrentPreBufferLength() const;

      bool SavePreBufferedData(const JobConfig& job_config);

      bool StartRecording(const JobConfig& job_config);
      bool StopRecording();

      RecorderStatus GetRecorderStatus() const;

      eCAL::rec::Error UploadMeasurement(const UploadConfig& upload_config);

      bool SetEnabledAddons(const std::set<std::string>& addon_ids);
      std::set<std::string> GetEnabledAddons() const;

      eCAL::rec::Error  AddComment(int64_t job_id, const std::string& comment);

      eCAL::rec::Error DeleteMeasurement(int64_t job_id);

      //////////////////////////////////////
      //// Topic filters                ////
      //////////////////////////////////////

      bool SetHostFilter(const std::set<std::string>& hosts);

      bool SetRecordMode(RecordMode mode, const std::set<std::string>& listed_topics = {});

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

      void EcalMessageReceived(const char* topic_name, const eCAL::SReceiveCallbackData* callback_data);

      //////////////////////////////////////
      //// API for external threads     ////
      //////////////////////////////////////
      void GarbageCollect();

      void SetTopicInfo(const std::map<std::string, TopicInfo>& topic_info_map);

    //////////////////////////////////////////////////////////////////////////////
    //// Private functions                                                    ////
    //////////////////////////////////////////////////////////////////////////////
    private:
      void UpdateAndCleanSubscribers();

      std::set<std::string> FilterAvailableTopics_NoLock(const std::map<std::string, TopicInfo>& topic_info_map) const;

      void CreateNewSubscribers_NoLock(const std::set<std::string>& topic_set);
      void RemoveOldSubscribers_NoLock(const std::set<std::string>& topic_set);

      bool StopRecording_NoLock();
      std::pair<int64_t, std::chrono::steady_clock::duration> GetCurrentPreBufferLength_NoLock() const;
      Error IsAnyJobUsingPath_NoLock(const std::string& path) const;

    //////////////////////////////////////////////////////////////////////////////
    //// Member variables                                                     ////
    //////////////////////////////////////////////////////////////////////////////
    private:
      mutable std::mutex                    recorder_mutex_;

      std::unique_ptr<AddonManager>         addon_manager_;

      std::list<RecordJob>                  record_job_history_;                /**< All Record Jobs ever started. Those include "Save Buffer" jobs and "Start Recording" jobs.*/
      RecordJob*                            recording_recorder_job_;            /**< The last RecorderJob that was started by hitting "Record" and may thus be in a state still accepting new frames. */

      std::deque<std::shared_ptr<Frame>>    frame_buffer_;
      bool                                  pre_buffering_enabled_;
      std::chrono::steady_clock::duration   max_pre_buffer_length_;

      std::pair<bool, std::string>          info_;

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
    };
  }
}