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

#include <map>
#include <string>
#include <set>
#include <chrono>

#include <rec_client_core/record_mode.h>

namespace eCAL
{
  namespace rec_server
  {
    struct UploadConfig
    {
      enum class Type
      {
        INTERNAL_FTP,
        FTP,
      };

      UploadConfig()
        : type_               (Type::INTERNAL_FTP)
        , host_               ("")
        , port_               (21)
        , username_           ("")
        , password_           ("")
        , root_path_          ("/")
        , delete_after_upload_(false)
      {}

      Type        type_;
      std::string host_;
      uint16_t    port_;
      std::string username_;
      std::string password_;
      std::string root_path_;
      bool        delete_after_upload_;
    };

    struct ClientConfig
    {
      std::set<std::string> host_filter_;
      std::set<std::string> enabled_addons_;
    };

    struct RecServerConfig
    {
      RecServerConfig()
        : root_dir_                 ("")
        , meas_name_                ("")
        , max_file_size_            (100)
        , description_              ("")
        , enabled_clients_config_   ()
        , pre_buffer_enabled_       (false)
        , pre_buffer_length_        (0)
        , built_in_recorder_enabled_(true)
        , record_mode_              (eCAL::rec::RecordMode::All)
        , listed_topics_            ()
        , upload_config_            ()
      {}

      std::string                         root_dir_;
      std::string                         meas_name_;
      int                                 max_file_size_;
      std::string                         description_;
      std::map<std::string, ClientConfig> enabled_clients_config_;
      bool                                pre_buffer_enabled_;
      std::chrono::steady_clock::duration pre_buffer_length_;
      bool                                built_in_recorder_enabled_;
      eCAL::rec::RecordMode               record_mode_;
      std::set<std::string>               listed_topics_;
      UploadConfig                        upload_config_;
    };
  }
}