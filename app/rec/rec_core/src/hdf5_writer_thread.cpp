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

#include "rec_core/ecal_rec_logger.h"
#include "recorder.h"

#include <EcalUtils/Path.h>

#include <fstream>
#include <EcalUtils/Path.h>

#include <EcalParser/EcalParser.h>

namespace eCAL
{
  namespace rec
  {

    ///////////////////////////////
    // Constructor & Destructor
    ///////////////////////////////

    Hdf5WriterThread::Hdf5WriterThread(const JobConfig& job_config, const std::map<std::string, TopicInfo>& initial_topic_info_map, const std::deque<std::shared_ptr<Frame>>& initial_frame_buffer)
      : InterruptibleThread    ()
      , job_config_            (job_config)
      , frame_buffer_          (initial_frame_buffer)
      , written_frames_        (0)
      , initial_topic_info_map_(initial_topic_info_map)
      , flushing_              (false)
    {
      hdf5_writer_ = std::make_unique<eCAL::eh5::HDF5Meas>();

      // Parse job config
      auto now = std::chrono::system_clock::now();

      job_config_.SetMeasRootDir(EcalParser::Evaluate(job_config_.GetMeasRootDir(), true, now));
      job_config_.SetMeasName   (EcalParser::Evaluate(job_config_.GetMeasName(),    true, now));
      job_config_.SetDescription(EcalParser::Evaluate(job_config_.GetDescription(), true, now));
    }

    Hdf5WriterThread::~Hdf5WriterThread()
    {
      Interrupt();
      Join();
    }

    std::string Hdf5WriterThread::GetCompleteMeasurementPath() const
    {
      std::string joined_meas_path = job_config_.GetMeasRootDir();

      if ((!joined_meas_path.empty() && (joined_meas_path[joined_meas_path.size() - 1] != '\\') && (joined_meas_path[joined_meas_path.size() - 1] != '/'))
        && (!job_config_.GetMeasName().empty() && (job_config_.GetMeasName()[0] != '\\') && (job_config_.GetMeasName()[0] != '/')))
      {
        joined_meas_path += "/";
      }
      joined_meas_path += job_config_.GetMeasName();

      return EcalUtils::Path::AbsolutePath(joined_meas_path);
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
      frame_buffer_cv_.notify_all();
    }

    bool Hdf5WriterThread::AddFrame(const std::shared_ptr<Frame>& frame)
    {
      std::lock_guard<decltype(frame_buffer_mutex_)> frame_buffer_lock(frame_buffer_mutex_);
      if (!flushing_)
      {
        frame_buffer_.push_back(frame);
        frame_buffer_cv_.notify_one();
        return true;
      }
      else
      {
        return false;
      }
    }

    void Hdf5WriterThread::SetTopicInfo(const std::map<std::string, TopicInfo>& topic_info_map)
    {
      std::unique_lock<decltype(hdf5_writer_mutex_)> hdf5_writer_lock(hdf5_writer_mutex_);
      SetTopicInfo_NoLock(topic_info_map);
    }

    void Hdf5WriterThread::Flush()
    {
      std::lock_guard<decltype(frame_buffer_mutex_)> frame_buffer_lock(frame_buffer_mutex_);
      flushing_ = true;

      if (IsRunning())
      {
        // This log output is inside the IsRunning if condition, as a buffer writer thread would receive the flushing command before it has even been started
        if (frame_buffer_.size() > 0)
        {
          EcalRecLogger::Instance()->info("Flushing " + std::to_string(frame_buffer_.size()) + " frames...");  
        }
      }

      frame_buffer_cv_.notify_all();
    }

    void Hdf5WriterThread::Run()
    {
#ifndef NDEBUG
      EcalRecLogger::Instance()->debug("Hdf5WriterThread::Run(): Starting Thread");
#endif // NDEBUG

      EcalRecLogger::Instance()->info("Measurement directory: " +GetCompleteMeasurementPath());

      // Initialization
      CreateEcalmeasFile();
      SaveDescription();
      if (!OpenHdf5Writer()) return;

      // Loop
      while (!IsInterrupted())
      {
        std::shared_ptr<Frame> frame;
        {
          // Lock the frame buffer mutex
          std::unique_lock<decltype(frame_buffer_mutex_)> frame_buffer_lock(frame_buffer_mutex_);

          // Wait until something is in the buffer
          frame_buffer_cv_.wait(frame_buffer_lock, [this]() { return IsInterrupted() || IsFlushing() || !frame_buffer_.empty(); });

          if (IsInterrupted())
            break;

          // Copy one element from the buffer
          if (!frame_buffer_.empty())
          {
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

        if (frame)
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

    bool Hdf5WriterThread::IsFlushing()
    {
      return flushing_;
    }

    WriterState Hdf5WriterThread::GetState() const
    {
      WriterState state;

      // Recording status
      state.recording_ = IsRunning();

      // Flushing status
      state.flushing_ = (state.recording_ && flushing_);

      std::lock_guard<decltype(frame_buffer_mutex_)> frame_buffer_lock(frame_buffer_mutex_);

      // Buffer status
      size_t buffer_frame_count = frame_buffer_.size();
      std::chrono::steady_clock::duration buffer_length;
      if (buffer_frame_count > 1)
      {
        buffer_length = frame_buffer_.back()->system_receive_time_ - frame_buffer_.front()->system_receive_time_;
      }
      else
      {
        buffer_length = std::chrono::steady_clock::duration(0);
      }
      state.recording_queue_ = std::make_pair(buffer_frame_count, buffer_length);

      // Total status
      size_t total_frame_count = written_frames_ + buffer_frame_count;
      std::chrono::steady_clock::duration total_length;
      if (buffer_frame_count > 0)
      {
        total_length = frame_buffer_.back()->system_receive_time_ - first_written_frame_timestamp_;
      }
      else
      {
        total_length = last_written_frame_timestamp_ - first_written_frame_timestamp_;
      }
      state.recording_length_ = std::make_pair(total_frame_count, total_length);

      return state;
    }

    ///////////////////////////////
    // Helper Methods
    ///////////////////////////////

    bool Hdf5WriterThread::CreateEcalmeasFile() const
    {
      std::string measurement_path = GetCompleteMeasurementPath();
      if (!EcalUtils::Path::MkPath(measurement_path))
      {
        EcalRecLogger::Instance()->error("Error Creating directory \"" + measurement_path + "\"");
        return false;
      }

      // Get the last dirname of the complete measurement path and use it as meas name
      std::list<std::string> path_components = EcalUtils::Path::CleanPathComponentList(measurement_path);
      std::string meas_name;
      if (!path_components.empty())
      {
        meas_name = path_components.back();
      }

      if (meas_name.empty() || (meas_name == ".") || (meas_name == ".."))
      {
        // If the user only used empty strings, just call the meas name "measurement"
        meas_name = "measurement";
      }
      meas_name += ".ecalmeas";

      // Create file /meas/root/dir/meas_name/meas_name.ecalmeas
      std::string ecalmeas_file_path = EcalUtils::Path::ToNativeSeperators(GetCompleteMeasurementPath() + '/' + meas_name);

      EcalRecLogger::Instance()->info("Creating eacalmeas file: " + ecalmeas_file_path);

      std::ofstream file{ ecalmeas_file_path };
      return true;

    }

    bool Hdf5WriterThread::SaveDescription() const
    {
      std::string doc_dir   = GetCompleteMeasurementPath() + "/doc/";
      std::string full_path = doc_dir + "description.txt";

      full_path = EcalUtils::Path::ToNativeSeperators(full_path);

      EcalRecLogger::Instance()->info("Saving description to " + full_path);

      if (!EcalUtils::Path::MkPath(doc_dir))
      {
        EcalRecLogger::Instance()->error("Error saving description: Cannot create doc dir \"" + doc_dir + "\"");
        return false;
      }

      std::ofstream description_file;
      description_file.open(full_path, std::ios::out | std::ios::trunc);
      if (description_file.is_open())
      {
        description_file << job_config_.GetDescription();
        description_file.close();
        return true;
      }
      else
      {
        EcalRecLogger::Instance()->error("Error saving description: Cannot open file");
        return false;
      }
    }

    bool Hdf5WriterThread::OpenHdf5Writer() const
    {
      std::string host_name = eCAL::Process::GetHostName();
      std::string hdf5_dir  = EcalUtils::Path::ToNativeSeperators(GetCompleteMeasurementPath() + "/" + host_name);

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
        EcalRecLogger::Instance()->error("Hdf5WriterThread::Open(): Unable to create measurement \"" + hdf5_dir + "\"");
        return false;
      }

      SetTopicInfo_NoLock(initial_topic_info_map_);

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

    void Hdf5WriterThread::SetTopicInfo_NoLock(const std::map<std::string, TopicInfo>& topic_info_map) const
    {
      for (const auto& topic : topic_info_map)
      {
        hdf5_writer_->SetChannelType(topic.first, topic.second.type_);
        hdf5_writer_->SetChannelDescription(topic.first, topic.second.description_);
      }
    }
  }
}