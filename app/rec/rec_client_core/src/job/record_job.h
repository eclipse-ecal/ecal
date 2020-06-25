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

#include <rec_client_core/job_config.h>

#include <map>
#include <memory>
#include <mutex>
#include <deque>
#include <string>

#include <rec_client_core/state.h>
#include <rec_client_core/job_config.h>
#include <rec_client_core/upload_config.h>
#include <rec_client_core/topic_info.h>

#include <rec_client_core/rec_error.h>

namespace eCAL
{
  namespace rec
  {
    class Hdf5WriterThread;
#ifdef ECAL_HAS_CURL
    class FtpUploadThread;
#endif //ECAL_HAS_CURL
    class Frame;

    class RecordJob
    {
    public:
      // ==== Non-copyalbe ====
      RecordJob(const RecordJob&) = delete;
      RecordJob& operator=(const RecordJob&) = delete;

    ///////////////////////////////////////////////
    // Constructor & Destructor
    ///////////////////////////////////////////////
    public:
      RecordJob(const JobConfig& evaluated_job_config);
      ~RecordJob();
      void Interrupt();

    ///////////////////////////////////////////////
    // Recording, Flushing, Uploading
    ///////////////////////////////////////////////
    public:
      bool InitializeMeasurementDirectory();
      bool StartRecording(const std::map<std::string, TopicInfo>& initial_topic_info_map, const std::deque<std::shared_ptr<Frame>>& initial_frame_buffer);
      bool StopRecording ();
      bool SaveBuffer    (const std::map<std::string, TopicInfo>& topic_info_map,         const std::deque<std::shared_ptr<Frame>>& frame_buffer);

      bool AddFrame(const std::shared_ptr<Frame>& frame);
      void SetTopicInfo(const std::map<std::string, TopicInfo>& topic_info_map);

      eCAL::rec::Error Upload(const UploadConfig& upload_config);

      eCAL::rec::Error AddComment(const std::string& comment);

      Error DeleteMeasurement();

    ///////////////////////////////////////////////
    // Status API
    ///////////////////////////////////////////////
    public:
      const JobConfig& GetJobConfig() const;

      JobStatus GetJobStatus() const;
      void SetAddonStatus(const std::string& addon, const RecAddonJobStatus& status);

      JobState GetMainRecorderState() const;

      bool IsDeleted() const;

    private:
      bool AnyAddonStateIs_NoLock(RecAddonJobStatus::State state) const;

    ///////////////////////////////////////////////
    // Private helper methods
    ///////////////////////////////////////////////
    private:
      void UpdateJobState_NoLock() const;

    ///////////////////////////////////////////////
    // Member Variables
    ///////////////////////////////////////////////
    private:
      mutable std::mutex                       job_mutex_;

      const JobConfig                          job_config_;
      std::unique_ptr<Hdf5WriterThread>        hdf5_writer_thread_;

#ifdef ECAL_HAS_CURL
      std::unique_ptr<FtpUploadThread>         ftp_upload_thread_;
#endif // ECAL_HAS_CURL

      mutable JobState                         main_recorder_state_;
      std::map<std::string, RecAddonJobStatus> rec_addon_statuses_;
      UploadStatus                             upload_status_;
      
      bool                                     safe_to_delete_dir_;             /**< True when the meas directory had to be created or was empty */
      bool                                     is_deleted_;

      std::pair<bool, std::string>             info_;

      std::vector<std::string> files_with_metadata_;
    };

  }
}
