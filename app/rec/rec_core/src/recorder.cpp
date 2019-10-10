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

#include "recorder.h"

#include "rec_core/ecal_rec_logger.h"

#include <EcalUtils/Path.h>

#include <algorithm>

#include <hdf5_writer_thread.h>
#include <garbage_collector_trigger_thread.h>
#include <monitoring_thread.h>

#include <sstream>
#include <iomanip>


namespace eCAL
{
  namespace rec
  {
    Recorder::Recorder()
      : pre_buffering_enabled_(false)
      , connected_to_ecal_(false)
      , record_mode_(RecordMode::All)
    {
      main_recorder_thread_ = std::make_unique<Hdf5WriterThread>(JobConfig());

      garbage_collector_trigger_thread_ = std::make_unique<GarbageCollectorTriggerThread>(*this);
      garbage_collector_trigger_thread_->Start();

      monitoring_thread_ = std::make_unique<MonitoringThread>(*this);
      monitoring_thread_->Start();
    }

    Recorder::~Recorder()
    {
      DisconnectFromEcal();

      // Interrupt monitoring thread
      monitoring_thread_->Interrupt();
      monitoring_thread_->Join();

      // Interrupt garbage collector
      garbage_collector_trigger_thread_->Interrupt();
      garbage_collector_trigger_thread_->Join();

      std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);
      {
        // Interrupt all writer threads
        if (main_recorder_thread_)
        {
          main_recorder_thread_->Interrupt();
        }
        for (const auto& buffer_writer : buffer_writer_threads_)
        {
          buffer_writer->Interrupt();
        }

        if (main_recorder_thread_)
        {
          main_recorder_thread_->Join();
        }
        for (const auto& buffer_writer : buffer_writer_threads_)
        {
          buffer_writer->Join();
        }
      }
    }

    //////////////////////////////////////
    /// Buffer                        ////
    //////////////////////////////////////

    void Recorder::SetPreBufferingEnabled(bool enabled)
    {
      std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);
      if (!enabled)
      {
        frame_buffer_.clear();
      }
      pre_buffering_enabled_ = enabled;
      EcalRecLogger::Instance()->info(std::string("Pre-buffering enabled: ") + (enabled ? "True" : "False"));
    }

    void Recorder::SetMaxPreBufferLength(std::chrono::steady_clock::duration max_pre_buffer_length)
    {
      std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);
      max_pre_buffer_length_ = max_pre_buffer_length;
      EcalRecLogger::Instance()->info(std::string("Max pre-buffer length: ") + std::to_string(std::chrono::duration_cast<std::chrono::duration<double>>(max_pre_buffer_length).count()) + "s");
    }

    std::chrono::steady_clock::duration Recorder::GetMaxPreBufferLength() const
    {
      std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);
      return max_pre_buffer_length_;
    }

    bool Recorder::IsPreBufferingEnabled() const
    {
      std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);

      return pre_buffering_enabled_;
    }

    std::pair<size_t, std::chrono::steady_clock::duration> Recorder::GetCurrentPreBufferLength() const
    {
      std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);
      return GetCurrentPreBufferLength_NoLock();
    }

    bool Recorder::SavePreBufferedData(const JobConfig& job_config)
    {
      {
        // When we are not connected to eCAL, there is definitively nothing to save.
        std::lock_guard<decltype(ecal_mutex_)> ecal_lock(ecal_mutex_);
        if (!connected_to_ecal_)
          return false;
      }

      if (job_config.GetMeasName().empty() && job_config.GetMeasRootDir().empty())
      {
        EcalRecLogger::Instance()->error("Unable to save buffer: Target path not set");
        return false;
      }

      // Get the topic info map from the monitor in order to also store this information in the measurement
      auto topic_info_map = monitoring_thread_->GetTopicInfoMap();

      std::string hostname = eCAL::Process::GetHostName();

      {
        std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);

        if (pre_buffering_enabled_)
        {
          auto buffer_writer_thread = std::make_unique<Hdf5WriterThread>(job_config, topic_info_map, frame_buffer_);

          // Check if there is alreay something recording to the given path
          std::string complete_measurement_path = buffer_writer_thread->GetCompleteMeasurementPath();
          if (IsAnythingSavingToPath_NoLock(complete_measurement_path))
          {
            EcalRecLogger::Instance()->error("Unable to save buffer: The path \"" + complete_measurement_path + "\" is alreay in use");
            return false;
          }

          // We don't want to record anything, so we give the flushing command before even starting the thread
          buffer_writer_thread->Flush(); 

          // Write an info to the log output
          auto current_buffer_length = GetCurrentPreBufferLength_NoLock();
          double buffer_length_secs = std::chrono::duration_cast<std::chrono::duration<double>>(current_buffer_length.second).count();
          std::stringstream ss;
          ss << "Saving buffer to disk (" << current_buffer_length.first << " Frames / " << std::setprecision(3) << std::fixed << buffer_length_secs << " secs)";
          EcalRecLogger::Instance()->info(ss.str());

          // Start the thread
          if (!buffer_writer_thread->Start())
          {
            EcalRecLogger::Instance()->error("Unable to save buffer: Failed to start buffer writer thread");
            return false;
          }

          buffer_writer_threads_.push_back(std::move(buffer_writer_thread));

          return true;
        }
        else
        {
          EcalRecLogger::Instance()->warn("Cannot save buffer. Buffering is disabled.");

          // If buffering is disabled, we return false, as we have nothing to save
          return false;
        }
      }
    }

    //////////////////////////////////////
    /// Recorder Control              ////
    //////////////////////////////////////

    bool Recorder::StartRecording(const JobConfig& job_config)
    {
      if (job_config.GetMeasName().empty() && job_config.GetMeasRootDir().empty())
      {
        EcalRecLogger::Instance()->error("Unable to start recording: Target path not set");
        return false;
      }

      auto topic_info_map = monitoring_thread_->GetTopicInfoMap();

      // Connect to ecal (doesn't do any harm if we are alredy connected)
      ConnectToEcal();

      std::string hostname = eCAL::Process::GetHostName();

      {
        std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);

        if (main_recorder_thread_->IsRunning())
        {
          EcalRecLogger::Instance()->error("Unable to start recording: A recording is already running");
          // If we are already recording (or flushing), we cannot start recording, again.
          return false;
        }
        else
        {
          if (pre_buffering_enabled_)
          {
            auto current_buffer_length = GetCurrentPreBufferLength_NoLock();
            double buffer_length_secs = std::chrono::duration_cast<std::chrono::duration<double>>(current_buffer_length.second).count();
            std::stringstream ss;
            ss << "Start recording with initial buffer of " << current_buffer_length.first << " Frames / " << std::setprecision(3) << std::fixed << buffer_length_secs << " secs";
            EcalRecLogger::Instance()->info(ss.str());
          }
          else
          {
            EcalRecLogger::Instance()->info("Start recording");
          }

          std::unique_ptr<Hdf5WriterThread> new_main_recorder_thread = std::make_unique<Hdf5WriterThread>(job_config, topic_info_map, frame_buffer_);

          // Check if there is alreay something recording to the given path
          std::string complete_measurement_path = new_main_recorder_thread->GetCompleteMeasurementPath();
          if (IsAnythingSavingToPath_NoLock(complete_measurement_path))
          {
            EcalRecLogger::Instance()->error("Unable to start recording: The path \"" + complete_measurement_path + "\" is alreay in use");
            return false;
          }

          main_recorder_thread_->Join();
          main_recorder_thread_ = std::move(new_main_recorder_thread);

          if (!main_recorder_thread_->Start())
          {
            EcalRecLogger::Instance()->error("Unable to start recording: Failed to start recorder thread");
            return false;
          }
          return true;
        }
      }
    }

    bool Recorder::StopRecording()
    {
      std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);
      auto main_recorder_state = main_recorder_thread_->GetState();
      if (main_recorder_state.recording_ && !main_recorder_state.flushing_)
      {
        EcalRecLogger::Instance()->error("Stop recording.");
        main_recorder_thread_->Flush();
        return true;
      }
      else
      {
        EcalRecLogger::Instance()->error("Unable to stop recording: No recording is running");
        return false;
      }
    }

    WriterState Recorder::GetMainRecorderState() const
    {
      std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);
      return main_recorder_thread_->GetState();
    }

    RecorderState Recorder::GetRecorderState() const
    {
      RecorderState state;

      {
        std::lock_guard<decltype(ecal_mutex_)> ecal_lock(ecal_mutex_);

        // Initialized
        state.initialized_ = connected_to_ecal_;

        // Subscribed Topics
        for (const auto& subscriber : subscriber_map_)
        {
          state.subscribed_topics_.emplace(subscriber.first);
        }
      }

      {
        std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);

        // Main Recorder State
        state.main_recorder_state_ = main_recorder_thread_->GetState();

        // Buffer Length
        state.pre_buffer_length_ = GetCurrentPreBufferLength_NoLock();

        // Buffer Writer states
        for (const auto& buffer_writer : buffer_writer_threads_)
        {
          state.buffer_writers_.push_back(buffer_writer->GetState());
        }
      }

      return state;
    }

    //////////////////////////////////////
    /// Topic filters                 ////
    //////////////////////////////////////

    bool Recorder::SetHostFilter(const std::set<std::string>& hosts)
    {
      bool success = false;

      {
        std::lock(ecal_mutex_, recorder_mutex_);
        std::lock_guard<decltype(ecal_mutex_)>     ecal_lock    (ecal_mutex_,     std::adopt_lock);
        std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_, std::adopt_lock);

        auto recording_status = main_recorder_thread_->GetState();
        if (recording_status.recording_ && !recording_status.flushing_)
        {
          success = false;
        }
        else
        {
          hosts_filter_ = hosts;
          frame_buffer_.clear();
          success = true;
        }

        if (!connected_to_ecal_)
          success = true;
      }

      // Log information
      if (success)
        EcalRecLogger::Instance()->info(std::string("Host filter:           ") + EcalUtils::String::Join("; ", hosts));
      else
        EcalRecLogger::Instance()->error(std::string("Unable to set host filter"));

      // Update eCAL subscribers if necessary
      if (success && connected_to_ecal_)
      {
        UpdateAndCleanSubscribers();
      }

      return true;
    }

    bool Recorder::SetRecordMode(RecordMode mode, std::set<std::string> listed_topics)
    {
      bool success = false;

      {
        std::lock(ecal_mutex_, recorder_mutex_);
        std::lock_guard<decltype(ecal_mutex_)>     ecal_lock(ecal_mutex_, std::adopt_lock);
        std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_, std::adopt_lock);

        auto recording_status = main_recorder_thread_->GetState();

        if ((record_mode_ == mode)
          && ((mode == RecordMode::All) || (listed_topics_ == listed_topics)))
        {
          // Everything stays the same, nothing to change
          success = true;
        }
        else if (recording_status.recording_ && !recording_status.flushing_)
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
      }
      else
      {
        EcalRecLogger::Instance()->error(std::string("Unable to set record mode and listed topics"));
      }

      // Update eCAL subscribers if necessary
      if (success && connected_to_ecal_)
      {
        UpdateAndCleanSubscribers();
      }

      return success;
    }

    bool Recorder::SetListedTopics(const std::set<std::string>& listed_topics)
    {
      return SetRecordMode(record_mode_, listed_topics);
    }

    std::set<std::string> Recorder::GetHostsFilter() const
    {
      std::lock_guard<decltype(ecal_mutex_)> ecal_lock(ecal_mutex_);
      return hosts_filter_;
    }

    RecordMode Recorder::GetRecordMode() const
    {
      std::lock_guard<decltype(ecal_mutex_)> ecal_lock(ecal_mutex_);
      return record_mode_;
    }

    RecordMode Recorder::GetRecordMode(std::set<std::string>& listed_topics) const
    {
      std::lock_guard<decltype(ecal_mutex_)> ecal_lock(ecal_mutex_);
      listed_topics = listed_topics_;
      return record_mode_;
    }

    std::set<std::string> Recorder::GetListedTopics() const
    {
      std::lock_guard<decltype(ecal_mutex_)> ecal_lock(ecal_mutex_);
      return listed_topics_;
    }

    //////////////////////////////////////
    /// eCAL                          ////
    //////////////////////////////////////

    void Recorder::ConnectToEcal()
    {

      std::lock_guard<decltype(ecal_mutex_)> ecal_lock(ecal_mutex_);
      if (!connected_to_ecal_)
      {
        EcalRecLogger::Instance()->info("Connecting to eCAL");
        auto topic_info_map = monitoring_thread_->GetTopicInfoMap();
        auto filtered_topics = FilterAvailableTopics_NoLock(topic_info_map);
        CreateNewSubscribers_NoLock(filtered_topics);
        connected_to_ecal_ = true;
      }
    }

    void Recorder::DisconnectFromEcal()
    {
      {
        std::lock_guard<decltype(ecal_mutex_)> ecal_lock(ecal_mutex_);
        if (connected_to_ecal_)
        {
          EcalRecLogger::Instance()->info("Disconnecting from eCAL");

          subscriber_map_.clear();
          connected_to_ecal_ = false;
        }
      }

      {
        std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);
        StopRecording_NoLock();
        frame_buffer_.clear();
      }
    }

    bool Recorder::IsConnectedToEcal() const
    {
      std::lock_guard<decltype(ecal_mutex_)> ecal_lock(ecal_mutex_);
      return connected_to_ecal_;
    }

    std::set<std::string> Recorder::GetSubscribedTopics() const
    {
      std::set<std::string> subscribed_topics;

      std::lock_guard<decltype(ecal_mutex_)> ecal_lock(ecal_mutex_);
      for (const auto& subscriber : subscriber_map_)
      {
        subscribed_topics.emplace(subscriber.first);
      }
      return subscribed_topics;
    }

    std::map<std::string, bool> Recorder::GetHostsRunningEcalRpcService() const
    {
      return monitoring_thread_->GetHostsRunningEcalRpcService();
    }

    void Recorder::EcalMessageReceived(const char* topic_name, const eCAL::SReceiveCallbackData* callback_data)
    {
      auto ecal_receive_time = eCAL::Time::ecal_clock::now();
      auto system_receive_time = std::chrono::steady_clock::now();

      std::shared_ptr<Frame> frame = std::make_shared<Frame>(callback_data, topic_name, ecal_receive_time, system_receive_time);

      {
        std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);
        if (pre_buffering_enabled_)
        {
          frame_buffer_.push_back(frame);
        }

        auto main_recorder_state = main_recorder_thread_->GetState();
        if (main_recorder_state.recording_ && !main_recorder_state.flushing_)
        {
          main_recorder_thread_->AddFrame(frame);
        }
      }
    }

    //////////////////////////////////////
    /// API for external threads      ////
    //////////////////////////////////////
    void Recorder::GarbageCollect()
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

      // Clean threads
      buffer_writer_threads_.remove_if(
        [](const std::unique_ptr<Hdf5WriterThread>& buffer_writer)
      {
        if (!buffer_writer->IsRunning())
        {
          return true;
        }
        else
        {
          return false;
        }
      });
    }

    void Recorder::SetTopicInfo(const std::map<std::string, TopicInfo>& topic_info_map)
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

      // Set Topic information on all recording threads
      {
        std::lock_guard<decltype(recorder_mutex_)> recorder_lock(recorder_mutex_);

        main_recorder_thread_->SetTopicInfo(topic_info_map);

        for (const auto& buffer_writer : buffer_writer_threads_)
        {
          buffer_writer->SetTopicInfo(topic_info_map);
        }
      }
    }

    ////////////////////////////////////////////////////////////////////////////////
    //// Private functions                                                      ////
    ////////////////////////////////////////////////////////////////////////////////

    void Recorder::UpdateAndCleanSubscribers()
    {
      auto topic_info_map = monitoring_thread_->GetTopicInfoMap();

      std::lock_guard<decltype(ecal_mutex_)> ecal_lock(ecal_mutex_);

      if (!connected_to_ecal_)
        return;

      auto filtered_topic_set = FilterAvailableTopics_NoLock(topic_info_map);
      CreateNewSubscribers_NoLock(filtered_topic_set);
      RemoveOldSubscribers_NoLock(filtered_topic_set);
    }

    std::set<std::string> Recorder::FilterAvailableTopics_NoLock(const std::map<std::string, TopicInfo>& topic_info_map) const
    {
      std::set<std::string> topic_set;

      for (const auto& topic_info : topic_info_map)
      {
        if (topic_info.second.publisher_hosts_.empty())
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
          for (const auto& publisher_host : topic_info.second.publisher_hosts_)
          {
            if (EcalUtils::String::FindCaseInsensitive(hosts_filter_, publisher_host) != hosts_filter_.end())
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

    void Recorder::CreateNewSubscribers_NoLock(const std::set<std::string>& topic_set)
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
            continue;
          }
          if (!subscriber->AddReceiveCallback(std::bind(&Recorder::EcalMessageReceived, this, std::placeholders::_1, std::placeholders::_2)))
          {
            EcalRecLogger::Instance()->error("Error adding callback to subscriber on topic " + topic);
            continue;
          }
          subscriber_map_.emplace(topic, std::move(subscriber));
        }
      }
    }

    void Recorder::RemoveOldSubscribers_NoLock(const std::set<std::string>& topic_set)
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

    bool Recorder::StopRecording_NoLock()
    {
      auto main_recorder_state = main_recorder_thread_->GetState();
      if (!main_recorder_state.recording_ || main_recorder_state.flushing_)
      {
        // If we are not recording, we cannot stop it!
        return false;
      }
      else
      {
        // Start the flushing process of the main recorder thread. The thread will
        // terminate as soon as it finished flushing.
        main_recorder_thread_->Flush();
        return true;
      }
    }

    std::pair<size_t, std::chrono::steady_clock::duration> Recorder::GetCurrentPreBufferLength_NoLock() const
    {
      auto frame_count = frame_buffer_.size();
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

    bool Recorder::IsAnythingSavingToPath_NoLock(const std::string& path) const
    {
      if (main_recorder_thread_
        && main_recorder_thread_->GetState().recording_
        && EcalUtils::Path::IsEqual(path, main_recorder_thread_->GetCompleteMeasurementPath()))
      {
        return true;
      }

      for (const auto& buffer_writer : buffer_writer_threads_)
      {
        if (buffer_writer
          && buffer_writer->GetState().recording_
          && EcalUtils::Path::IsEqual(path, buffer_writer->GetCompleteMeasurementPath()))
        {
          return true;
        }
      }

      return false;
    }
  }
}