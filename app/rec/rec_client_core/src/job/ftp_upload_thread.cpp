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

#include "ftp_upload_thread.h"

#include <vector>
#include <fstream>
#include <iostream>
#include <chrono>
#include <cstdlib>

#include <ecal_utils/filesystem.h>

#ifdef WIN32
  #define WIN32_LEAN_AND_MEAN
  #define NOMINMAX
  #include <Windows.h>
  #include <WinSock2.h>

  #include <cctype>
#elif __linux__
  #include <unistd.h>
#endif

#include <rec_client_core/ecal_rec_logger.h>

namespace eCAL
{
  namespace rec
  {
    /////////////////////////////////////////////
    // InterruptibleThread overrides
    /////////////////////////////////////////////

    FtpUploadThread::FtpUploadThread(const std::string& local_root_dir, const std::string& ftp_server, const std::string& ftp_root_dir, const std::vector<std::string>& skip_files)
      : curl_handle                 (nullptr)
      , local_root_dir_             (local_root_dir)
      , ftp_server_                 (ftp_server)
      , ftp_root_dir_               (ftp_root_dir)
      , skip_files_                 (skip_files)
      , current_file_size_bytes_    (0)
      , current_file_uploaded_bytes_(0)
      , info_                       { true, "" }
      , num_file_upload_errors_     (0)
    {
      finished_files_progress_.num_total_files_    = 0;
      finished_files_progress_.num_complete_files_ = 0;
      finished_files_progress_.bytes_total_        = 0;
      finished_files_progress_.bytes_completed_    = 0;

      for(size_t i = 0; i < skip_files_.size(); i++)
      {
        skip_files_[i] = EcalUtils::Filesystem::CleanPath(local_root_dir_ + "/" + skip_files_[i]);
#ifdef WIN32
        // On windows we lower-case-compare files
        std::transform(skip_files_[i].begin(), skip_files_[i].end(), skip_files_[i].begin(), [](unsigned char c) { return static_cast<unsigned char>(std::tolower(c)); });
#endif // WIN32
      }
    }

    FtpUploadThread::~FtpUploadThread()
    {}

    void FtpUploadThread::Run()
    {
      // Create a list of all files to upload
      EcalRecLogger::Instance()->info("Scanning directory for upload: " + local_root_dir_);

      auto root_dir_status = EcalUtils::Filesystem::FileStatus(local_root_dir_);
      if (!root_dir_status.IsOk() || (root_dir_status.GetType() != EcalUtils::Filesystem::Type::Dir))
      {
        info_ = { false, "Resource unavailable (" + local_root_dir_ + ")"};
        return;
      }

      auto files_to_upload = CreateFileList(local_root_dir_);

      {
        std::lock_guard<std::mutex> progress_lock(progress_mutex_);
        finished_files_progress_.num_total_files_ = files_to_upload.size();

        // Remove files that should not be uploaded
        files_to_upload.remove_if([this](const std::pair<std::string, uint64_t>& file_pair_to_upload) -> bool
        {
          std::string normalized_file_to_upload = EcalUtils::Filesystem::CleanPath(local_root_dir_ + "/" + file_pair_to_upload.first);
#ifdef WIN32
          // On windows we lower-case-compare files
          std::transform(normalized_file_to_upload.begin(), normalized_file_to_upload.end(), normalized_file_to_upload.begin(), [](unsigned char c) { return static_cast<unsigned char>(std::tolower(c)); });
#endif // WIN32

          return (std::find(skip_files_.begin(), skip_files_.end(), normalized_file_to_upload) != skip_files_.end());
        });

        // Count bytes for statistics
        for (const auto& file_info : files_to_upload)
        {
          finished_files_progress_.bytes_total_ += file_info.second;
        }

        EcalRecLogger::Instance()->info("Found " + std::to_string(finished_files_progress_.num_total_files_) + " files with total " + std::to_string(finished_files_progress_.bytes_total_) + " bytes.");
      }

      // Create a suffix for our temporary files
      std::string temp_suffix = CreateTempSuffix();

      if (IsInterrupted()) return;

      curl_handle = curl_easy_init();

      if (!curl_handle)
        return;

      // Get the ftp_proxy environment variable. It may be usefull, but it may
      // also be set by the user on accident.
      std::string ftp_proxy;
      {
        char* ftp_proxy_charp = std::getenv("ftp_proxy");
        if (ftp_proxy_charp != nullptr)
        {
          ftp_proxy = std::string(ftp_proxy_charp);
        }
      }
      
      EcalRecLogger::Instance()->info("Start uploading to " + ftp_server_ + "/" + ftp_root_dir_);

      for (const auto& file_info : files_to_upload)
      {
        // Save statistics
        {
          std::lock_guard<std::mutex> progress_lock(progress_mutex_);
          current_file_size_bytes_ = file_info.second;
        }

        if (IsInterrupted()) return;

        std::string file_path = file_info.first;

        std::string temporary_file_path = file_path + temp_suffix;

        std::string local_complete_file_path = EcalUtils::Filesystem::CleanPath(EcalUtils::Filesystem::ToNativeSeperators(local_root_dir_ + "/" + file_path));
        
        std::string file_name_only = EcalUtils::Filesystem::CleanPathComponentList(file_path).back();
        std::string temporary_file_name_only = EcalUtils::Filesystem::CleanPathComponentList(temporary_file_path).back();

#ifndef _NDEBUG
        EcalRecLogger::Instance()->debug("Uploading File: " + local_complete_file_path);
#endif // !_NDEBUG

        // Open the local file
        std::ifstream file;
        file.open(EcalUtils::Filesystem::ToNativeSeperators(local_complete_file_path), std::ios::binary);

        if (!file.is_open())
        {
          const std::string error_string = "Error uploading " + local_complete_file_path + ": Unable to open file.";
          logError(error_string);
        }
        else
        {
          // Use our own read function (mandatory on Windows)
          ReadCallbackParams read_callback_params;
          read_callback_params.this_ = this;
          read_callback_params.file_ = &file;
          curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, FtpUploadThread::ReadCallback);
          curl_easy_setopt(curl_handle, CURLOPT_READDATA, &read_callback_params);

          // enable uploading
          curl_easy_setopt(curl_handle, CURLOPT_UPLOAD, 1L);

          // specify target path
          std::string target_path = (ftp_server_ + ftp_root_dir_ + temporary_file_path);
          curl_easy_setopt(curl_handle, CURLOPT_URL, target_path.c_str());
          curl_easy_setopt(curl_handle, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);

          // Create a command list for renaming the file back to the original name
          std::vector<std::string> command_list
          {
            "RNFR " + temporary_file_name_only,
            "RNTO " + file_name_only
          };
          struct curl_slist *curl_command_list = nullptr;
          for (size_t i = 0; i < command_list.size(); i++)
          {
            curl_command_list = curl_slist_append(curl_command_list, command_list[i].c_str());
          }

          curl_easy_setopt(curl_handle, CURLOPT_POSTQUOTE, curl_command_list);

          // Set a progress callback
          curl_easy_setopt(curl_handle, CURLOPT_XFERINFOFUNCTION, ProgressCallback);
          curl_easy_setopt(curl_handle, CURLOPT_XFERINFODATA, this);
          curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);

          // Start the FTP Session and upload!
          CURLcode res = curl_easy_perform(curl_handle);

          if (res != CURLE_OK)
          {
            std::string error_string = "Error uploading " + local_complete_file_path + ": " + curl_easy_strerror(res);
            if (!ftp_proxy.empty())
            {
              error_string += " [WARNING: Using ftp_proxy=" + ftp_proxy + "]";
            }
              
            EcalRecLogger::Instance()->error(error_string);

            std::lock_guard<std::mutex> progress_lock(progress_mutex_);

            num_file_upload_errors_++;

            info_.first = false;
            if (num_file_upload_errors_ > 1)
            {
              info_.second = "[" + std::to_string(num_file_upload_errors_) + " Errors] Last error: ";
            }
            else
            {
              info_.second.clear();
            }
          
            info_.second += error_string;
          }

          // Clean up the command list
          curl_slist_free_all(curl_command_list);

          file.close();

          // Update statistics
          {
            std::lock_guard<std::mutex> progress_lock(progress_mutex_);
            finished_files_progress_.num_complete_files_++;
            finished_files_progress_.bytes_completed_ += file_info.second;

            current_file_size_bytes_ = 0;
            current_file_uploaded_bytes_ = 0;
          }

  #ifndef _NDEBUG
          EcalRecLogger::Instance()->debug("Finished uploading " + file_name_only);
  #endif // !_NDEBUG
        }
      }

      curl_easy_cleanup(curl_handle);

      EcalRecLogger::Instance()->info("Finished uploading.");
    }

    /////////////////////////////////////////////
    // API
    /////////////////////////////////////////////

    UploadStatus FtpUploadThread::GetStatus() const
    {
      UploadStatus upload_status;

      std::lock_guard<std::mutex> progress_lock(progress_mutex_);
      upload_status.bytes_total_size_ = finished_files_progress_.bytes_total_;
      upload_status.bytes_uploaded_   = finished_files_progress_.bytes_completed_ + current_file_uploaded_bytes_;
      upload_status.info_             = info_;

      return upload_status;
    }

    /////////////////////////////////////////////
    // Helper methods
    /////////////////////////////////////////////

    std::list<std::pair<std::string, unsigned long long>> FtpUploadThread::CreateFileList(const std::string& root_dir)
    {
      std::list<std::pair<std::string, unsigned long long>> files;

      auto dir_content = EcalUtils::Filesystem::DirContent(root_dir);

      for (const auto& entry : dir_content)
      {
        if (!entry.second.IsOk())
          continue;

        if (entry.second.GetType() == EcalUtils::Filesystem::Type::RegularFile)
        {
          // Just a top-level file
          files.emplace_back(std::make_pair(entry.first, (unsigned long long)(entry.second.FileSize())));
        }
        else if (entry.second.GetType() == EcalUtils::Filesystem::Type::Dir)
        {
          // We found a directory! Let's get the files of that and add the current directory to it
          auto nested_files = CreateFileList(root_dir + "/" + entry.first);
          for (const auto& nested_file : nested_files)
          {
            files.emplace_back(std::make_pair(entry.first + "/" + nested_file.first, nested_file.second));
          }
        }
      }

      return files;
    }

    std::string FtpUploadThread::CreateTempSuffix()
    {
      auto now = std::chrono::system_clock::now();
      time_t time_t_now = std::chrono::system_clock::to_time_t(now);
      char time_char[64];
      strftime(time_char, 64, "%F_%H-%M-%S", localtime(&time_t_now));

#ifdef WIN32
      WORD wVersionRequested = MAKEWORD(2, 2);

      WSADATA wsaData;
      int err = WSAStartup(wVersionRequested, &wsaData);
      if (err != 0)
      {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        printf("WSAStartup failed with error: %d\n", err);
      }
#endif // WIN32

      char hostname_char[1024] = { 0 };
      gethostname(hostname_char, 1024);

      return std::string(".") + hostname_char + "_" + time_char + ".tmp";
    }

    void FtpUploadThread::logError(const std::string& error_message)
    {
      EcalRecLogger::Instance()->error(error_message);

      std::lock_guard<std::mutex> progress_lock(progress_mutex_);

      num_file_upload_errors_++;

      info_.first = false;
      if (num_file_upload_errors_ > 1)
      {
        info_.second = "[" + std::to_string(num_file_upload_errors_) + " Errors] Last error: ";
      }
      else
      {
        info_.second.clear();
      }

      info_.second += error_message;

    }

    /////////////////////////////////////////////
    // Callbacks
    /////////////////////////////////////////////

    size_t FtpUploadThread::ReadCallback(char *buffer, size_t size, size_t nitems, void* read_callback_params)
    {
      ReadCallbackParams* params = static_cast<ReadCallbackParams*>(read_callback_params);

      if (params->this_->IsInterrupted()) return CURL_READFUNC_ABORT;

      params->file_->read(buffer, size * nitems);
      size_t bytes_read = static_cast<size_t>(params->file_->gcount());

      if (params->this_->IsInterrupted()) return CURL_READFUNC_ABORT;

      return bytes_read;
    }

    int FtpUploadThread::ProgressCallback(void *this_p, curl_off_t /*dltotal*/, curl_off_t /*dlnow*/, curl_off_t /*ultotal*/, curl_off_t ulnow)
    {
      static_cast<FtpUploadThread*>(this_p)->current_file_uploaded_bytes_ = ulnow;
      return 0;
    }
  }
}