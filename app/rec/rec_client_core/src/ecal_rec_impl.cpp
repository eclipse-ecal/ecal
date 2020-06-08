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

#include "ecal_rec_impl.h"

#include "rec_client_core/ecal_rec_logger.h"

#include <ecal_utils/filesystem.h>
#include <EcalParser/EcalParser.h>

#include <algorithm>

#include <garbage_collector_trigger_thread.h>
#include <monitoring_thread.h>

#include "addons/addon_manager.h"

#include <sstream>
#include <iomanip>

#ifdef WIN32
#include <process.h>
#endif // WIN32



namespace eCAL
{
  namespace rec
  {
    EcalRecImpl::EcalRecImpl()
      : addon_manager_(std::make_unique<AddonManager>([this](int64_t job_id, const std::string& addon_id, const RecAddonJobStatus& job_status)
                                                      {
                                                        std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);

                                                        // find the corresponding record job
                                                        auto record_job_it = std::find_if(record_job_history_.begin(), record_job_history_.end(), [job_id](const RecordJob& job) -> bool { return job.GetJobConfig().GetJobId() == job_id; });
                                                        if (record_job_it != record_job_history_.end())
                                                        {
                                                          record_job_it->SetAddonStatus(addon_id, job_status);
                                                        }
                                                      }))
      , recording_recorder_job_(nullptr)
      , pre_buffering_enabled_ (false)
      , max_pre_buffer_length_ (std::chrono::steady_clock::duration(0))
      , info_                  {true, ""}
      , connected_to_ecal_     (false)
      , record_mode_           (RecordMode::All)
    {
      garbage_collector_trigger_thread_ = std::make_unique<GarbageCollectorTriggerThread>(*this);
      garbage_collector_trigger_thread_->Start();

      monitoring_thread_ = std::make_unique<MonitoringThread>(*this);
      monitoring_thread_->Start();
    }

    EcalRecImpl::~EcalRecImpl()
    {
      DisconnectFromEcal();

      // Interrupt monitoring thread
      monitoring_thread_->Interrupt();
      monitoring_thread_->Join();

      // Interrupt garbage collector
      garbage_collector_trigger_thread_->Interrupt();
      garbage_collector_trigger_thread_->Join();

      {
        std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);

        // Interrupt all writer / upload threads
        for (auto& rec_job : record_job_history_)
        {
          rec_job.Interrupt();
        }
        record_job_history_.clear();
      }
    }

    //////////////////////////////////////
    /// Buffer                        ////
    //////////////////////////////////////

    void EcalRecImpl::SetPreBufferingEnabled(bool enabled)
    {
      std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);
      {
        if (!enabled)
        {
          frame_buffer_.clear();
        }
        pre_buffering_enabled_ = enabled;
      }
      addon_manager_->SetPreBuffer(pre_buffering_enabled_, max_pre_buffer_length_);
      EcalRecLogger::Instance()->info(std::string("Pre-buffering enabled: ") + (enabled ? "True" : "False"));
    }

    void EcalRecImpl::SetMaxPreBufferLength(std::chrono::steady_clock::duration max_pre_buffer_length)
    {
      {
        std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);
        max_pre_buffer_length_ = max_pre_buffer_length;
      }
      addon_manager_->SetPreBuffer(pre_buffering_enabled_, max_pre_buffer_length_);
      EcalRecLogger::Instance()->info(std::string("Max pre-buffer length: ") + std::to_string(std::chrono::duration_cast<std::chrono::duration<double>>(max_pre_buffer_length).count()) + "s");
    }

    std::chrono::steady_clock::duration EcalRecImpl::GetMaxPreBufferLength() const
    {
      std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);
      return max_pre_buffer_length_;
    }

    bool EcalRecImpl::IsPreBufferingEnabled() const
    {
      std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);

      return pre_buffering_enabled_;
    }

    std::pair<int64_t, std::chrono::steady_clock::duration> EcalRecImpl::GetCurrentPreBufferLength() const
    {
      std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);
      return GetCurrentPreBufferLength_NoLock();
    }

    bool EcalRecImpl::SavePreBufferedData(const JobConfig& job_config)
    {
      {
        // When we are not connected to eCAL, there is definitively nothing to save.
        std::lock_guard<decltype(ecal_mutex_)> ecal_lock(ecal_mutex_);
        if (!connected_to_ecal_)
          return false;
      }

      // Evaluate the Job config and check if a target path is set
      JobConfig evaluated_job_config = job_config.CreateEvaluatedJobConfig(true);
      if (evaluated_job_config.GetMeasName().empty() && evaluated_job_config.GetMeasRootDir().empty())
      {
        const std::string error_string = "Unable to save buffer: Target path not set";
        info_ = { false, error_string };
        EcalRecLogger::Instance()->error(error_string);
        return false;
      }

      // Get the topic info map from the monitor in order to also store this information in the measurement
      auto topic_info_map  = monitoring_thread_->GetTopicInfoMap();

      {
        std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);

        if (pre_buffering_enabled_)
        {
          // Check if there is alreay something recording to the given path
          std::string complete_measurement_path = evaluated_job_config.GetCompleteMeasurementPath();
          if (IsAnyJobUsingPath_NoLock(complete_measurement_path))
          {
            const std::string error_string = "Unable to save buffer: The path \"" + complete_measurement_path + "\" is currently in use";
            info_ = { false, error_string };
            EcalRecLogger::Instance()->error(error_string);
            return false;
          }

          // Write an info to the log output
          auto current_buffer_length = GetCurrentPreBufferLength_NoLock();
          double buffer_length_secs = std::chrono::duration_cast<std::chrono::duration<double>>(current_buffer_length.second).count();
          std::stringstream ss;
          ss << "Saving buffer to disk (ID: " << evaluated_job_config.GetJobId() << "). Content: " << current_buffer_length.first << " Frames / " << std::setprecision(3) << std::fixed << buffer_length_secs << " secs";
          EcalRecLogger::Instance()->info(ss.str());

          // Create the job
          record_job_history_.emplace_back(evaluated_job_config);

          if (!record_job_history_.back().InitializeMeasurementDirectory())
          {
            return false;
          }

          // Fill the addon states with empty states
          for (const auto& addon_id : addon_manager_->GetEnabledAddons())
          {
            record_job_history_.back().SetAddonStatus(addon_id, eCAL::rec::RecAddonJobStatus());
          }

          // Start the job
          if (!record_job_history_.back().SaveBuffer(topic_info_map, frame_buffer_))
          {
            const std::string error_string = "Unable to save buffer: Failed to start buffer writer thread";
            info_ = { false, error_string };
            EcalRecLogger::Instance()->error(error_string);
            return false;
          }

          // Start the addons
          addon_manager_->SaveBuffer(std::move(evaluated_job_config));

          info_ = { true, "" };

          return true;
        }
        else
        {
          const std::string error_string = "Cannot save buffer. Buffering is disabled.";
          info_ = { false, error_string };
          EcalRecLogger::Instance()->error(error_string);

          // If buffering is disabled, we return false, as we have nothing to save
          return false;
        }
      }
    }

    //////////////////////////////////////
    /// Recorder Control              ////
    //////////////////////////////////////

    bool EcalRecImpl::StartRecording(const JobConfig& job_config)
    {
      // Connect to ecal (doesn't do any harm if we are alredy connected)
      ConnectToEcal();

      // Evaluate the Job config and check if a target path is set
      JobConfig evaluated_job_config = job_config.CreateEvaluatedJobConfig(true);
      if (evaluated_job_config.GetMeasName().empty() && evaluated_job_config.GetMeasRootDir().empty())
      {
        const std::string error_message = "Unable to save buffer: Target path not set";
        info_ = { false, error_message };
        EcalRecLogger::Instance()->error(error_message);
        return false;
      }

      // Get the topic info map from the monitor in order to also store this information in the measurement
      auto topic_info_map = monitoring_thread_->GetTopicInfoMap();

      {
        std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);

        // Check if we are already recording. We prevent starting simultaneous recordings.
        if ((recording_recorder_job_ != nullptr)
          && (recording_recorder_job_->GetMainRecorderState() == JobState::Recording))
        {
          // If we are already recording, we cannot start recording, again.
          const std::string error_message = "Unable to start recording: A recording is already running";
          info_ = { false, error_message };
          EcalRecLogger::Instance()->error(error_message);
          return false;
        }

        // Check if there is alreay something recording to the given path
        std::string complete_measurement_path = evaluated_job_config.GetCompleteMeasurementPath();

        Error path_in_use_error = IsAnyJobUsingPath_NoLock(complete_measurement_path);
        if (path_in_use_error)
        {
          const std::string error_message = "Unable to start recording: " + path_in_use_error.ToString();
          info_ = { false, error_message };
          EcalRecLogger::Instance()->error(error_message);
          return false;
        }

        // Write an info to the log output
        if (pre_buffering_enabled_)
        {
          auto current_buffer_length = GetCurrentPreBufferLength_NoLock();
          double buffer_length_secs = std::chrono::duration_cast<std::chrono::duration<double>>(current_buffer_length.second).count();
          std::stringstream ss;
          ss << "Start recording (ID: " << evaluated_job_config.GetJobId() << "). Initial buffer: " << current_buffer_length.first << " Frames / " << std::setprecision(3) << std::fixed << buffer_length_secs << " secs";
          EcalRecLogger::Instance()->info(ss.str());
        }
        else
        {
          EcalRecLogger::Instance()->info("Start recording (ID: " + std::to_string(evaluated_job_config.GetJobId()) + ")");
        }

        // Create the job
        record_job_history_.emplace_back(evaluated_job_config);

        if (!record_job_history_.back().InitializeMeasurementDirectory())
        {
          return false;
        }

        // Fill the addon states with empty states
        for (const auto& addon_id : addon_manager_->GetEnabledAddons())
        {
          record_job_history_.back().SetAddonStatus(addon_id, eCAL::rec::RecAddonJobStatus());
        }

        // Start the job
        if (!record_job_history_.back().StartRecording(topic_info_map, frame_buffer_))
        {
          const std::string error_message = "Unable to start recording: Failed to start recorder thread";
          info_ = { false, error_message };
          EcalRecLogger::Instance()->error(error_message);
          return false;
        }
        recording_recorder_job_ = &(record_job_history_.back());

        // Start the addons
        addon_manager_->StartRecording(std::move(evaluated_job_config));

        info_ = { true, "" };
        return true;
      }
    }

    bool EcalRecImpl::StopRecording()
    {
      std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);

      bool success = StopRecording_NoLock();

      if (success)
      {
        EcalRecLogger::Instance()->info("Stop recording.");
      }
      else
      {
        EcalRecLogger::Instance()->error("Unable to stop recording: No recording is running");
      }

      return success;
    }

    RecorderStatus EcalRecImpl::GetRecorderStatus() const
    {
      RecorderStatus recorder_status;

      recorder_status.timestamp_ = eCAL::Time::ecal_clock::now();
#ifdef WIN32
      recorder_status.pid_       = static_cast<int>(_getpid());
#else // WIN32
      recorder_status.pid_       = static_cast<int>(getpid());
#endif // WIN32

      {
        std::lock_guard<decltype(ecal_mutex_)> ecal_lock(ecal_mutex_);

        // initialized_
        recorder_status.initialized_ = connected_to_ecal_;

        // subscribed_topics_
        for (const auto& subscriber : subscriber_map_)
        {
          recorder_status.subscribed_topics_.emplace(subscriber.first);
        }
      }

      // addon_statuses_
      recorder_status.addon_statuses_ = addon_manager_->GetAddonStatuses();

      // info_
      recorder_status.info_ = info_;
      
      {
        std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);

        // pre_buffer_length_
        recorder_status.pre_buffer_length_ = GetCurrentPreBufferLength_NoLock();

        // job_statuses_
        recorder_status.job_statuses_.reserve(record_job_history_.size());
        for (const RecordJob& job : record_job_history_)
        {
          recorder_status.job_statuses_.emplace_back(job.GetJobStatus());
        }
      }

      return recorder_status;
    }

    eCAL::rec::Error  EcalRecImpl::UploadMeasurement(const UploadConfig& upload_config)
    {
      {
        eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::OK);

        std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);

        // Look up the measurement
        auto job_it = std::find_if(record_job_history_.begin(), record_job_history_.end(), [job_id = upload_config.meas_id_](const RecordJob& job_history_entry) -> bool { return job_history_entry.GetJobConfig().GetJobId() == job_id; });

        if (job_it == record_job_history_.end())
          error = eCAL::rec::Error(eCAL::rec::Error::ErrorCode::MEAS_ID_NOT_FOUND, std::to_string(upload_config.meas_id_));

        if (!error)
        {
          error = job_it->Upload(upload_config);
        }

        if (error)
        {
          const std::string error_message = "Error uploading measurement: " + error.ToString();
          info_ = { false, error_message };
          EcalRecLogger::Instance()->error(error_message);
        }
        else
        {
          EcalRecLogger::Instance()->info("Uploading measurement " + std::to_string(upload_config.meas_id_));
        }

        return error;
      }
    }

    bool EcalRecImpl::SetEnabledAddons(const std::set<std::string>& addon_ids)
    {
      std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);
      if (recording_recorder_job_ && (recording_recorder_job_->GetMainRecorderState() == JobState::Recording))
      {
        // We block enabling / disabling addons while a recording is running
        const std::string error_message = "Failed to set enabled addons. Please stop the recording and try again.";
        info_ = { false, error_message };
        EcalRecLogger::Instance()->error(error_message);
        return false;
      }
      else
      {
        info_ = { true, "" };
        addon_manager_->SetEnabledAddons(addon_ids);
        return true;
      }
    }

    std::set<std::string> EcalRecImpl::GetEnabledAddons() const
    {
      std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);
      return addon_manager_->GetEnabledAddons();
    }

    eCAL::rec::Error EcalRecImpl::AddComment(int64_t job_id, const std::string& comment)
    {
      eCAL::rec::Error error (eCAL::rec::Error::ErrorCode::OK);

      std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);

      auto job_it = std::find_if(record_job_history_.begin(), record_job_history_.end(), [job_id](const RecordJob& job_history_entry) -> bool { return job_history_entry.GetJobConfig().GetJobId() == job_id; });

      if (job_it == record_job_history_.end())
        error = eCAL::rec::Error(eCAL::rec::Error::ErrorCode::MEAS_ID_NOT_FOUND, std::to_string(job_id));

      if (!error)
      {
        error = job_it->AddComment(comment);
      }

      if (error)
      {
        const std::string error_message = "Error adding comment: " + error.ToString();
        info_ = { false, error_message };
        EcalRecLogger::Instance()->error(error_message);
      }
      else
      {
        EcalRecLogger::Instance()->info("Adding comment to " + std::to_string(job_id));
      }

      return error;
    }

    eCAL::rec::Error EcalRecImpl::DeleteMeasurement(int64_t job_id)
    {
      Error error(Error::OK);

      {
        std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);

        auto job_it = std::find_if(record_job_history_.begin(), record_job_history_.end(), [job_id](const RecordJob& job_history_entry) -> bool { return job_history_entry.GetJobConfig().GetJobId() == job_id; });
        if (job_it == record_job_history_.end())
        {
          error = Error(Error::MEAS_ID_NOT_FOUND, std::to_string(job_id));
          info_ = { false, error.ToString() };
        }
        else
        {
          error = job_it->DeleteMeasurement();

          if (error)
          {
            EcalRecLogger::Instance()->error("Error deleting measurement: " + error.ToString());
          }
          else
          {
            EcalRecLogger::Instance()->info("Successfully deleted measurement " + std::to_string(job_id) + " from \"" + job_it->GetJobConfig().GetCompleteMeasurementPath() + "\"");
          }
        }
      }

      return error;
    }


    //////////////////////////////////////
    /// Topic filters                 ////
    //////////////////////////////////////

    bool EcalRecImpl::SetHostFilter(const std::set<std::string>& hosts)
    {
      bool success = false;

      {
        std::lock(ecal_mutex_, recorder_mutex_);
        std::lock_guard<decltype(ecal_mutex_)>     ecal_lock    (ecal_mutex_,     std::adopt_lock);
        std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_, std::adopt_lock);

        if (recording_recorder_job_ && (recording_recorder_job_->GetMainRecorderState() == JobState::Recording))
        {
          // We cannot change the host filter while a recording is running
          success = false;
        }
        else
        {
          hosts_filter_ = hosts;
          frame_buffer_.clear();
          success = true;
        }
      }

      // Log information
      if (success)
      {
        info_ = { true, "" };
        EcalRecLogger::Instance()->info(std::string("Host filter:           ") + EcalUtils::String::Join("; ", hosts));
      }
      else
      {
        const std::string error_message = "Unable to set host filter";
        info_ = { false, error_message };
        EcalRecLogger::Instance()->error(error_message);
      }

      // Update eCAL subscribers if necessary
      if (success && connected_to_ecal_)
      {
        UpdateAndCleanSubscribers();
      }

      return success;
    }

    bool EcalRecImpl::SetRecordMode(RecordMode mode, const std::set<std::string>& listed_topics)
    {
      bool success = false;

      {
        std::lock(ecal_mutex_, recorder_mutex_);
        std::lock_guard<decltype(ecal_mutex_)>     ecal_lock    (ecal_mutex_,     std::adopt_lock);
        std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_, std::adopt_lock);

        if (recording_recorder_job_ && (recording_recorder_job_->GetMainRecorderState() == JobState::Recording))
        {
          // We cannot switch the record mode while recording, as this would influence the current recording
          success = false;
        }
        else
        {
          record_mode_   = mode;
          listed_topics_ = listed_topics;
          frame_buffer_.clear();
          success = true;
        }
      }

      // Log a status
      if (success)
      {
        EcalRecLogger::Instance()->info(std::string("Record mode:           ") + (mode == RecordMode::All ? "All" : (mode == RecordMode::Blacklist ? "Blacklist" : "Whitelist")));
        if (mode != RecordMode::All)
        {
          EcalRecLogger::Instance()->info(std::string("Listed Topics:\n  ") + EcalUtils::String::Join("\n  ", listed_topics));
        }
        info_ = { true, "" };
      }
      else
      {
        const std::string error_message = "Unable to set record mode and listed topics";
        info_ = { false, error_message };
        EcalRecLogger::Instance()->error(error_message);
      }

      // Update eCAL subscribers if necessary
      if (success && connected_to_ecal_)
      {
        UpdateAndCleanSubscribers();
      }

      return success;
    }

    bool EcalRecImpl::SetListedTopics(const std::set<std::string>& listed_topics)
    {
      return SetRecordMode(record_mode_, listed_topics);
    }

    std::set<std::string> EcalRecImpl::GetHostsFilter() const
    {
      std::lock_guard<decltype(ecal_mutex_)> ecal_lock(ecal_mutex_);
      return hosts_filter_;
    }

    RecordMode EcalRecImpl::GetRecordMode() const
    {
      std::lock_guard<decltype(ecal_mutex_)> ecal_lock(ecal_mutex_);
      return record_mode_;
    }

    RecordMode EcalRecImpl::GetRecordMode(std::set<std::string>& listed_topics) const
    {
      std::lock_guard<decltype(ecal_mutex_)> ecal_lock(ecal_mutex_);
      listed_topics = listed_topics_;
      return record_mode_;
    }

    std::set<std::string> EcalRecImpl::GetListedTopics() const
    {
      std::lock_guard<decltype(ecal_mutex_)> ecal_lock(ecal_mutex_);
      return listed_topics_;
    }

    //////////////////////////////////////
    /// eCAL                          ////
    //////////////////////////////////////

    void EcalRecImpl::ConnectToEcal()
    {
      {
        std::lock_guard<decltype(ecal_mutex_)> ecal_lock(ecal_mutex_);
        if (connected_to_ecal_)
        {
          return;
        }

        EcalRecLogger::Instance()->info("Connecting to eCAL");
        auto topic_info_map = monitoring_thread_->GetTopicInfoMap();
        auto filtered_topics = FilterAvailableTopics_NoLock(topic_info_map);
        CreateNewSubscribers_NoLock(filtered_topics);
        connected_to_ecal_ = true;
      }

      addon_manager_->Initialize();
    }

    void EcalRecImpl::DisconnectFromEcal()
    {
      bool need_to_deinitialize;

      {
        std::lock_guard<decltype(ecal_mutex_)> ecal_lock(ecal_mutex_);
        need_to_deinitialize = connected_to_ecal_;
        if (connected_to_ecal_)
        {
          EcalRecLogger::Instance()->info("Disconnecting from eCAL");

          subscriber_map_.clear();
          connected_to_ecal_ = false;
        }
      }

      if (need_to_deinitialize)
        addon_manager_->Deinitialize();

      {
        std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);
        StopRecording_NoLock();
        frame_buffer_.clear();
      }
    }

    bool EcalRecImpl::IsConnectedToEcal() const
    {
      std::lock_guard<decltype(ecal_mutex_)> ecal_lock(ecal_mutex_);
      return connected_to_ecal_;
    }

    std::set<std::string> EcalRecImpl::GetSubscribedTopics() const
    {
      std::set<std::string> subscribed_topics;

      std::lock_guard<decltype(ecal_mutex_)> ecal_lock(ecal_mutex_);
      for (const auto& subscriber : subscriber_map_)
      {
        subscribed_topics.emplace(subscriber.first);
      }
      return subscribed_topics;
    }

    void EcalRecImpl::EcalMessageReceived(const char* topic_name, const eCAL::SReceiveCallbackData* callback_data)
    {
      auto ecal_receive_time   = eCAL::Time::ecal_clock::now();
      auto system_receive_time = std::chrono::steady_clock::now();

      std::shared_ptr<Frame> frame = std::make_shared<Frame>(callback_data, topic_name, ecal_receive_time, system_receive_time);

      {
        std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);
        if (pre_buffering_enabled_)
        {
          frame_buffer_.push_back(frame);
        }

        if (recording_recorder_job_ != nullptr)
        {
          recording_recorder_job_->AddFrame(std::move(frame));
        }
      }
    }

    //////////////////////////////////////
    /// API for external threads      ////
    //////////////////////////////////////
    void EcalRecImpl::GarbageCollect()
    {
      auto now = std::chrono::steady_clock::now();

      std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);

      // Clean frame buffer
      while (!frame_buffer_.empty())
      {
        if ((now - max_pre_buffer_length_) > frame_buffer_.front()->system_receive_time_)
        {
          frame_buffer_.pop_front();
        }
        else
        {
          break;
        }
      }
    }

    void EcalRecImpl::SetTopicInfo(const std::map<std::string, TopicInfo>& topic_info_map)
    {
      // Create subscribers for new topics if necessary
      {
        std::lock_guard<decltype(ecal_mutex_)> ecal_lock(ecal_mutex_);

        if (connected_to_ecal_)
        {
          auto filtered_topics = FilterAvailableTopics_NoLock(topic_info_map);
          CreateNewSubscribers_NoLock(filtered_topics);
        }
      }

      {
        // Set Topic information on the recording job
        std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);
        if (recording_recorder_job_ != nullptr)
        {
          recording_recorder_job_->SetTopicInfo(topic_info_map);
        }
      }
    }

    ////////////////////////////////////////////////////////////////////////////////
    //// Private functions                                                      ////
    ////////////////////////////////////////////////////////////////////////////////

    void EcalRecImpl::UpdateAndCleanSubscribers()
    {
      auto topic_info_map = monitoring_thread_->GetTopicInfoMap();

      std::lock_guard<decltype(ecal_mutex_)> ecal_lock(ecal_mutex_);

      if (!connected_to_ecal_)
        return;

      auto filtered_topic_set = FilterAvailableTopics_NoLock(topic_info_map);
      CreateNewSubscribers_NoLock(filtered_topic_set);
      RemoveOldSubscribers_NoLock(filtered_topic_set);
    }

    std::set<std::string> EcalRecImpl::FilterAvailableTopics_NoLock(const std::map<std::string, TopicInfo>& topic_info_map) const
    {
      std::set<std::string> topic_set;

      for (const auto& topic_info : topic_info_map)
      {
        if (topic_info.second.publishers_.empty())
        {
          // Nobody is publishing this channel
          continue;
        }

        // Evaluate the record mode (All / Blacklist / Whitelist)
        if ((record_mode_ == RecordMode::Blacklist)
          && (listed_topics_.find(topic_info.first) != listed_topics_.end()))
        {
          // The topic is blacklisted
          continue;
        }
        else if ((record_mode_ == RecordMode::Whitelist)
          && (listed_topics_.find(topic_info.first) == listed_topics_.end()))
        {
          // The topic is not whitelisted
          continue;
        }

        if (!hosts_filter_.empty())
        {
          bool host_filter_matches = false;
          for (const auto& publisher : topic_info.second.publishers_)
          {
            if (hosts_filter_.find(publisher.first) != hosts_filter_.end())
            {
              host_filter_matches = true;
              break;
            }
          }

          if (!host_filter_matches)
          {
            // The user set a hosts filter and it doesn't match
            continue;
          }
        }

        // Add the topic to the filtered set if we haven't found any reason not to do that :)
        topic_set.emplace(topic_info.first);
      }

      return topic_set;
    }

    void EcalRecImpl::CreateNewSubscribers_NoLock(const std::set<std::string>& topic_set)
    {
      for (const std::string& topic : topic_set)
      {
        if (subscriber_map_.find(topic) == subscriber_map_.end())
        {
          EcalRecLogger::Instance()->info("Subscribing to " + topic);
          std::unique_ptr<eCAL::CSubscriber> subscriber = std::make_unique<eCAL::CSubscriber>();
          if (!subscriber->Create(topic))
          {
            EcalRecLogger::Instance()->error("Error creating subscriber for topic " + topic);
            info_ = { false, "Error creating eCAL subsribers" };
            continue;
          }
          if (!subscriber->AddReceiveCallback(std::bind(&EcalRecImpl::EcalMessageReceived, this, std::placeholders::_1, std::placeholders::_2)))
          {
            EcalRecLogger::Instance()->error("Error adding callback for subscriber on topic " + topic);
            info_ = { false, "Error creating eCAL subsribers" };
            continue;
          }
          subscriber_map_.emplace(topic, std::move(subscriber));
        }
      }
    }

    void EcalRecImpl::RemoveOldSubscribers_NoLock(const std::set<std::string>& topic_set)
    {
      auto subscriber_it = subscriber_map_.begin();
      while (subscriber_it != subscriber_map_.end())
      {
        if (topic_set.find(subscriber_it->first) == topic_set.end())
        {
          EcalRecLogger::Instance()->info("Unsubscribing from " + subscriber_it->first);

          subscriber_it->second->Destroy();
          subscriber_it = subscriber_map_.erase(subscriber_it);
        }
        else
        {
          subscriber_it++;
        }
      }
    }

    bool EcalRecImpl::StopRecording_NoLock()
    {
      if ((recording_recorder_job_ == nullptr)
        || (recording_recorder_job_->GetMainRecorderState() != JobState::Recording))
      {
        // If we are not recording, we cannot stop it!
        return false;
      }
      else
      {
        // Stop eCAL Recorder
        bool success = recording_recorder_job_->StopRecording();

        // Stop recorder addons
        addon_manager_->StopRecording();

        recording_recorder_job_ = nullptr;
        return success;
      }
    }

    std::pair<int64_t, std::chrono::steady_clock::duration> EcalRecImpl::GetCurrentPreBufferLength_NoLock() const
    {
      int64_t frame_count = frame_buffer_.size();
      std::chrono::steady_clock::duration buffer_length;
      if (frame_count > 0)
      {
        buffer_length = std::chrono::steady_clock::now() - frame_buffer_.front()->system_receive_time_;
      }
      else
      {
        buffer_length = std::chrono::steady_clock::duration(0);
      }
      return std::make_pair(frame_count, buffer_length);
    }

    Error EcalRecImpl::IsAnyJobUsingPath_NoLock(const std::string& path) const
    {
      for (const auto& rec_job : record_job_history_)
      {
        if (!rec_job.IsDeleted()
          && (rec_job.GetMainRecorderState() != JobState::NotStarted)
          && EcalUtils::Filesystem::IsEqual(path, rec_job.GetJobConfig().GetCompleteMeasurementPath()))
        {
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::DIR_NOT_EMPTY, "\"" + path + "\" is used by measurement " + std::to_string(rec_job.GetJobConfig().GetJobId()));
        }
      }
      return eCAL::rec::Error::ErrorCode::OK;
    }
  }
}
