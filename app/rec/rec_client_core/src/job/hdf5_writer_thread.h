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
#include <ThreadingUtils/InterruptibleThread.h>

#include <ecalhdf5/eh5_meas.h>

#include <mutex>
#include <deque>
#include <map>

#include "frame.h"
#include "rec_client_core/job_config.h"
#include "rec_client_core/topic_info.h"
#include "rec_client_core/state.h"

namespace eCAL
{
  namespace rec
  {
    class Hdf5WriterThread : public InterruptibleThread
    {
    ///////////////////////////////
    // Constructor & Destructor
    ///////////////////////////////
    public:
      Hdf5WriterThread(const JobConfig& job_config, const std::map<std::string, TopicInfo>& initial_topic_info_map = {}, const std::deque<std::shared_ptr<Frame>>& initial_frame_buffer = {});

      ~Hdf5WriterThread();

    ///////////////////////////////
    // Thread control
    ///////////////////////////////
    public:
      void Interrupt() override;

      bool AddFrame(const std::shared_ptr<Frame>& frame);

      void SetTopicInfo(const std::map<std::string, TopicInfo>& topic_info_map);

      void Flush();

    protected:
      void Run() override;

    ///////////////////////////////
    // State
    ///////////////////////////////
    public:
      const JobConfig& GetJobConfig() const;

      bool IsFlushing();
      
      RecHdf5JobStatus GetStatus() const;

    ///////////////////////////////
    // Helper Methods
    ///////////////////////////////
    private:
      bool        OpenHdf5Writer() const;
      bool        CloseHdf5Writer();

      void SetTopicInfo_NoLock(const std::map<std::string, TopicInfo>& topic_info_map) const;

    ///////////////////////////////
    // Member Variables
    ///////////////////////////////
    private:
      JobConfig job_config_;

      mutable std::mutex                    frame_buffer_mutex_;
      std::deque<std::shared_ptr<Frame>>    frame_buffer_;
      mutable std::condition_variable       frame_buffer_cv_;
      size_t                                written_frames_;
      std::chrono::steady_clock::time_point first_written_frame_timestamp_;
      std::chrono::steady_clock::time_point last_written_frame_timestamp_;
      mutable RecHdf5JobStatus              last_status_;

      mutable std::mutex                   hdf5_writer_mutex_;
      std::unique_ptr<eCAL::eh5::HDF5Meas> hdf5_writer_;
      std::map<std::string, TopicInfo>     initial_topic_info_map_;

      std::atomic<bool> flushing_;
    };
  }
}