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

#include "set_config.h"

#include <ecal_utils/string.h>
#include <ecal_utils/ecal_utils.h>


#include <clocale> // localeconv

#include <regex>

namespace eCAL
{
  namespace rec_cli
  {
    namespace command
    {
      //////////////////////////////////////////////
      /// Command functions (-> overrides)
      //////////////////////////////////////////////
 
      std::string SetConfig::Usage() const
      {
        CmdLine cmdline;
        std::vector<TCLAP::Arg*> arg_vector =
        {
          &cmdline.set_client_arg,
          &cmdline.set_addons_arg,
          &cmdline.remove_client_arg,
          &cmdline.pre_buffer_secs_arg,
          &cmdline.blacklist_arg,
          &cmdline.whitelist_arg,
          &cmdline.meas_root_dir_arg,
          &cmdline.meas_name_arg,
          &cmdline.max_file_size_mib_arg,
          &cmdline.description_arg,
          &cmdline.ftp_server_arg,
          &cmdline.delete_after_upload_arg,
          &cmdline.enable_built_in_client_arg,
          &cmdline.help_arg,
          &cmdline.unlabled_arg,
        };

        std::stringstream ss;
       
        for (size_t i = 0; i < arg_vector.size(); i++)
        {
          if (i != 0)
            ss << " | ";
          ss << arg_vector[i]->shortID();
        }

        return ss.str();
      }

      std::string SetConfig::Help() const
      {
        return "Sets a config parameter. Not available in remote-control mode. Multiple configuration settings can be changed with a single command. Tip: A more extensive help is available by --help.";
      }

      std::string SetConfig::Example() const
      {
        CmdLine cmdline;
        return "--" + cmdline.pre_buffer_secs_arg.getName() + " 3.5 " + "--" + cmdline.blacklist_arg.getName() + " \"Topic1,Topic2\"";
      }

      eCAL::rec::Error SetConfig::Execute(const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::vector<std::string>& argv) const
      {
        CmdLine cmdline;
        cmdline.parse(argv);


        if (cmdline.help_arg.isSet())
        {
          // Help
          std::cout << cmdline.help() << std::endl;
        }
        else
        {
          // Paramter validation
          if (cmdline.unlabled_arg.isSet() && !cmdline.unlabled_arg.getValue().empty())
          {
            std::cerr << "WARNING: Unrecognized parameters: " + EcalUtils::CommandLine::ToCommandLine(cmdline.unlabled_arg.getValue()) << std::endl;
          }

          if (cmdline.blacklist_arg.isSet() && cmdline.whitelist_arg.isSet())
          {
            return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::TOO_MANY_PARAMETERS, "Cannot use both blacklist and whitelist at the same time");
          }
          
          if (cmdline.blacklist_arg.isSet()
            || cmdline.whitelist_arg.isSet() 
            || cmdline.set_client_arg.isSet()
            || cmdline.set_addons_arg.isSet()
            || cmdline.remove_client_arg.isSet())
          {
            if (rec_server_instance->IsRecording())
              return eCAL::rec::Error::ErrorCode::CURRENTLY_RECORDING;
          }

          // Apply config
          
          // Set clients
          if (cmdline.set_client_arg.isSet())
          {
            auto error = SetClient(rec_server_instance, cmdline.set_client_arg.getValue());
            if (error)
              return error;
          }

          // Set addons
          if (cmdline.set_addons_arg.isSet())
          {
            auto error = SetAddons(rec_server_instance, cmdline.set_addons_arg.getValue());
            if (error)
              return error;
          }

          // remove clients
          if (cmdline.remove_client_arg.isSet())
          {
            auto error = RemoveClient(rec_server_instance, cmdline.remove_client_arg.getValue());
            if (error)
              return error;
          }

          // Ftp Server
          if (cmdline.ftp_server_arg.isSet())
          {
            auto error = setFtpServer(rec_server_instance, cmdline.ftp_server_arg.getValue());
            if (error)
              return error;
          }

          // Delete after upload
          if (cmdline.delete_after_upload_arg.isSet())
          {
            auto error = setDeleteAfterUpload(rec_server_instance, cmdline.delete_after_upload_arg.getValue());
            if (error)
              return error;
          }

          // Set built-in client enabled
          if (cmdline.enable_built_in_client_arg.isSet())
          {
            auto error = setBuiltInClientEnabled(rec_server_instance, cmdline.enable_built_in_client_arg.getValue());
            if (error)
              return error;
          }

          // Pre-buffer
          if (cmdline.pre_buffer_secs_arg.isSet())
          {
            auto error = setPreBuffer(rec_server_instance, cmdline.pre_buffer_secs_arg.getValue());
            if (error)
              return error;
          }

          // blacklist
          if (cmdline.blacklist_arg.isSet())
          {
            auto error = setBlacklist(rec_server_instance, cmdline.blacklist_arg.getValue());
            if (error)
              return error;
          }

          // whitelist
          if (cmdline.whitelist_arg.isSet())
          {
            auto error = setWhitelist(rec_server_instance, cmdline.whitelist_arg.getValue());
            if (error)
              return error;
          }

          // meas-root-dir
          if (cmdline.meas_root_dir_arg.isSet())
          {
            auto error = setMeasRootDir(rec_server_instance, cmdline.meas_root_dir_arg.getValue());
            if (error)
              return error;
          }

          // meas-name
          if (cmdline.meas_name_arg.isSet())
          {
            auto error = setMeasName(rec_server_instance, cmdline.meas_name_arg.getValue());
            if (error)
              return error;
          }

          // max-file-size
          if (cmdline.max_file_size_mib_arg.isSet())
          {
            auto error = setMaxFileSize(rec_server_instance, cmdline.max_file_size_mib_arg.getValue());
            if (error)
              return error;
          }

          // description
          if (cmdline.description_arg.isSet())
          {
            auto error = setDescription(rec_server_instance, cmdline.description_arg.getValue());
            if (error)
              return error;
          }
        }

        return eCAL::rec::Error::ErrorCode::OK;
      }

      eCAL::rec::Error SetConfig::Execute(const std::string& /*hostname*/, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& /*remote_rec_server_service*/, const std::vector<std::string>& /*argv*/) const
      {
        return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::COMMAND_NOT_AVAILABLE_IN_REMOTE_MODE);
      }

      //////////////////////////////////////////////
      /// SetConfig functions              
      //////////////////////////////////////////////
      
      eCAL::rec::Error SetConfig::SetClient           (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::vector<std::string>& param)
      {
        auto enabled_rec_clients = rec_server_instance->GetEnabledRecClients();

        for (const std::string& client : param)
        { 
          std::string           client_hostname;
          std::set<std::string> client_host_filter;

          // 1. Parse the input (Hostname:List,of,hosts)
          size_t colon_pos = client.find_first_of(":");
          if (colon_pos == std::string::npos)
          {
            client_hostname = EcalUtils::String::Trim(client);
          }
          else
          {
            client_hostname    = EcalUtils::String::Trim(client.substr(0, colon_pos));
            std::vector<std::string> tmp_client_host_filter;
            EcalUtils::String::Split(client.substr(colon_pos + 1), ",", tmp_client_host_filter);
            for (auto& host : tmp_client_host_filter)
            {
              std::string trimmed_host = EcalUtils::String::Trim(host);
              if (trimmed_host.empty())
                continue;

              client_host_filter.emplace(trimmed_host);
            }
          }

          if (client_hostname.empty())
            return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Hostname of client is mandatory, but got: \"" + client + "\"");

          // 2. Check if that hostname already exists. If not, add it!
          auto existing_rec_client_it = enabled_rec_clients.find(client_hostname);
          if (existing_rec_client_it == enabled_rec_clients.end())
          {
            enabled_rec_clients.emplace(client_hostname, eCAL::rec_server::ClientConfig());
            existing_rec_client_it = enabled_rec_clients.find(client_hostname);
          }

          // 3. Modify the client settings
          existing_rec_client_it->second.host_filter_ = client_host_filter;
        }

        bool success = rec_server_instance->SetEnabledRecClients(enabled_rec_clients);
        if (success)
          return eCAL::rec::Error::ErrorCode::OK;
        else
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR, "Failed to set rec clients");      }

      eCAL::rec::Error SetConfig::SetAddons           (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::vector<std::string>& param)
      {
        auto enabled_rec_clients = rec_server_instance->GetEnabledRecClients();

        for (const std::string& client : param)
        { 
          std::string           client_hostname;
          std::set<std::string> enabled_addons;

          // 1. Parse the input (Hostname:List,of,addon_ids)
          size_t colon_pos = client.find_first_of(":");
          if (colon_pos == std::string::npos)
          {
            client_hostname = EcalUtils::String::Trim(client);
          }
          else
          {
            client_hostname    = EcalUtils::String::Trim(client.substr(0, colon_pos));
            std::vector<std::string> tmp_enabled_addon_ids;
            EcalUtils::String::Split(client.substr(colon_pos + 1), ",", tmp_enabled_addon_ids);
            for (auto& host : tmp_enabled_addon_ids)
            {
              std::string trimmed_addon_id = EcalUtils::String::Trim(host);
              if (trimmed_addon_id.empty())
                continue;

              enabled_addons.emplace(trimmed_addon_id);
            }
          }

          if (client_hostname.empty())
            return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Hostname of client is mandatory, but got: \"" + client + "\"");

          // 2. Check if that hostname already exists
          auto existing_rec_client_it = enabled_rec_clients.find(client_hostname);
          if (existing_rec_client_it == enabled_rec_clients.end())
            return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::CLIENT_UNKNOWN, "Client \"" + client_hostname + "\" does not exist");


          // 3. Modify the client settings
          existing_rec_client_it->second.enabled_addons_ = enabled_addons;
        }

        bool success = rec_server_instance->SetEnabledRecClients(enabled_rec_clients);
        if (success)
          return eCAL::rec::Error::ErrorCode::OK;
        else
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR, "Failed to set rec clients");
      }
      
      eCAL::rec::Error SetConfig::RemoveClient        (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::vector<std::string>& param)
      {
        auto enabled_rec_clients = rec_server_instance->GetEnabledRecClients();

        for (const std::string& client_hostname : param)
        {
          // Check if that hostname exists
          auto existing_rec_client_it = enabled_rec_clients.find(client_hostname);
          if (existing_rec_client_it == enabled_rec_clients.end())
            return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::CLIENT_UNKNOWN, "Client \"" + client_hostname + "\" does not exist and cannot be removed");

          enabled_rec_clients.erase(existing_rec_client_it);
        }

        bool success = rec_server_instance->SetEnabledRecClients(enabled_rec_clients);
        if (success)
          return eCAL::rec::Error::ErrorCode::OK;
        else
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR, "Failed to set rec clients");      }

      eCAL::rec::Error SetConfig::setFtpServer        (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& param)
      {
        if (param.empty())
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Parameter for FTP Server must not be empty");

        auto new_upload_config = rec_server_instance->GetUploadConfig();

        if (param == "internal")
        {
          new_upload_config.type_ = eCAL::rec_server::UploadConfig::Type::INTERNAL_FTP;
        }
        else
        {
          const std::regex ftp_path_regex(R"_(ftp://((.*):(.*)@)?([0-9A-Za-z\.\-]+)(:([0-9]+))?(/.*)?)_");
          std::smatch      ftp_path_match;
          if (std::regex_match(param, ftp_path_match, ftp_path_regex))
          {

            // Regex match structure:
            // 
            // [0]: complete match
            // [1]: USER:PASS@    (optional)
            //   [2]: USER        (optional)
            //   [3]: PASS        (optional)
            // [4]: HOSTNAME
            // [5]: :PORT         (optional)
            //   [6]: PORT        (optional)
            // [7]: ROOT_PATH     (optional)

            if (ftp_path_match.size() != 8)
              return eCAL::rec::Error(eCAL::rec::Error::PARAMETER_ERROR, "Failed regex-parsing " + param);

            new_upload_config.username_ = ftp_path_match[2].str();
            new_upload_config.password_ = ftp_path_match[3].str();
            new_upload_config.host_     = ftp_path_match[4].str();

            std::string port_string     = ftp_path_match[6].str();
            if (port_string.empty())
            {
              new_upload_config.port_ = 21;
            }
            else
            {
              try
              {
                unsigned long port_long = std::stoul(port_string);
                if ((port_long > std::numeric_limits<uint16_t>::max()) || (port_long <= 0))
                  return eCAL::rec::Error(eCAL::rec::Error::PARAMETER_ERROR, "Invalid port: " + port_string);

                new_upload_config.port_ = static_cast<uint16_t>(port_long);
              }
              catch (const std::exception& /*e*/)
              {
                return eCAL::rec::Error(eCAL::rec::Error::PARAMETER_ERROR, "Invalid port: " + port_string);
              }
            }

            new_upload_config.root_path_ = ftp_path_match[7].str();
            new_upload_config.type_      = eCAL::rec_server::UploadConfig::Type::FTP;
          }
          else
          {
            return eCAL::rec::Error(eCAL::rec::Error::PARAMETER_ERROR, "Malformed FTP server setting: " + param);
          }
        }

        rec_server_instance->SetUploadConfig(new_upload_config);
        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::setDeleteAfterUpload(const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, bool param)
      {
        auto new_upload_config = rec_server_instance->GetUploadConfig();
        new_upload_config.delete_after_upload_ = param;
        rec_server_instance->SetUploadConfig(new_upload_config);
        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::setBuiltInClientEnabled(const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, bool param)
      {
        if (rec_server_instance->IsRecording())
          return eCAL::rec::Error::ErrorCode::CURRENTLY_RECORDING;

        bool success = rec_server_instance->SetUsingBuiltInRecorderEnabled(param);

        if (success)
          return eCAL::rec::Error::OK;
        else
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR, std::string("Failed to set built-in client enabled to ") + (param ? "true" : "false"));
      }

      eCAL::rec::Error SetConfig::setPreBuffer        (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& param)
      {
        char decimal_point = std::localeconv()->decimal_point[0]; // decimal point for std::stod()

        double pre_buffer_secs;
        std::string pre_buffer_secs_string = param;
        std::replace(pre_buffer_secs_string.begin(), pre_buffer_secs_string.end(), '.', decimal_point);

        if (pre_buffer_secs_string.empty())
        {
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Value for pre-buffer length must not  be empty");
        }

        try
        {
          pre_buffer_secs = std::stod(pre_buffer_secs_string);
        }
        catch (const std::exception& e)
        {
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Failed parsing value \"" + pre_buffer_secs_string + "\": " + e.what());
        }

        if (pre_buffer_secs >= 0)
        {
          rec_server_instance->SetMaxPreBufferLength(std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(pre_buffer_secs)));
          rec_server_instance->SetPreBufferingEnabled(true);
        }
        else
        {
          rec_server_instance->SetPreBufferingEnabled(false);
        }

        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::setBlacklist        (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& param)
      {
        std::set<std::string> blacklist;
        EcalUtils::String::Split(param, ",", blacklist);

        for (std::string topic : blacklist)
        {
          topic = EcalUtils::String::Trim(topic);
        }

        bool success = false;

        if (blacklist.empty())
        {
          success = rec_server_instance->SetRecordMode(eCAL::rec::RecordMode::All);
        }
        else
        {
          success = rec_server_instance->SetRecordMode(eCAL::rec::RecordMode::Blacklist, blacklist);
        }

        if (!success)
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR, "Failed to set blacklist");
        else
          return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::setWhitelist        (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& param)
      {
        std::set<std::string> whitelist;
        EcalUtils::String::Split(param, ",", whitelist);

        for (std::string topic : whitelist)
        {
          topic = EcalUtils::String::Trim(topic);
        }

        bool success = rec_server_instance->SetRecordMode(eCAL::rec::RecordMode::Whitelist, whitelist);

        if (!success)
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR, "Failed to set whitelist");
        else
          return eCAL::rec::Error::ErrorCode::OK;
      }

      eCAL::rec::Error SetConfig::setMeasRootDir      (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& param)
      {
        rec_server_instance->SetMeasRootDir(param);
        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::setMeasName         (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& param)
      {
        rec_server_instance->SetMeasName(param);
        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::setMaxFileSize      (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& param)
      {
        if (param.empty())
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Value for max HDF5 file size must not  be empty");

        unsigned int max_file_size_mib;

        try
        {
          max_file_size_mib = std::stoul(param);
        }
        catch (const std::exception& e)
        {
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Failed parsing value \"" + param + "\": " + e.what());
        }

        rec_server_instance->SetMaxFileSizeMib(max_file_size_mib);
        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::setDescription      (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& param)
      {
        rec_server_instance->SetDescription(param);
        return eCAL::rec::Error::OK;
      }
    }
  }
}
