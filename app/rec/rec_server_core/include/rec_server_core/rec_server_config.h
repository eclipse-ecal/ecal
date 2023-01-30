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
        , max_file_size_            (1000)
        , one_file_per_topic_       (false)
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
      int64_t                             max_file_size_;
      bool                                one_file_per_topic_;
      std::string                         description_;
      std::map<std::string, ClientConfig> enabled_clients_config_;
      bool                                pre_buffer_enabled_;
      std::chrono::steady_clock::duration pre_buffer_length_;
      bool                                built_in_recorder_enabled_;
      eCAL::rec::RecordMode               record_mode_;
      std::set<std::string>               listed_topics_;
      UploadConfig                        upload_config_;
    };

    ////////////////////////////////////
    // Operators
    ////////////////////////////////////

    inline bool operator==(const UploadConfig& lhs, const UploadConfig& rhs)
    {
      return  (lhs.type_                == rhs.type_) &&
              (lhs.host_                == rhs.host_) &&
              (lhs.port_                == rhs.port_) &&
              (lhs.username_            == rhs.username_) &&
              (lhs.password_            == rhs.password_) &&
              (lhs.root_path_           == rhs.root_path_) &&
              (lhs.delete_after_upload_ == rhs.delete_after_upload_);
    }

    inline bool operator==(const ClientConfig& lhs, const ClientConfig& rhs)
    {
      return (lhs.host_filter_  == rhs.host_filter_)
        && (lhs.enabled_addons_ == rhs.enabled_addons_);
    }

    inline bool operator==(const RecServerConfig& lhs, const RecServerConfig& rhs)
    {
      return  (lhs.root_dir_                  == rhs.root_dir_) &&
              (lhs.meas_name_                 == rhs.meas_name_) &&
              (lhs.max_file_size_             == rhs.max_file_size_) &&
              (lhs.one_file_per_topic_        == rhs.one_file_per_topic_) &&
              (lhs.description_               == rhs.description_) &&
              (lhs.enabled_clients_config_    == rhs.enabled_clients_config_) &&
              (lhs.pre_buffer_enabled_        == rhs.pre_buffer_enabled_) &&
              (lhs.pre_buffer_length_         == rhs.pre_buffer_length_) &&
              (lhs.built_in_recorder_enabled_ == rhs.built_in_recorder_enabled_) &&
              (lhs.record_mode_               == rhs.record_mode_) &&
              (lhs.listed_topics_             == rhs.listed_topics_) &&
              (lhs.upload_config_             == rhs.upload_config_);
    }

    inline bool operator!=(const UploadConfig& lhs, const UploadConfig& rhs)
    {
      return !(lhs == rhs);
    }

    inline bool operator!=(const ClientConfig& lhs, const ClientConfig& rhs)
    {
      return !(lhs == rhs);
    }

    inline bool operator!=(const RecServerConfig& lhs, const RecServerConfig& rhs)
    {
      return !(lhs == rhs);
    }
  }

}