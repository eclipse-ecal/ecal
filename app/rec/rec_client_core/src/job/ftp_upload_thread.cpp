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

#include <algorithm>
#include <vector>
#include <fstream>
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <array>
#include <ctime>

#ifdef ECAL_OS_LINUX
#include <unistd.h>
#endif

#include <ecal_utils/filesystem.h>
#include <ecal_utils/str_convert.h>

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

    namespace
    {
      // Lookup table to tell us which character we must escape for a proper curl URI
      // Information on URI are here: https://www.ietf.org/rfc/rfc3986.txt
      // We don't escape codepage characters, as this would break UTF-8
      constexpr std::array<bool, 256> is_reserved_
      {
        true,   //  NUL     (0x00)
        true,   //  SOH     (0x01)
        true,   //  STX     (0x02)
        true,   //  ETX     (0x03)
        true,   //  EOT     (0x04)
        true,   //  ENQ     (0x05)
        true,   //  ACK     (0x06)
        true,   //  BEL     (0x07)
        true,   //  BS      (0x08)
        true,   //  HT      (0x09)
        true,   //  LF      (0x0A)
        true,   //  VT      (0x0B)
        true,   //  FF      (0x0C)
        true,   //  CR      (0x0D)
        true,   //  SO      (0x0E)
        true,   //  SI      (0x0F)
        true,   //  DLE     (0x10)
        true,   //  DC1     (0x11)
        true,   //  DC2     (0x12)
        true,   //  DC3     (0x13)
        true,   //  DC4     (0x14)
        true,   //  NAK     (0x15)
        true,   //  SYN     (0x16)
        true,   //  ETB     (0x17)
        true,   //  CAN     (0x18)
        true,   //  EM      (0x19)
        true,   //  SUB     (0x1A)
        true,   //  ESC     (0x1B)
        true,   //  FS      (0x1C)
        true,   //  GS      (0x1D)
        true,   //  RS      (0x1E)
        true,   //  US      (0x1F)
        true,   //  SP      (0x20)
        true,   //  !       (0x21)
        true,   //  "       (0x22)
        true,   //  #       (0x23)
        true,   //  $       (0x24)
        true,   //  %       (0x25)
        true,   //  &       (0x26)
        true,   //  '       (0x27)
        true,   //  (       (0x28)
        true,   //  )       (0x29)
        true,   //  *       (0x2A)
        true,   //  +       (0x2B)
        true,   //  ,       (0x2C)
        false,  //  -       (0x2D)
        false,  //  .       (0x2E)
        false,  //  /       (0x2F)
        false,  //  0       (0x30)
        false,  //  1       (0x31)
        false,  //  2       (0x32)
        false,  //  3       (0x33)
        false,  //  4       (0x34)
        false,  //  5       (0x35)
        false,  //  6       (0x36)
        false,  //  7       (0x37)
        false,  //  8       (0x38)
        false,  //  9       (0x39)
        true,   //  :       (0x3A)
        true,   //  ;       (0x3B)
        true,   //  <       (0x3C)
        true,   //  =       (0x3D)
        true,   //  >       (0x3E)
        true,   //  ?       (0x3F)
        true,   //  @       (0x40)
        false,  //  A       (0x41)
        false,  //  B       (0x42)
        false,  //  C       (0x43)
        false,  //  D       (0x44)
        false,  //  E       (0x45)
        false,  //  F       (0x46)
        false,  //  G       (0x47)
        false,  //  H       (0x48)
        false,  //  I       (0x49)
        false,  //  J       (0x4A)
        false,  //  K       (0x4B)
        false,  //  L       (0x4C)
        false,  //  M       (0x4D)
        false,  //  N       (0x4E)
        false,  //  O       (0x4F)
        false,  //  P       (0x50)
        false,  //  Q       (0x51)
        false,  //  R       (0x52)
        false,  //  S       (0x53)
        false,  //  T       (0x54)
        false,  //  U       (0x55)
        false,  //  V       (0x56)
        false,  //  W       (0x57)
        false,  //  X       (0x58)
        false,  //  Y       (0x59)
        false,  //  Z       (0x5A)
        true,   //  [       (0x5B)
        true,   //  \       (0x5C)
        true,   //  ]       (0x5D)
        true,   //  ^       (0x5E)
        false,  //  _       (0x5F)
        true,   //  `       (0x60)
        false,  //  a       (0x61)
        false,  //  b       (0x62)
        false,  //  c       (0x63)
        false,  //  d       (0x64)
        false,  //  e       (0x65)
        false,  //  f       (0x66)
        false,  //  g       (0x67)
        false,  //  h       (0x68)
        false,  //  i       (0x69)
        false,  //  j       (0x6A)
        false,  //  k       (0x6B)
        false,  //  l       (0x6C)
        false,  //  m       (0x6D)
        false,  //  n       (0x6E)
        false,  //  o       (0x6F)
        false,  //  p       (0x70)
        false,  //  q       (0x71)
        false,  //  r       (0x72)
        false,  //  s       (0x73)
        false,  //  t       (0x74)
        false,  //  u       (0x75)
        false,  //  v       (0x76)
        false,  //  w       (0x77)
        false,  //  x       (0x78)
        false,  //  y       (0x79)
        false,  //  z       (0x7A)
        true,   //  {       (0x7B)
        true,   //  |       (0x7C)
        true,   //  }       (0x7D)
        false,  //  ~       (0x7E)
        true,   //  DEL     (0x7F)
        false,  // Codepage (0x80)
        false,  // Codepage (0x81)
        false,  // Codepage (0x82)
        false,  // Codepage (0x83)
        false,  // Codepage (0x84)
        false,  // Codepage (0x85)
        false,  // Codepage (0x86)
        false,  // Codepage (0x87)
        false,  // Codepage (0x88)
        false,  // Codepage (0x89)
        false,  // Codepage (0x8A)
        false,  // Codepage (0x8B)
        false,  // Codepage (0x8C)
        false,  // Codepage (0x8D)
        false,  // Codepage (0x8E)
        false,  // Codepage (0x8F)
        false,  // Codepage (0x90)
        false,  // Codepage (0x91)
        false,  // Codepage (0x92)
        false,  // Codepage (0x93)
        false,  // Codepage (0x94)
        false,  // Codepage (0x95)
        false,  // Codepage (0x96)
        false,  // Codepage (0x97)
        false,  // Codepage (0x98)
        false,  // Codepage (0x99)
        false,  // Codepage (0x9A)
        false,  // Codepage (0x9B)
        false,  // Codepage (0x9C)
        false,  // Codepage (0x9D)
        false,  // Codepage (0x9E)
        false,  // Codepage (0x9F)
        false,  // Codepage (0xA0)
        false,  // Codepage (0xA1)
        false,  // Codepage (0xA2)
        false,  // Codepage (0xA3)
        false,  // Codepage (0xA4)
        false,  // Codepage (0xA5)
        false,  // Codepage (0xA6)
        false,  // Codepage (0xA7)
        false,  // Codepage (0xA8)
        false,  // Codepage (0xA9)
        false,  // Codepage (0xAA)
        false,  // Codepage (0xAB)
        false,  // Codepage (0xAC)
        false,  // Codepage (0xAD)
        false,  // Codepage (0xAE)
        false,  // Codepage (0xAF)
        false,  // Codepage (0xB0)
        false,  // Codepage (0xB1)
        false,  // Codepage (0xB2)
        false,  // Codepage (0xB3)
        false,  // Codepage (0xB4)
        false,  // Codepage (0xB5)
        false,  // Codepage (0xB6)
        false,  // Codepage (0xB7)
        false,  // Codepage (0xB8)
        false,  // Codepage (0xB9)
        false,  // Codepage (0xBA)
        false,  // Codepage (0xBB)
        false,  // Codepage (0xBC)
        false,  // Codepage (0xBD)
        false,  // Codepage (0xBE)
        false,  // Codepage (0xBF)
        false,  // Codepage (0xC0)
        false,  // Codepage (0xC1)
        false,  // Codepage (0xC2)
        false,  // Codepage (0xC3)
        false,  // Codepage (0xC4)
        false,  // Codepage (0xC5)
        false,  // Codepage (0xC6)
        false,  // Codepage (0xC7)
        false,  // Codepage (0xC8)
        false,  // Codepage (0xC9)
        false,  // Codepage (0xCA)
        false,  // Codepage (0xCB)
        false,  // Codepage (0xCC)
        false,  // Codepage (0xCD)
        false,  // Codepage (0xCE)
        false,  // Codepage (0xCF)
        false,  // Codepage (0xD0)
        false,  // Codepage (0xD1)
        false,  // Codepage (0xD2)
        false,  // Codepage (0xD3)
        false,  // Codepage (0xD4)
        false,  // Codepage (0xD5)
        false,  // Codepage (0xD6)
        false,  // Codepage (0xD7)
        false,  // Codepage (0xD8)
        false,  // Codepage (0xD9)
        false,  // Codepage (0xDA)
        false,  // Codepage (0xDB)
        false,  // Codepage (0xDC)
        false,  // Codepage (0xDD)
        false,  // Codepage (0xDE)
        false,  // Codepage (0xDF)
        false,  // Codepage (0xE0)
        false,  // Codepage (0xE1)
        false,  // Codepage (0xE2)
        false,  // Codepage (0xE3)
        false,  // Codepage (0xE4)
        false,  // Codepage (0xE5)
        false,  // Codepage (0xE6)
        false,  // Codepage (0xE7)
        false,  // Codepage (0xE8)
        false,  // Codepage (0xE9)
        false,  // Codepage (0xEA)
        false,  // Codepage (0xEB)
        false,  // Codepage (0xEC)
        false,  // Codepage (0xED)
        false,  // Codepage (0xEE)
        false,  // Codepage (0xEF)
        false,  // Codepage (0xF0)
        false,  // Codepage (0xF1)
        false,  // Codepage (0xF2)
        false,  // Codepage (0xF3)
        false,  // Codepage (0xF4)
        false,  // Codepage (0xF5)
        false,  // Codepage (0xF6)
        false,  // Codepage (0xF7)
        false,  // Codepage (0xF8)
        false,  // Codepage (0xF9)
        false,  // Codepage (0xFA)
        false,  // Codepage (0xFB)
        false,  // Codepage (0xFC)
        false,  // Codepage (0xFD)
        false,  // Codepage (0xFE)
        false,  // Codepage (0xFF)
      };
    }

    /////////////////////////////////////////////
    // InterruptibleThread overrides
    /////////////////////////////////////////////

    FtpUploadThread::FtpUploadThread(const std::string&                local_root_dir
                                   , const std::string&                ftp_server
                                   , const std::string&                ftp_root_dir
                                   , const std::vector<std::string>&   skip_files
                                   , const std::function<Error(void)>& after_successfull_upload_function)
      : curl_handle                       (nullptr)
      , local_root_dir_                   (local_root_dir)
      , ftp_server_                       (ftp_server)
      , ftp_root_dir_                     (ftp_root_dir)
      , skip_files_                       (skip_files)
      , finished_files_progress_          {}
      , current_file_size_bytes_          (0)
      , current_file_uploaded_bytes_      (0)
      , info_                             { true, "" }
      , num_file_upload_errors_           (0)
      , post_upload_function_(after_successfull_upload_function)
    {
      finished_files_progress_.num_total_files_    = 0;
      finished_files_progress_.num_complete_files_ = 0;
      finished_files_progress_.bytes_total_        = 0;
      finished_files_progress_.bytes_completed_    = 0;

      for(size_t i = 0; i < skip_files_.size(); i++)
      {
        skip_files_[i] = EcalUtils::Filesystem::CleanPath(local_root_dir_ + "/" + skip_files_[i], EcalUtils::Filesystem::OsStyle::Current);
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

      auto root_dir_status = EcalUtils::Filesystem::FileStatus(local_root_dir_, EcalUtils::Filesystem::OsStyle::Current);
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
          std::string normalized_file_to_upload = EcalUtils::Filesystem::CleanPath(local_root_dir_ + "/" + file_pair_to_upload.first, EcalUtils::Filesystem::OsStyle::Current);
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

        std::string local_complete_file_path = EcalUtils::Filesystem::CleanPath(EcalUtils::Filesystem::ToNativeSeperators(local_root_dir_ + "/" + file_path, EcalUtils::Filesystem::OsStyle::Current), EcalUtils::Filesystem::OsStyle::Current);
        
        std::string file_name_only = EcalUtils::Filesystem::CleanPathComponentList(file_path, EcalUtils::Filesystem::OsStyle::Current).back();
        std::string temporary_file_name_only = EcalUtils::Filesystem::CleanPathComponentList(temporary_file_path, EcalUtils::Filesystem::OsStyle::Current).back();

#ifndef _NDEBUG
        EcalRecLogger::Instance()->debug("Uploading File: " + local_complete_file_path);
#endif // !_NDEBUG

        // Open the local file
        std::ifstream file;
#ifdef WIN32
        std::wstring w_native_path = EcalUtils::StrConvert::Utf8ToWide(EcalUtils::Filesystem::ToNativeSeperators(local_complete_file_path, EcalUtils::Filesystem::OsStyle::Current));
        file.open(w_native_path, std::ios::binary);
#else
        file.open(EcalUtils::Filesystem::ToNativeSeperators(local_complete_file_path, EcalUtils::Filesystem::OsStyle::Current), std::ios::binary);
#endif // WIN32

        bool abort_uploading = false;

        if (!file.is_open())
        {
          const std::string error_string = "Error uploading " + local_complete_file_path + ": Unable to open file.";
          logError(error_string);
        }
        else
        {
          // Use our own read function (mandatory on Windows)
          ReadCallbackParams read_callback_params{};
          read_callback_params.this_ = this;
          read_callback_params.file_ = &file;
          curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, FtpUploadThread::ReadCallback);
          curl_easy_setopt(curl_handle, CURLOPT_READDATA, &read_callback_params);

          // enable uploading
          curl_easy_setopt(curl_handle, CURLOPT_UPLOAD, 1L);

          // specify target path
          std::string target_path;
          target_path.reserve(ftp_server_.size() + ftp_root_dir_.size() + temporary_file_path.size() * 3 + 1);
          target_path += ftp_server_;
          target_path += ftp_root_dir_;
          for (char c : temporary_file_path)
          {
            if (is_reserved_.at(static_cast<unsigned char>(c)))
            {
              target_path += "%xx";
              std::snprintf(&target_path[target_path.size() - 2], 3, "%02X", c);
            }
            else
            {
              target_path += c;
            }
          }
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
            std::string error_string = "Error uploading: " + std::string(curl_easy_strerror(res)) + " (" + local_complete_file_path + ")";
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

            // Check if we want to abort uploading. For instance if the hostname
            // cannot be resolved, we don't try any further.
            if ((res == CURLE_UNSUPPORTED_PROTOCOL)
              || (res == CURLE_NOT_BUILT_IN)
              || (res == CURLE_COULDNT_RESOLVE_PROXY)
              || (res == CURLE_COULDNT_RESOLVE_HOST)
              || (res == CURLE_COULDNT_CONNECT)
              || (res == CURLE_REMOTE_DISK_FULL)
#if (LIBCURL_VERSION_MAJOR >= 7) && (LIBCURL_VERSION_MINOR >= 66)
              || (res == CURLE_AUTH_ERROR)
#endif
              || (res == CURLE_ABORTED_BY_CALLBACK)
              || (res == CURLE_LOGIN_DENIED))
            {
              abort_uploading = true;
            }
          }

          // Clean up the command list
          curl_slist_free_all(curl_command_list);

          file.close();

          // Update statistics
          {
            std::lock_guard<std::mutex> progress_lock(progress_mutex_);
            finished_files_progress_.num_complete_files_++;

            if (res == CURLE_OK)
              finished_files_progress_.bytes_completed_ += file_info.second;
            else
              finished_files_progress_.bytes_completed_ += current_file_uploaded_bytes_;

            current_file_size_bytes_ = 0;
            current_file_uploaded_bytes_ = 0;
          }

  #ifndef _NDEBUG
          EcalRecLogger::Instance()->debug("Finished uploading " + file_name_only);
  #endif // !_NDEBUG
        }

        if (abort_uploading) break;
      }

      curl_easy_cleanup(curl_handle);

      EcalRecLogger::Instance()->info("Finished uploading.");
      
      if (IsInterrupted()) return;
      if (!info_.first)    return; // Only execute post-upload function if everything is OK

      Error post_upload_function_error = post_upload_function_();
      if (post_upload_function_error)
      {
        std::string info_string = "Post upload error: " + post_upload_function_error.ToString();
        EcalRecLogger::Instance()->error(info_string);

        std::lock_guard<std::mutex> progress_lock(progress_mutex_);
        info_ = std::make_pair(false, info_string);
      }
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

      auto dir_content = EcalUtils::Filesystem::DirContent(root_dir, EcalUtils::Filesystem::OsStyle::Current);

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
      std::array<char, 64> time_char_array{};
      size_t bytes_written = std::strftime(&time_char_array.front(), time_char_array.size(), "%F_%H-%M-%S", localtime(&time_t_now));

      // On error, 0 is returned. Should never happen.
      if (bytes_written == 0)
      {
        time_char_array.front() = 0;
      }


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

      std::array<char, 1024> hostname_char_array{};
      gethostname(&hostname_char_array.front(), static_cast<int>(hostname_char_array.size()));
      hostname_char_array.back() = 0;    // When the hostname was too long, there may be no terminating null-byte.

      return std::string(".") + &hostname_char_array.front() + "_" + &time_char_array.front() + ".tmp";
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
