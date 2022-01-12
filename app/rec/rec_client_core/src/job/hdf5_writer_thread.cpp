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

#include "hdf5_writer_thread.h"

#include "rec_client_core/ecal_rec_logger.h"

#include <ecal_utils/filesystem.h>

namespace eCAL
{
  namespace rec
  {

    ///////////////////////////////
    // Constructor & Destructor
    ///////////////////////////////

    Hdf5WriterThread::Hdf5WriterThread(const JobConfig& job_config, const std::map<std::string, TopicInfo>& initial_topic_info_map, const std::deque<std::shared_ptr<Frame>>& initial_frame_buffer)
      : InterruptibleThread          ()
      , job_config_                  (job_config)
      , frame_buffer_                (initial_frame_buffer)
      , written_frames_              (0)
      , new_topic_info_map_          (initial_topic_info_map)
      , new_topic_info_map_available_(true)
      , flushing_                    (false)
    {
      hdf5_writer_ = std::make_unique<eCAL::eh5::HDF5Meas>();
    }

    Hdf5WriterThread::~Hdf5WriterThread()
    {
      Interrupt();
      Join();
    }


    ///////////////////////////////
    // Thread control
    ///////////////////////////////

    void Hdf5WriterThread::Interrupt()
    {
#ifndef NDEBUG
      EcalRecLogger::Instance()->debug("Hdf5WriterThread::Interrupt()");
#endif // NDEBUG

      InterruptibleThread::Interrupt();
      input_cv_.notify_all();
    }

    bool Hdf5WriterThread::AddFrame(const std::shared_ptr<Frame>& frame)
    {
      std::lock_guard<decltype(input_mutex_)> input_lock(input_mutex_);
      if (!flushing_)
      {
        frame_buffer_.push_back(frame);
        input_cv_.notify_one();
        return true;
      }
      else
      {
        return false;
      }
    }

    void Hdf5WriterThread::SetTopicInfo(std::map<std::string, TopicInfo> topic_info_map)
    {
      std::unique_lock<decltype(input_mutex_)> input_lock(input_mutex_);
      
      new_topic_info_map_           = std::move(topic_info_map);
      new_topic_info_map_available_ = true;

      input_cv_.notify_one();
    }

    void Hdf5WriterThread::Flush()
    {
      std::lock_guard<decltype(input_mutex_)> input_lock(input_mutex_);
      flushing_ = true;

      if (IsRunning())
      {
        // This log output is inside the IsRunning if condition, as a buffer writer thread would receive the flushing command before it has even been started
        if (frame_buffer_.size() > 0)
        {
          EcalRecLogger::Instance()->info("Flushing " + std::to_string(frame_buffer_.size()) + " frames...");  
        }
      }

      input_cv_.notify_all();
    }

    void Hdf5WriterThread::Run()
    {
#ifndef NDEBUG
      EcalRecLogger::Instance()->debug("Hdf5WriterThread::Run(): Starting Thread");
#endif // NDEBUG

      EcalRecLogger::Instance()->info("Measurement directory: " + job_config_.GetCompleteMeasurementPath());

      // Initialization
      if (!OpenHdf5Writer()) return;

      // Loop
      while (!IsInterrupted())
      {
        // Frame to write to the HDF5 file
        std::shared_ptr<Frame> frame;

        // Topic info to write to the HDF5 file
        bool set_topic_info_map = false;
        std::map<std::string, TopicInfo> topic_info_map_to_set; 

        {
          // Lock the input mutex
          std::unique_lock<decltype(input_mutex_)> input_lock(input_mutex_);

          // Wait until something is set to an input variable (frame_buffer_, topic info)
          input_cv_.wait(input_lock, [this]() { return IsInterrupted() || IsFlushing() || !frame_buffer_.empty() || !new_topic_info_map_available_; });

          if (IsInterrupted())
            break;

          if (new_topic_info_map_available_)
          {
            // Take topic info map
            set_topic_info_map = true;
            topic_info_map_to_set.swap(new_topic_info_map_);

            new_topic_info_map_available_ = false;
          }
          else if (!frame_buffer_.empty())
          {
            // take one frame from the framebuffer
            frame = frame_buffer_.front();
            frame_buffer_.pop_front();
            if (written_frames_ == 0)
            {
              first_written_frame_timestamp_ = frame->system_receive_time_;
            }
            last_written_frame_timestamp_ = frame->system_receive_time_;
            written_frames_++;
          }
        }

        if (set_topic_info_map)
        {
          std::unique_lock<decltype(hdf5_writer_mutex_)> hdf5_writer_lock(hdf5_writer_mutex_);

          for (const auto& topic : topic_info_map_to_set)
          {
            hdf5_writer_->SetChannelType(topic.first, topic.second.type_);
            hdf5_writer_->SetChannelDescription(topic.first, topic.second.description_);
          }
        }
        else if (frame)
        {
          std::unique_lock<decltype(hdf5_writer_mutex_)> hdf5_writer_lock(hdf5_writer_mutex_);

          if (IsInterrupted())
            break;

          // Write Frame element to HDF5
          if (!hdf5_writer_->AddEntryToFile(
            frame->data_.data(),
            frame->data_.size(),
            std::chrono::duration_cast<std::chrono::microseconds>(frame->ecal_publish_time_.time_since_epoch()).count(),
            std::chrono::duration_cast<std::chrono::microseconds>(frame->ecal_receive_time_.time_since_epoch()).count(),
            frame->topic_name_,
            frame->id_,
            frame->clock_
          ))
          {
            last_status_.info_ = { false, "Error adding frame to measurement" };
            EcalRecLogger::Instance()->error("Hdf5WriterThread::Run(): Unable to add Frame to measurement");
          }
        }
        else
        {
          if (flushing_)
          {
            // If there was no frame left and we were only supposed to flush existing frames, we terminate.
#ifndef NDEBUG
            EcalRecLogger::Instance()->debug("Hdf5WriterThread::Run(): Finished flushing frames");
#endif // NDEBUG
            break;
          }
        }
      }

      CloseHdf5Writer();

      EcalRecLogger::Instance()->info("Finished saving measurement");

#ifndef NDEBUG
      EcalRecLogger::Instance()->debug("Hdf5WriterThread: Thread is terminating");
#endif // NDEBUG
    }

    ///////////////////////////////
    // State
    ///////////////////////////////

    const JobConfig& Hdf5WriterThread::GetJobConfig() const
    {
      return job_config_;
    }

    bool Hdf5WriterThread::IsFlushing()
    {
      return flushing_;
    }

    RecHdf5JobStatus Hdf5WriterThread::GetStatus() const
    {
      {
        std::lock_guard<decltype(input_mutex_)> input_lock(input_mutex_);

        if (frame_buffer_.size() > 0)
        {
          last_status_.total_length_        = frame_buffer_.back()->system_receive_time_ - first_written_frame_timestamp_;
        }
        else
        {
          last_status_.total_length_        = last_written_frame_timestamp_ - first_written_frame_timestamp_;
        }

        last_status_.unflushed_frame_count_ = frame_buffer_.size();
        last_status_.total_frame_count_     = written_frames_ + frame_buffer_.size();
      }

      return last_status_;
    }

    ///////////////////////////////
    // Helper Methods
    ///////////////////////////////
    bool Hdf5WriterThread::OpenHdf5Writer() const
    {
      std::string host_name = eCAL::Process::GetHostName();
      std::string hdf5_dir  = EcalUtils::Filesystem::ToNativeSeperators(job_config_.GetCompleteMeasurementPath() + "/" + host_name);

#ifndef NDEBUG
      EcalRecLogger::Instance()->debug("Hdf5WriterThread::Open(): hdf5_dir: \"" + hdf5_dir + "\", base_name: \"" + host_name + "\"");
#endif // NDEBUG
      std::unique_lock<decltype(hdf5_writer_mutex_)> hdf5_writer_lock(hdf5_writer_mutex_);

      if (hdf5_writer_->Open(hdf5_dir, eCAL::eh5::eAccessType::CREATE))
      {
#ifndef NDEBUG
        EcalRecLogger::Instance()->debug("Hdf5WriterThread::Open(): Successfully opened HDF5-Writer with path \"" + hdf5_dir + "\"");
#endif // NDEBUG

        hdf5_writer_->SetFileBaseName(host_name);
        hdf5_writer_->SetMaxSizePerFile(job_config_.GetMaxFileSize());
      }
      else
      {
        last_status_.info_ = { false, "Unable to create measurement \"" + hdf5_dir + "\"" };
        EcalRecLogger::Instance()->error("Hdf5WriterThread::Open(): Unable to create measurement \"" + hdf5_dir + "\"");
        return false;
      }

      return true;
    }

    bool Hdf5WriterThread::CloseHdf5Writer()
    {
#ifndef NDEBUG
      EcalRecLogger::Instance()->debug("Hdf5WriterThread::Close()");
#endif // NDEBUG

      std::unique_lock<decltype(hdf5_writer_mutex_)> hdf5_writer_lock(hdf5_writer_mutex_);

      if (!hdf5_writer_->Close())
      {
        EcalRecLogger::Instance()->error("Hdf5WriterThread::Close(): Unable to close measurement");
        return false;
      }
      else
      {
#ifndef NDEBUG
        EcalRecLogger::Instance()->debug("Hdf5WriterThread::Close(): Successfully closed measurement");
#endif // NDEBUG
        return true;
      }
    }
  }
}