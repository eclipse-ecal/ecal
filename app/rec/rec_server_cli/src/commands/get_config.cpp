/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
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

#include "get_config.h"

#include <iostream>
#include <iomanip>
#include <sstream>

#include <ecal_utils/string.h>
#include <ecal_utils/filesystem.h>

#include <rec_server_core/proto_helpers.h>

#include "table_printer.h"

namespace eCAL
{
  namespace rec_cli
  {
    namespace command
    {
      //////////////////////////////////////////////
      /// Command functions (-> overrides)
      //////////////////////////////////////////////
 
      std::string GetConfig::Usage() const
      {
        return "";
      }

      std::string GetConfig::Help() const
      {
        return "Prints the configuration of the recorder.";
      }

      std::string GetConfig::Example() const
      {
        return "";
      }

      eCAL::rec::Error GetConfig::Execute(const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::vector<std::string>& /*argv*/) const
      {
        return PrintConfig(rec_server_instance->GetConfig());
      }

      eCAL::rec::Error GetConfig::Execute(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::vector<std::string>& /*argv*/) const
      {
        eCAL::rec_server::RecServerConfig config;
        eCAL::rec::Error error = GetRemoteConfig(hostname, remote_rec_server_service, config);

        if (error)
          return error;
        else
          return PrintConfig(config);
      }

      eCAL::rec::Error GetConfig::PrintConfig(const eCAL::rec_server::RecServerConfig& config) const
      {
        std::stringstream ss;

        // Meas dir
        {
          ss << "Meas dir:            " << config.root_dir_ << std::endl;
        }

        // Meas name
        {
          ss << "Meas name:           " << config.meas_name_ << std::endl;
        }

        // Max file size
        {
          ss << "Max hdf5 file size:  "  << config.max_file_size_ << " MiB" << std::endl;
        }

        // Description
        {
          ss << "Description:" << std::endl;

          std::vector<std::string> lines;
          EcalUtils::String::Split(config.description_, "\n", lines);

          for (std::string& line : lines)
          {
            ss << "  ";
            if (!line.empty() && line[line.size() - 1] == '\r')
              ss << line.substr(0, line.size() - 2);
            else
              ss << line;
            ss << std::endl;
          }
        }

        // Pre buffer enabled & size
        {
          ss << "Pre-buffer:          ";
          if (config.pre_buffer_enabled_)
          {
            double pre_buffer_max_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(config.pre_buffer_length_).count();
            ss << pre_buffer_max_seconds << " secs" << std::endl;
          }
          else
          {
            ss << "Disabled" << std::endl;
          }
        }

        // Blacklist / Whitelist / All topics
        {
          auto record_mode = config.record_mode_;
          ss << "Record mode:         ";
          if (record_mode == eCAL::rec::RecordMode::Blacklist)
          {
            ss << "Blacklist" << std::endl;
            ss << "Blacklisted topics:" << std::endl;
          }
          else if (record_mode == eCAL::rec::RecordMode::Whitelist)
          {
            ss << "Whitelist" << std::endl;
            ss << "Whitelisted topics:" << std::endl;
          }
          else if (record_mode == eCAL::rec::RecordMode::All)
          {
            ss << "All Topics" << std::endl;
          }

          if (record_mode != eCAL::rec::RecordMode::All)
          {
            // Print topic list
            auto topic_list = config.listed_topics_;
            if (topic_list.empty())
            {
              ss << "  -- No topics --" << std::endl;
            }
            else
            {
              for (const std::string& topic_name : topic_list)
              {
                ss << "  " << topic_name << std::endl;
              }
            }
          }
        }

        // Upload Settings
        {
          ss << "Upload to:           ";
          auto upload_config = config.upload_config_;

          if (upload_config.type_ == eCAL::rec_server::UploadConfig::Type::INTERNAL_FTP)
          {
            ss << "Internal FTP" << std::endl;
          }
          else if (upload_config.type_ == eCAL::rec_server::UploadConfig::Type::FTP)
          {
            ss << "ftp://" + upload_config.username_ + ":" + upload_config.password_ + "@" + upload_config.host_ + ":" + std::to_string(upload_config.port_)
              + EcalUtils::Filesystem::CleanPath("/" + upload_config.root_path_ + "/", EcalUtils::Filesystem::Unix) << std::endl;;
          }

          ss << "Delete after upload: " << (upload_config.delete_after_upload_ ? "Yes" : "No") << std::endl;
        }

        // Built-in recorder enabled
        {
          ss << "Use built-in client: " << (config.built_in_recorder_enabled_ ? "Yes" : "No") << std::endl;
        }


        // Clients
        {
          for (const auto& client : config.enabled_clients_config_)
          {
            ss << std::endl;
            ss << "Client: " << client.first << std::endl;
            ss << "  Hosts: ";
            if (client.second.host_filter_.empty())
              ss << "-- All --" << std::endl;
            else
              ss << EcalUtils::String::Join(", ", client.second.host_filter_) << std::endl;

            ss << "  Addons:" << std::endl;

            if (client.second.enabled_addons_.empty())
            {
              ss << "    -- None --" << std::endl;
            }
            else
            {
              for (const auto& addon_id_string : client.second.enabled_addons_)
              {
                ss << "    " << addon_id_string << std::endl;
              }
            }
          }
        }

        std::cout << ss.str();

        return eCAL::rec::Error::ErrorCode::OK;
      }
    }
  }
}
