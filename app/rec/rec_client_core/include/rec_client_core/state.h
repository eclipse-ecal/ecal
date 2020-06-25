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

#include <string>
#include <utility>
#include <map>
#include <set>
#include <vector>
#include <chrono>

#include <ecal/ecal_time.h>

namespace eCAL
{
  namespace rec
  {
    struct RecHdf5JobStatus
    {
      RecHdf5JobStatus() : total_length_(0), total_frame_count_(0), unflushed_frame_count_(0), info_{ true, "" } {}

      std::chrono::steady_clock::duration total_length_;
      int64_t                             total_frame_count_;
      int64_t                             unflushed_frame_count_;
      std::pair<bool, std::string>        info_;

      bool operator==(const RecHdf5JobStatus& other) const { return (total_length_ == other.total_length_) && (total_frame_count_ == other.total_frame_count_) && (unflushed_frame_count_ == other.unflushed_frame_count_) && (info_ == other.info_); }
      bool operator!=(const RecHdf5JobStatus& other) const { return !operator==(other); }
    };

    struct RecAddonJobStatus
    {
      enum State
      {
        NotStarted,
        Recording,
        Flushing,
        FinishedFlushing,
      };

      RecAddonJobStatus() : state_(State::NotStarted), total_frame_count_(0), unflushed_frame_count_(0), info_{ true, "" } {}

      State                        state_;
      int64_t                      total_frame_count_;
      int64_t                      unflushed_frame_count_;
      std::pair<bool, std::string> info_;

      bool operator==(const RecAddonJobStatus& other) const
      {
        return (state_               == other.state_)
          && (total_frame_count_     == other.total_frame_count_)
          && (unflushed_frame_count_ == other.unflushed_frame_count_)
          && (info_                  == other.info_);
      }
      bool operator!=(const RecAddonJobStatus& other) const { return !operator==(other); }
    };

    struct UploadStatus
    {
      UploadStatus() : bytes_total_size_(0), bytes_uploaded_(0), info_{ true, "" } {}

      uint64_t                     bytes_total_size_;
      uint64_t                     bytes_uploaded_;
      std::pair<bool, std::string> info_;

      bool operator==(const UploadStatus& other) const { return (bytes_total_size_ == other.bytes_total_size_) && (bytes_uploaded_ == other.bytes_uploaded_) && (info_ == other.info_); }
      bool operator!=(const UploadStatus& other) const { return !operator==(other); }
    };

    enum class JobState
    {
      NotStarted,

      Recording,
      Flushing,
      FinishedFlushing,

      Uploading,
      FinishedUploading,
    };

    struct JobStatus
    {
      JobStatus() : job_id_(0), state_(JobState::NotStarted), is_deleted_(false) {}

      int64_t                                  job_id_;
      JobState                                 state_;
      RecHdf5JobStatus                         rec_hdf5_status_;
      std::map<std::string, RecAddonJobStatus> rec_addon_statuses_;
      UploadStatus                             upload_status_;
      bool                                     is_deleted_;

      bool operator==(const JobStatus& other) const
      {
        return (job_id_           == other.job_id_)
          && (state_              == other.state_)
          && (rec_hdf5_status_    == other.rec_hdf5_status_)
          && (rec_addon_statuses_ == other.rec_addon_statuses_)
          && (upload_status_      == other.upload_status_)
          && (is_deleted_         == other.is_deleted_);
      }
      bool operator!=(const JobStatus& other) const { return !operator==(other); }
    };

    struct RecorderAddonStatus
    {
      RecorderAddonStatus() : initialized_(false), pre_buffer_length_frame_count_(0), info_{ true, "" }
      {}

      std::string                  addon_executable_path_;
      std::string                  addon_id_;
      bool                         initialized_;
      int64_t                      pre_buffer_length_frame_count_;
      std::string                  name_;
      std::pair<bool, std::string> info_;

      bool operator==(const RecorderAddonStatus& other) const
      {
        return (addon_executable_path_       == other.addon_executable_path_)
          && (addon_id_                      == other.addon_id_)
          && (initialized_                   == other.initialized_)
          && (pre_buffer_length_frame_count_ == other.pre_buffer_length_frame_count_)
          && (name_                          == other.name_)
          && (info_                          == other.info_);
      }
      bool operator!=(const RecorderAddonStatus& other) const { return !operator==(other); }
    };

    struct RecorderStatus
    {
      RecorderStatus() : pid_(-1), timestamp_(eCAL::Time::ecal_clock::duration(0)), initialized_(false), pre_buffer_length_{ 0, std::chrono::steady_clock::duration(0) }, info_{ true, "" } {}
      int                                                     pid_;
      eCAL::Time::ecal_clock::time_point                      timestamp_;
      bool                                                    initialized_;
      std::pair<int64_t, std::chrono::steady_clock::duration> pre_buffer_length_;
      std::set<std::string>                                   subscribed_topics_;
      std::vector<RecorderAddonStatus>                        addon_statuses_;
      std::vector<JobStatus>                                  job_statuses_;
      std::pair<bool, std::string>                            info_;


      bool operator==(const RecorderStatus& other) const
      {
        return (timestamp_       == other.timestamp_)
          && (initialized_       == other.initialized_)
          && (pre_buffer_length_ == other.pre_buffer_length_)
          && (subscribed_topics_ == other.subscribed_topics_)
          && (addon_statuses_    == other.addon_statuses_)
          && (job_statuses_      == other.job_statuses_)
          && (info_              == other.info_);
      }
      bool operator!=(const RecorderStatus& other) const { return !operator==(other); }
    };
  }
}