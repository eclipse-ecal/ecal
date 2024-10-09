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

#include "record_job.h"

#include <ecal_utils/filesystem.h>
#include <ecal_utils/ecal_utils.h>

#include "hdf5_writer_thread.h"

#include <ecal_utils/str_convert.h>

#ifdef ECAL_HAS_CURL
  #include "ftp_upload_thread.h"
#endif // ECAL_HAS_CURL

#include "frame.h"

#include <rec_client_core/ecal_rec_logger.h>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

// TODO: The RecordJob should log stuff, not the API using it
namespace eCAL
{
  namespace rec
  {

    ///////////////////////////////////////////////
    // Constructor & Destructor
    ///////////////////////////////////////////////

    RecordJob::RecordJob(const JobConfig& evaluated_job_config)
      : job_config_         (evaluated_job_config)
      , main_recorder_state_(JobState::NotStarted)
      , safe_to_delete_dir_ (false)
      , is_deleted_         (false)
      , info_               {true, ""}
    {}

    RecordJob::~RecordJob()
    {
      if (hdf5_writer_thread_)
      {
        hdf5_writer_thread_->Interrupt();
        hdf5_writer_thread_->Join();
        hdf5_writer_thread_ = nullptr;
      }
#ifdef ECAL_HAS_CURL
      if (ftp_upload_thread_)
      {
        ftp_upload_thread_->Interrupt();
        ftp_upload_thread_->Join();
        ftp_upload_thread_ = nullptr;
      }
#endif //ECAL_HAS_CURL
    }

    void RecordJob::Interrupt()
    {
      if (hdf5_writer_thread_)
        hdf5_writer_thread_->Interrupt();
#ifdef ECAL_HAS_CURL
      if (ftp_upload_thread_)
        hdf5_writer_thread_->Interrupt();
#endif // ECAL_HAS_CURL
    }

    ///////////////////////////////////////////////
    // Recording, Flushing, Uploading
    ///////////////////////////////////////////////
    bool RecordJob::InitializeMeasurementDirectory()
    {
      std::string measurement_path = job_config_.GetCompleteMeasurementPath();

      // Check whether the measurement path does not exist or is at least empty
      {
        auto meas_dir_status = EcalUtils::Filesystem::FileStatus(measurement_path);
        if (meas_dir_status.IsOk())
        {
          auto dir_content = EcalUtils::Filesystem::DirContent(measurement_path);
          if (dir_content.size() == 0)
          {
            safe_to_delete_dir_ = true;
          }
          else
          {
            EcalRecLogger::Instance()->warn("Warning: The measurement folder is not empty!");
          }
        }
        else
        {
          safe_to_delete_dir_ = true;
        }
      }

      // doc directory
      {
        if (!EcalUtils::Filesystem::MkPath(measurement_path + "/doc/"))
        {
          const std::string error_message = "Error Creating directory \"" + measurement_path  + "/doc/\"";
          info_ = { false, error_message };
          EcalRecLogger::Instance()->error(error_message);
          return false;
        }
      }

      // Hostname directory
      {
        std::string hostname_dir = measurement_path + "/" + eCAL::Process::GetHostName() + "/";

        // Create directory
        if (!EcalUtils::Filesystem::MkPath(hostname_dir))
        {
          const std::string error_message = "Error Creating directory \"" + measurement_path + "/" + eCAL::Process::GetHostName() + "/\"";
          info_ = { false, error_message };
          EcalRecLogger::Instance()->error(error_message);
          return false;
        }

        // Create system_information.txt
        {
          std::string system_information_path = hostname_dir + "/system_information.txt";

          std::ofstream system_information_file;
#ifdef WIN32
          std::wstring w_system_information_path = EcalUtils::StrConvert::Utf8ToWide(system_information_path);
          system_information_file.open(w_system_information_path, std::ios::trunc);
#else
          system_information_file.open(system_information_path, std::ios::trunc);
#endif // WIN32

          if (system_information_file.is_open())
          {
            std::string config_dump;
            eCAL::Process::DumpConfig(config_dump);
            system_information_file << config_dump;
            system_information_file.close();
          }
          else
          {
            const std::string error_message = "Error creating file \"" + system_information_path + "\""; 
            info_ = { false, error_message };
            EcalRecLogger::Instance()->error(error_message);
          }
        }

        // Copy ecal.yaml
#ifdef CopyFile
#define CopyFile_6376c040f4f54106b205ef6ddbb2090a CopyFile
#undef CopyFile
#endif // CopyFile
        {
          std::string ecal_ini_original_path = eCAL::Config::GetLoadedEcalIniPath();

          if (ecal_ini_original_path.empty())
          {
            const std::string error_message = "Error copying ecal.yaml: The system information does not contain \"Default INI\" identifier."; 
            info_ = { false, error_message };
            EcalRecLogger::Instance()->error(error_message);
          }

          if (EcalUtils::Filesystem::IsFile(ecal_ini_original_path))
          {
            std::string ecal_ini_dest_path = EcalUtils::Filesystem::ToNativeSeperators(hostname_dir + "/ecal.yaml");
            EcalRecLogger::Instance()->info("Copying ecal.yaml from \"" + ecal_ini_original_path + "\" to \"" + ecal_ini_dest_path + "\"");

            if (!EcalUtils::Filesystem::CopyFile(ecal_ini_original_path, ecal_ini_dest_path, EcalUtils::Filesystem::OsStyle::Current))
            {
              const std::string error_message = "Error copying ecal.yaml file to \"" + ecal_ini_dest_path + "\""; 
              info_ = { false, error_message };
              EcalRecLogger::Instance()->error(error_message);
            }
          }
          else
          {
            const std::string error_message = "Warning: ecal.yaml does not exist and will not be copied."; 
            info_ = { false, error_message };
            EcalRecLogger::Instance()->warn("Warning: ecal.yaml does not exist and will not be copied.");
          }
        }
#ifdef CopyFile_6376c040f4f54106b205ef6ddbb2090a
#define CopyFile CopyFile_6376c040f4f54106b205ef6ddbb2090a
#undef CopyFile_6376c040f4f54106b205ef6ddbb2090a
#endif // CopyFile_6376c040f4f54106b205ef6ddbb2090a
      }

      // .ecalmeas file
      {
        // Get the last dirname of the complete measurement path and use it as meas name
        std::vector<std::string> path_components = EcalUtils::Filesystem::CleanPathComponentList(measurement_path);
        std::string ecalmeas_file_name;
        if (!path_components.empty())
        {
          ecalmeas_file_name = path_components.back();
        }

        if (ecalmeas_file_name.empty() || (ecalmeas_file_name == ".") || (ecalmeas_file_name == ".."))
        {
          // If the user only used empty strings, just call the meas name "measurement"
          ecalmeas_file_name = "measurement";
        }
        ecalmeas_file_name += ".ecalmeas";

        // Create file /meas/root/dir/ecalmeas_file_name/ecalmeas_file_name.ecalmeas
        std::string ecalmeas_file_path = EcalUtils::Filesystem::ToNativeSeperators(job_config_.GetCompleteMeasurementPath() + '/' + ecalmeas_file_name);

        EcalRecLogger::Instance()->info("Creating eacalmeas file: " + ecalmeas_file_path);

        std::ofstream ecalmeas_file;
#ifdef WIN32
        std::wstring w_ecalmeas_file_path = EcalUtils::StrConvert::Utf8ToWide(ecalmeas_file_path);
        ecalmeas_file.open(w_ecalmeas_file_path, std::ios::out | std::ios::trunc);
#else
        ecalmeas_file.open(ecalmeas_file_path, std::ios::out | std::ios::trunc);
#endif // WIN32

        if (ecalmeas_file.is_open())
        {
          // For mimetypes to work, ecalmeas file should not be empty
          ecalmeas_file << std::string(" ");
          ecalmeas_file.close();
          files_with_metadata_.push_back(ecalmeas_file_name);
        }
        else
        {
          const std::string error_message = "Error creating file \"" + ecalmeas_file_path + "\""; 
          info_ = { false, error_message };
          EcalRecLogger::Instance()->error(error_message);
          return false;
        }
      }

      // description.txt file
      {
        std::string relative_description_path = "doc/description.txt";
        std::string full_path = EcalUtils::Filesystem::ToNativeSeperators(measurement_path + "/" + relative_description_path);

        EcalRecLogger::Instance()->info("Saving description to " + full_path);

        std::ofstream description_file;
#ifdef WIN32
        std::wstring w_full_path = EcalUtils::StrConvert::Utf8ToWide(full_path);
        description_file.open(w_full_path, std::ios::out | std::ios::trunc);
#else
        description_file.open(full_path, std::ios::out | std::ios::trunc);
#endif // WIN32

        if (description_file.is_open())
        {
          description_file << job_config_.GetDescription();
          description_file.close();
          files_with_metadata_.push_back(relative_description_path);
        }
        else
        {
          const std::string error_message = "Error saving description: Cannot open file"; 
          info_ = { false, error_message };
          EcalRecLogger::Instance()->error(error_message);
          return false;
        }
      }

      return true;
    }

    bool RecordJob::StartRecording(const std::map<std::string, TopicInfo>& initial_topic_info_map, const std::deque<std::shared_ptr<Frame>>& initial_frame_buffer)
    {
      std::unique_lock<std::shared_timed_mutex> lock(job_mutex_);

      if (main_recorder_state_ != JobState::NotStarted)
      {
        return false;
      }

      hdf5_writer_thread_ = std::make_unique<Hdf5WriterThread>(job_config_, initial_topic_info_map, initial_frame_buffer);
      hdf5_writer_thread_->Start();

      main_recorder_state_ = JobState::Recording;

      return true;
    }

    bool RecordJob::StopRecording()
    {
      std::unique_lock<std::shared_timed_mutex> lock(job_mutex_);

      if ((main_recorder_state_ != JobState::Recording) || !hdf5_writer_thread_)
      {
        return false;
      }

      hdf5_writer_thread_->Flush();

      main_recorder_state_ = JobState::Flushing;

      return true;
    }


    bool RecordJob::SaveBuffer(const std::map<std::string, TopicInfo>& topic_info_map, const std::deque<std::shared_ptr<Frame>>& frame_buffer)
    {
      std::unique_lock<std::shared_timed_mutex> lock(job_mutex_);

      if (main_recorder_state_ != JobState::NotStarted)
      {
        return false;
      }

      hdf5_writer_thread_ = std::make_unique<Hdf5WriterThread>(job_config_, topic_info_map, frame_buffer);
      hdf5_writer_thread_->Flush();
      hdf5_writer_thread_->Start();

      main_recorder_state_ = JobState::Flushing;

      return true;
    }

    bool RecordJob::AddFrame(const std::shared_ptr<Frame>& frame)
    {
      std::shared_lock<std::shared_timed_mutex> lock(job_mutex_);
      if ((main_recorder_state_ != JobState::Recording) || !hdf5_writer_thread_)
        return false;

      return hdf5_writer_thread_->AddFrame(frame);
    }

    void RecordJob::SetTopicInfo(const std::map<std::string, TopicInfo>& topic_info_map)
    {
      std::shared_lock<std::shared_timed_mutex> lock(job_mutex_);
      if ((main_recorder_state_ != JobState::Recording) || !hdf5_writer_thread_)
        return;

      hdf5_writer_thread_->SetTopicInfo(topic_info_map);
    }

    eCAL::rec::Error RecordJob::Upload(const UploadConfig& upload_config)
    {
      eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::OK);

#ifdef ECAL_HAS_CURL
      std::unique_lock<std::shared_timed_mutex> lock(job_mutex_);
      UpdateJobState_NoLock();

      if (is_deleted_)
        error = Error::ErrorCode::MEAS_IS_DELETED;
      else if ((main_recorder_state_ == JobState::Recording) || (AnyAddonStateIs_NoLock(RecAddonJobStatus::State::Recording)))
        error = eCAL::rec::Error::ErrorCode::CURRENTLY_RECORDING;
      else if ((main_recorder_state_ == JobState::Flushing) || (AnyAddonStateIs_NoLock(RecAddonJobStatus::State::Flushing)))
        error = eCAL::rec::Error::ErrorCode::CURRENTLY_FLUSHING;
      else if (main_recorder_state_ == JobState::Uploading)
        error = eCAL::rec::Error::ErrorCode::CURRENTLY_UPLOADING;

      if (!error)
      {
        main_recorder_state_ = JobState::Uploading;

        if (ftp_upload_thread_)
        {
          ftp_upload_thread_->Interrupt();
          ftp_upload_thread_->Join();
        }

        std::string local_root_dir = job_config_.GetCompleteMeasurementPath();
        std::string ftp_server     = "ftp://" + upload_config.username_ + ":" + upload_config.password_ + "@" + upload_config.host_ + ":" + std::to_string(upload_config.port_);
        std::string upload_path    = EcalUtils::Filesystem::CleanPath("/" + upload_config.upload_path_ + "/", EcalUtils::Filesystem::Unix);

        if (upload_config.upload_metadata_files_)
        {
          if (upload_config.delete_after_upload_)
          {
            ftp_upload_thread_ = std::make_unique<FtpUploadThread>(std::move(local_root_dir)
                                                                 , std::move(ftp_server)
                                                                 , std::move(upload_path)
                                                                 , std::vector<std::string>()
                                                                 , [this]() -> Error { return this->DeleteMeasurement(true); });
          }
          else
          {
            ftp_upload_thread_ = std::make_unique<FtpUploadThread>(std::move(local_root_dir)
                                                                 , std::move(ftp_server)
                                                                 , std::move(upload_path)
                                                                 , std::vector<std::string>());
          }
        }
        else
        {
          if (upload_config.delete_after_upload_)
          {
            ftp_upload_thread_ = std::make_unique<FtpUploadThread>(std::move(local_root_dir)
                                                                 , std::move(ftp_server)
                                                                 , std::move(upload_path)
                                                                 , files_with_metadata_
                                                                 , [this]() -> Error { return this->DeleteMeasurement(true); });
          }
          else
          {
            ftp_upload_thread_ = std::make_unique<FtpUploadThread>(std::move(local_root_dir)
                                                                 , std::move(ftp_server)
                                                                 , std::move(upload_path)
                                                                 , files_with_metadata_);
          }
        }
        ftp_upload_thread_->Start();
      }
      else
      {
        info_ = { false, "Error Uploading measurement: " + error.ToString() }; // TODO: This was not here in the original code. check if it makes sense to add it.
        EcalRecLogger::Instance()->error(info_.second);
      }
#else // ECAL_HAS_CURL
      info_ = { false, "eCAL has been built without CURL support" };
      error = eCAL::rec::Error(eCAL::rec::Error::ErrorCode::UNSUPPORTED_ACTION, "eCAL has been built without CURL support");
#endif // ECAL_HAS_CURL

      return error;
    }

    eCAL::rec::Error RecordJob::AddComment(const std::string& comment)
    {
      eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::OK);

      {
        std::unique_lock<std::shared_timed_mutex> lock(job_mutex_);
        UpdateJobState_NoLock();

        if (is_deleted_)
          error = Error::ErrorCode::MEAS_IS_DELETED;
        else if (main_recorder_state_ == JobState::Uploading)
          error = eCAL::rec::Error::ErrorCode::CURRENTLY_UPLOADING;
      }

      std::string measurement_path = job_config_.GetCompleteMeasurementPath();
      std::string description_path = EcalUtils::Filesystem::ToNativeSeperators(measurement_path + "/doc/description.txt");

      if (!error)
      {
        std::ofstream description_file;
#ifdef WIN32
        std::wstring w_description_path = EcalUtils::StrConvert::Utf8ToWide(description_path);
        description_file.open(w_description_path, std::ios::out | std::ios::app);
#else
        description_file.open(description_path, std::ios::out | std::ios::app);
#endif // WIN32

        if (description_file.is_open())
        {
          description_file << std::endl;
          description_file << comment;
          description_file.close();
        }
        else
        {
          error = eCAL::rec::Error(eCAL::rec::Error::ErrorCode::RESOURCE_UNAVAILABLE, description_path);
        }
      }

      if (!error)
      {
        EcalRecLogger::Instance()->info("Added comment to " + description_path);
      }
      else
      {
        std::unique_lock<std::shared_timed_mutex> lock(job_mutex_);

        info_ = { false, "Error adding comment: " + error.ToString() };
        EcalRecLogger::Instance()->error(info_.second);
      }

      return error;
    }

    Error RecordJob::DeleteMeasurement(bool omit_uploading_check)
    {
      Error error(Error::OK);


      {
        std::unique_lock<std::shared_timed_mutex> lock(job_mutex_);
        UpdateJobState_NoLock();

        // Check if we can safely delete the directory
        if (is_deleted_)
          error = Error::ErrorCode::MEAS_IS_DELETED;
        else if ((main_recorder_state_ == JobState::Recording) || (AnyAddonStateIs_NoLock(RecAddonJobStatus::State::Recording)))
          error = Error::CURRENTLY_RECORDING;
        else if ((main_recorder_state_ == JobState::Flushing) || (AnyAddonStateIs_NoLock(RecAddonJobStatus::State::Flushing)))
          error = Error::CURRENTLY_FLUSHING;
        else if (!omit_uploading_check && (main_recorder_state_ == JobState::Uploading))
          error = Error::CURRENTLY_UPLOADING;
        else if (!EcalUtils::Filesystem::FileStatus(job_config_.GetCompleteMeasurementPath()).IsOk())
          error = Error(Error::RESOURCE_UNAVAILABLE, job_config_.GetCompleteMeasurementPath());
        else if (!safe_to_delete_dir_)
          error = Error(Error::DIR_NOT_EMPTY, "\"" + job_config_.GetCompleteMeasurementPath() + "\" already contained files when the measurement was started");
      }

      if (!error)
      {
        bool success = EcalUtils::Filesystem::DeleteDir(job_config_.GetCompleteMeasurementPath());
        if (!success)
        {
          error = Error(Error::ErrorCode::GENERIC_ERROR, "Failed deleting \"" + job_config_.GetCompleteMeasurementPath() + "\"");
        }
      }

      {
        std::unique_lock<std::shared_timed_mutex> lock(job_mutex_);

        if (error)
          info_ = { false, "Error deleting measurement: " + error.ToString() };
        else
          is_deleted_ = true;
      }

      return error;
    }

    ///////////////////////////////////////////////
    // Status API
    ///////////////////////////////////////////////
    const JobConfig& RecordJob::GetJobConfig() const
    {
      return job_config_;
    }

    JobStatus RecordJob::GetJobStatus() const
    {
      JobStatus job_status;
      job_status.job_id_     = job_config_.GetJobId();
      job_status.is_deleted_ = is_deleted_;

      UpdateJobState(); // Peforms unique lock on job_mutex_

      {
        std::shared_lock<std::shared_timed_mutex> lock(job_mutex_);
        job_status.state_ = main_recorder_state_;

        if (hdf5_writer_thread_)
        {
          job_status.rec_hdf5_status_ = hdf5_writer_thread_->GetStatus();
          if (job_status.rec_hdf5_status_.info_.first)
          {
            job_status.rec_hdf5_status_.info_ = info_;
          }
        }

        job_status.rec_addon_statuses_ = rec_addon_statuses_;

#ifdef ECAL_HAS_CURL
        if (ftp_upload_thread_)
          job_status.upload_status_ = ftp_upload_thread_->GetStatus();
#endif // ECAL_HAS_CURL
      }

      return job_status;
    }

    JobState RecordJob::GetMainRecorderState() const
    {
      UpdateJobState(); // Peforms unique lock on job_mutex_

      std::shared_lock<std::shared_timed_mutex> lock(job_mutex_);
      return main_recorder_state_;
    }

    bool RecordJob::IsDeleted() const
    {
      std::shared_lock<std::shared_timed_mutex> lock(job_mutex_);
      return is_deleted_;
    }

    void RecordJob::SetAddonStatus(const std::string& addon, const RecAddonJobStatus& status)
    {
      std::unique_lock<std::shared_timed_mutex> lock(job_mutex_);
      rec_addon_statuses_[addon] = status;
    }

    bool RecordJob::AnyAddonStateIs_NoLock(RecAddonJobStatus::State state) const
    {
        for (const auto& rec_addon_job_status : rec_addon_statuses_)
        {
          if (rec_addon_job_status.second.state_ == state)
            return true;
        }
        return false;
    }


    ///////////////////////////////////////////////
    // Private helper methods
    ///////////////////////////////////////////////
    void RecordJob::UpdateJobState_NoLock() const
    {
      if (main_recorder_state_ == JobState::Flushing)
      {
        // Flushing -> FinishedFlushing, if recorder finished flushing.
        if (hdf5_writer_thread_
          && (!hdf5_writer_thread_->IsRunning() || !hdf5_writer_thread_->IsFlushing()))
        {
          main_recorder_state_ = JobState::FinishedFlushing;
        }
      }
      else if (main_recorder_state_ == JobState::Uploading)
      {
        // Uploading -> FinishedUploading, if the uploading thread has terminated

#ifdef ECAL_HAS_CURL
        if (ftp_upload_thread_ && !ftp_upload_thread_->IsRunning())
        {
          main_recorder_state_ = JobState::FinishedUploading;
        }
#endif //ECAL_HAS_CURL
      }
    }

    void RecordJob::UpdateJobState() const
    {
      std::unique_lock<std::shared_timed_mutex> lock(job_mutex_);
      UpdateJobState_NoLock();
    }
  }
}
