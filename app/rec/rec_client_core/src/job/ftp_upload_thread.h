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

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <curl/curl.h>
#include <string>
#include <list>
#include <atomic>
#include <vector>

#include <rec_client_core/state.h>

namespace eCAL
{
  namespace rec
  {
    class FtpUploadThread : public InterruptibleThread
    {
    public:
      struct UploadProgress
      {
        size_t num_total_files_;
        size_t num_complete_files_;
        unsigned long long bytes_total_;
        unsigned long long bytes_completed_;
      };

      /////////////////////////////////////////////
      // InterruptibleThread overrides
      /////////////////////////////////////////////
    public:
      FtpUploadThread(const std::string& local_root_dir, const std::string& ftp_server, const std::string& ftp_root_dir, const std::vector<std::string>& skip_files);
      ~FtpUploadThread();

      void Run() override;

      /////////////////////////////////////////////
      // API
      /////////////////////////////////////////////
    public:
      UploadStatus GetStatus() const;

      /////////////////////////////////////////////
      // Helper methods
      /////////////////////////////////////////////
    private:
      static std::list<std::pair<std::string, unsigned long long>> CreateFileList(const std::string& root_dir);
      static std::string CreateTempSuffix();

      void logError(const std::string& error_message);

      /////////////////////////////////////////////
      // Callbacks
      /////////////////////////////////////////////
    private:
      struct ReadCallbackParams
      {
        FtpUploadThread* this_;
        std::ifstream* file_;
      };
      static size_t ReadCallback(char *buffer, size_t size, size_t nitems, void* read_callback_params);
      static int    ProgressCallback(void *this_p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

      /////////////////////////////////////////////
      // Member variables
      /////////////////////////////////////////////
    private:
      CURL* curl_handle;

      std::string              local_root_dir_;
      std::string              ftp_server_;
      std::string              ftp_root_dir_;
      std::vector<std::string> skip_files_;

      mutable std::mutex progress_mutex_;
      UploadProgress finished_files_progress_;
      unsigned long long current_file_size_bytes_;
      unsigned long long current_file_uploaded_bytes_;

      std::pair<bool, std::string> info_;
      int num_file_upload_errors_;
    };
  }
}