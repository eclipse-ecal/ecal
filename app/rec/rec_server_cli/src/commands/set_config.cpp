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

#include <rec_server_core/proto_helpers.h>

#include <clocale> // localeconv
#include <functional>

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
          &cmdline.enable_one_file_per_topic_arg,
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
            auto error = SetFtpServer(rec_server_instance, cmdline.ftp_server_arg.getValue());
            if (error)
              return error;
          }

          // Delete after upload
          if (cmdline.delete_after_upload_arg.isSet())
          {
            auto error = SetDeleteAfterUpload(rec_server_instance, cmdline.delete_after_upload_arg.getValue());
            if (error)
              return error;
          }

          // Set built-in client enabled
          if (cmdline.enable_built_in_client_arg.isSet())
          {
            auto error = SetBuiltInClientEnabled(rec_server_instance, cmdline.enable_built_in_client_arg.getValue());
            if (error)
              return error;
          }

          // Pre-buffer
          if (cmdline.pre_buffer_secs_arg.isSet())
          {
            auto error = SetPreBuffer(rec_server_instance, cmdline.pre_buffer_secs_arg.getValue());
            if (error)
              return error;
          }

          // blacklist
          if (cmdline.blacklist_arg.isSet())
          {
            auto error = SetBlacklist(rec_server_instance, cmdline.blacklist_arg.getValue());
            if (error)
              return error;
          }

          // whitelist
          if (cmdline.whitelist_arg.isSet())
          {
            auto error = SetWhitelist(rec_server_instance, cmdline.whitelist_arg.getValue());
            if (error)
              return error;
          }

          // meas-root-dir
          if (cmdline.meas_root_dir_arg.isSet())
          {
            auto error = SetMeasRootDir(rec_server_instance, cmdline.meas_root_dir_arg.getValue());
            if (error)
              return error;
          }

          // meas-name
          if (cmdline.meas_name_arg.isSet())
          {
            auto error = SetMeasName(rec_server_instance, cmdline.meas_name_arg.getValue());
            if (error)
              return error;
          }

          // max-file-size
          if (cmdline.max_file_size_mib_arg.isSet())
          {
            auto error = SetMaxFileSize(rec_server_instance, cmdline.max_file_size_mib_arg.getValue());
            if (error)
              return error;
          }

          // description
          if (cmdline.description_arg.isSet())
          {
            auto error = SetDescription(rec_server_instance, cmdline.description_arg.getValue());
            if (error)
              return error;
          }

          // Enable one file per topic
          if (cmdline.enable_one_file_per_topic_arg.isSet())
          {
            auto error = SetOneFilePerTopicEnabled(rec_server_instance, cmdline.enable_one_file_per_topic_arg.getValue());
            if (error)
              return error;
          }
        }

        return eCAL::rec::Error::ErrorCode::OK;
      }

      eCAL::rec::Error SetConfig::Execute(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::vector<std::string>& argv) const
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

          // Get current config
          const eCAL::pb::rec_server::GenericRequest      request_pb;
          eCAL::pb::rec_server::RecServerConfig           rec_server_config_pb;

          {
            eCAL::rec::Error service_call_error = CallRemoteEcalrecService(remote_rec_server_service, hostname, "GetConfig", request_pb, rec_server_config_pb);

            // Service call failed
            if (service_call_error)
            {
              return service_call_error;
            }
          }

          // Parse the protobuf config to the intern config struct.
          // We are going to work on that for ease of use.
          // Later however we are NOT going to convert it back and set the entire
          // config, again. In cases where we are cummunicating with a newer
          // eCAL rec this would cause settings to get lost and being set to
          // default values.
          // Instead, we are going to work on the original config_pb, that still
          // contains all unknown values, and change only what we need to.


          // Set clients
          if (cmdline.set_client_arg.isSet())
          {
            auto error = SetClient(rec_server_config_pb, cmdline.set_client_arg.getValue());
            if (error)
              return error;
          }

          // Set Addons
          if (cmdline.set_addons_arg.isSet())
          {
            auto error = SetAddons(rec_server_config_pb, cmdline.set_addons_arg.getValue());
            if (error)
              return error;
          }

          // Remove Client 
          if (cmdline.remove_client_arg.isSet())
          {
            auto error = RemoveClient(rec_server_config_pb, cmdline.remove_client_arg.getValue());
            if (error)
              return error;
          }

          // Set FTP Server
          if (cmdline.ftp_server_arg.isSet())
          {
            auto error = SetFtpServer(rec_server_config_pb, cmdline.ftp_server_arg.getValue());
            if (error)
              return error;
          }

          // Set Delete-after-upload
          if (cmdline.delete_after_upload_arg.isSet())
          {
            auto error = SetDeleteAfterUpload(rec_server_config_pb, cmdline.delete_after_upload_arg.getValue());
            if (error)
              return error;
          }

          // Build-in client
          if (cmdline.enable_built_in_client_arg.isSet())
          {
            auto error = SetBuiltInClientEnabled(rec_server_config_pb, cmdline.enable_built_in_client_arg.getValue());
            if (error)
              return error;
          }

          // SetPreBuffer
          if (cmdline.pre_buffer_secs_arg.isSet())
          {
            auto error = SetPreBuffer(rec_server_config_pb, cmdline.pre_buffer_secs_arg.getValue());
            if (error)
              return error;
          }

          // SetBlacklist
          if (cmdline.blacklist_arg.isSet())
          {
            auto error = SetBlacklist(rec_server_config_pb, cmdline.blacklist_arg.getValue());
            if (error)
              return error;
          }

          // SetWhitelist
          if (cmdline.whitelist_arg.isSet())
          {
            auto error = SetWhitelist(rec_server_config_pb, cmdline.whitelist_arg.getValue());
            if (error)
              return error;
          }

          // SetMeasRootDir
          if (cmdline.meas_root_dir_arg.isSet())
          {
            auto error = SetMeasRootDir(rec_server_config_pb, cmdline.meas_root_dir_arg.getValue());
            if (error)
              return error;
          }

          // SetMeasName
          if (cmdline.meas_name_arg.isSet())
          {
            auto error = SetMeasName(rec_server_config_pb, cmdline.meas_name_arg.getValue());
            if (error)
              return error;
          }

          // SetMaxFileSize
          if (cmdline.max_file_size_mib_arg.isSet())
          {
            auto error = SetMaxFileSize(rec_server_config_pb, cmdline.max_file_size_mib_arg.getValue());
            if (error)
              return error;
          }

          // SetDescription
          if (cmdline.description_arg.isSet())
          {
            auto error = SetDescription(rec_server_config_pb, cmdline.description_arg.getValue());
            if (error)
              return error;
          }

          // SetOneFilePerTopicEnabled
          if (cmdline.enable_one_file_per_topic_arg.isSet())
          {
            auto error = SetOneFilePerTopicEnabled(rec_server_config_pb, cmdline.enable_one_file_per_topic_arg.getValue());
            if (error)
              return error;
          }

          // Call the SetConfig RPC Service
          {
            eCAL::pb::rec_server::ServiceResult service_call_response_pb;
            eCAL::rec::Error service_call_error = CallRemoteEcalrecService(remote_rec_server_service, hostname, "SetConfig", rec_server_config_pb, service_call_response_pb);

            // Service call failed
            if (service_call_error)
              return service_call_error;

            // Service call reported error
            {
              eCAL::rec::Error error = eCAL::rec_server::proto_helpers::FromProtobuf(service_call_response_pb);
              if (error)
                return error;
            }
          }
        }

        return eCAL::rec::Error::OK;
      }

      //////////////////////////////////////////////
      /// Parse Param Functions
      //////////////////////////////////////////////

      eCAL::rec::Error SetConfig::ParseParams_SetClient         (const std::vector<std::string>& param, std::map<std::string, eCAL::rec_server::ClientConfig>& client_configs_out)
      {
        client_configs_out.clear();

        for(const std::string& client : param)
        {
          std::string           client_hostname;
          std::set<std::string> client_host_filter;

          // 1. Parse the input (Hostname:List,of,hosts)
          const size_t colon_pos = client.find_first_of(':');
          if (colon_pos == std::string::npos)
          {
            client_hostname = EcalUtils::String::Trim(client);
          }
          else
          {
            client_hostname    = EcalUtils::String::Trim(client.substr(0, colon_pos));
            std::vector<std::string> tmp_client_host_filter;
            EcalUtils::String::Split(client.substr(colon_pos + 1), ",", tmp_client_host_filter);
            for(auto& host : tmp_client_host_filter)
            {
              const std::string trimmed_host = EcalUtils::String::Trim(host);
              if (trimmed_host.empty())
                continue;

              client_host_filter.emplace(trimmed_host);
            }
          }

          // Return error, if the hostname is unclear
          if (client_hostname.empty())
            return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Hostname of client is mandatory, but got: \"" + client + "\"");

          // Add the config to the client config map
          eCAL::rec_server::ClientConfig client_config;
          client_config.host_filter_ = client_host_filter;

          client_configs_out.emplace(client_hostname, client_config);
        }

        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::ParseParams_SetAddons         (const std::vector<std::string>& param, std::map<std::string, eCAL::rec_server::ClientConfig>& client_configs_out)
      {
        client_configs_out.clear();

        for (const std::string& client : param)
        { 
          std::string           client_hostname;
          std::set<std::string> enabled_addons;

          // 1. Parse the input (Hostname:List,of,addon_ids)
          const size_t colon_pos = client.find_first_of(':');
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
              const std::string trimmed_addon_id = EcalUtils::String::Trim(host);
              if (trimmed_addon_id.empty())
                continue;

              enabled_addons.emplace(trimmed_addon_id);
            }
          }

          // Return error, if the hostname is unclear
          if (client_hostname.empty())
            return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Hostname of client is mandatory, but got: \"" + client + "\"");

          // Add the config to the client config map
          eCAL::rec_server::ClientConfig client_config;
          client_config.enabled_addons_ = enabled_addons;

          client_configs_out.emplace(client_hostname, client_config);
        }

        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::ParseParams_SetFtpServer      (const std::string& param,              eCAL::rec_server::UploadConfig&                        upload_config_out)
      {
        if (param.empty())
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Parameter for FTP Server must not be empty");

        if (param == "internal")
        {
          upload_config_out.type_ = eCAL::rec_server::UploadConfig::Type::INTERNAL_FTP;
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

            upload_config_out.username_ = ftp_path_match[2].str();
            upload_config_out.password_ = ftp_path_match[3].str();
            upload_config_out.host_     = ftp_path_match[4].str();

            const std::string port_string     = ftp_path_match[6].str();
            if (port_string.empty())
            {
              upload_config_out.port_ = 21;
            }
            else
            {
              try
              {
                const unsigned long port_long = std::stoul(port_string);
                if ((port_long > std::numeric_limits<uint16_t>::max()) || (port_long <= 0))
                  return eCAL::rec::Error(eCAL::rec::Error::PARAMETER_ERROR, "Invalid port: " + port_string);

                upload_config_out.port_ = static_cast<uint16_t>(port_long);
              }
              catch (const std::exception& /*e*/)
              {
                return eCAL::rec::Error(eCAL::rec::Error::PARAMETER_ERROR, "Invalid port: " + port_string);
              }
            }

            upload_config_out.root_path_ = ftp_path_match[7].str();
            upload_config_out.type_      = eCAL::rec_server::UploadConfig::Type::FTP;
          }
          else
          {
            return eCAL::rec::Error(eCAL::rec::Error::PARAMETER_ERROR, "Malformed FTP server setting: " + param);
          }
        }

        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::ParseParams_SetPreBuffer      (const std::string& param,              std::chrono::duration<double>&                         pre_buffer_length_out)
      {
        const char decimal_point = std::localeconv()->decimal_point[0]; // decimal point for std::stod()

        double pre_buffer_secs{0};
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
          pre_buffer_length_out = std::chrono::duration<double>(pre_buffer_secs);
        }
        else
        {
          pre_buffer_length_out = std::chrono::duration<double>(0.0);
        }

        return eCAL::rec::Error::OK;

      }

      eCAL::rec::Error SetConfig::ParseParams_TopicList          (const std::string& param,              std::set<std::string>&                                 topic_list_out)
      {
        topic_list_out.clear();
        std::vector<std::string> topic_list_tmp;

        EcalUtils::String::Split(param, ",", topic_list_tmp);

        for (const std::string& topic : topic_list_tmp)
        {
          const std::string trimmed_topic = EcalUtils::String::Trim(topic);
          if (!trimmed_topic.empty())
            topic_list_out.insert(trimmed_topic);
        }

        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::ParseParams_SetMaxFileSize     (const std::string& param,              unsigned int&                                          max_file_size_mib_out)
      {
        if (param.empty())
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Value for max HDF5 file size must not  be empty");

        try
        {
          max_file_size_mib_out = std::stoul(param);
        }
        catch (const std::exception& e)
        {
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Failed parsing value \"" + param + "\": " + e.what());
        }
        
        return eCAL::rec::Error::OK;
      }

      //////////////////////////////////////////////
      /// SetConfig functions (remote directly)
      //////////////////////////////////////////////
      eCAL::rec::Error SetConfig::SetClient                 (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::vector<std::string>& param)
      {
        // Select the correct overload of the function and bind it to an std::function object
        auto f = static_cast<eCAL::rec::Error(*)(eCAL::pb::rec_server::RecServerConfig&, const std::vector<std::string>&)>(SetClient);

        // Call the wrapper function that will set the according setting
        return SetConfigDirectly<const std::vector<std::string>&>(hostname, remote_rec_server_service, param, f);
      }

      eCAL::rec::Error SetConfig::SetAddons                 (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::vector<std::string>& param)
      {
        // Select the correct overload of the function and bind it to an std::function object
        auto f = static_cast<eCAL::rec::Error(*)(eCAL::pb::rec_server::RecServerConfig&, const std::vector<std::string>&)>(SetAddons);

        // Call the wrapper function that will set the according setting
        return SetConfigDirectly<const std::vector<std::string>&>(hostname, remote_rec_server_service, param, f);
      }

      eCAL::rec::Error SetConfig::RemoveClient              (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::vector<std::string>& param)
      {
        // Select the correct overload of the function and bind it to an std::function object
        auto f = static_cast<eCAL::rec::Error(*)(eCAL::pb::rec_server::RecServerConfig&, const std::vector<std::string>&)>(RemoveClient);

        // Call the wrapper function that will set the according setting
        return SetConfigDirectly<const std::vector<std::string>&>(hostname, remote_rec_server_service, param, f);
      }

      eCAL::rec::Error SetConfig::SetFtpServer              (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::string& param)
      {
        // Select the correct overload of the function and bind it to an std::function object
        auto f = static_cast<eCAL::rec::Error(*)(eCAL::pb::rec_server::RecServerConfig&, const std::string&)>(SetFtpServer);

        // Call the wrapper function that will set the according setting
        return SetConfigDirectly<const std::string&>(hostname, remote_rec_server_service, param, f);
      }

      eCAL::rec::Error SetConfig::SetDeleteAfterUpload      (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, bool param)
      {
        // Select the correct overload of the function and bind it to an std::function object
        auto f = static_cast<eCAL::rec::Error(*)(eCAL::pb::rec_server::RecServerConfig&, bool)>(SetDeleteAfterUpload);

        // Call the wrapper function that will set the according setting
        return SetConfigDirectly<bool>(hostname, remote_rec_server_service, param, f);
      }

      eCAL::rec::Error SetConfig::SetBuiltInClientEnabled   (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, bool param)
      {
        // Select the correct overload of the function and bind it to an std::function object
        auto f = static_cast<eCAL::rec::Error(*)(eCAL::pb::rec_server::RecServerConfig&, bool)>(SetBuiltInClientEnabled);

        // Call the wrapper function that will set the according setting
        return SetConfigDirectly<bool>(hostname, remote_rec_server_service, param, f);
      }

      eCAL::rec::Error SetConfig::SetPreBuffer              (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::string& param)
      {
        // Select the correct overload of the function and bind it to an std::function object
        auto f = static_cast<eCAL::rec::Error(*)(eCAL::pb::rec_server::RecServerConfig&, const std::string&)>(SetPreBuffer);

        // Call the wrapper function that will set the according setting
        return SetConfigDirectly<const std::string&>(hostname, remote_rec_server_service, param, f);
      }

      eCAL::rec::Error SetConfig::SetBlacklist              (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::string& param)
      {
        // Select the correct overload of the function and bind it to an std::function object
        auto f = static_cast<eCAL::rec::Error(*)(eCAL::pb::rec_server::RecServerConfig&, const std::string&)>(SetBlacklist);

        // Call the wrapper function that will set the according setting
        return SetConfigDirectly<const std::string&>(hostname, remote_rec_server_service, param, f);
      }

      eCAL::rec::Error SetConfig::SetWhitelist              (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::string& param)
      {
        // Select the correct overload of the function and bind it to an std::function object
        auto f = static_cast<eCAL::rec::Error(*)(eCAL::pb::rec_server::RecServerConfig&, const std::string&)>(SetWhitelist);

        // Call the wrapper function that will set the according setting
        return SetConfigDirectly<const std::string&>(hostname, remote_rec_server_service, param, f);
      }

      eCAL::rec::Error SetConfig::SetMeasRootDir            (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::string& param)
      {
        // Select the correct overload of the function and bind it to an std::function object
        auto f = static_cast<eCAL::rec::Error(*)(eCAL::pb::rec_server::RecServerConfig&, const std::string&)>(SetMeasRootDir);

        // Call the wrapper function that will set the according setting
        return SetConfigDirectly<const std::string&>(hostname, remote_rec_server_service, param, f);
      }

      eCAL::rec::Error SetConfig::SetMeasName               (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::string& param)
      {
        // Select the correct overload of the function and bind it to an std::function object
        auto f = static_cast<eCAL::rec::Error(*)(eCAL::pb::rec_server::RecServerConfig&, const std::string&)>(SetMeasName);

        // Call the wrapper function that will set the according setting
        return SetConfigDirectly<const std::string&>(hostname, remote_rec_server_service, param, f);
      }

      eCAL::rec::Error SetConfig::SetMaxFileSize            (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::string& param)
      {
        // Select the correct overload of the function and bind it to an std::function object
        auto f = static_cast<eCAL::rec::Error(*)(eCAL::pb::rec_server::RecServerConfig&, const std::string&)>(SetMaxFileSize);

        // Call the wrapper function that will set the according setting
        return SetConfigDirectly<const std::string&>(hostname, remote_rec_server_service, param, f);
      }

      eCAL::rec::Error SetConfig::SetDescription            (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::string& param)
      {
        // Select the correct overload of the function and bind it to an std::function object
        auto f = static_cast<eCAL::rec::Error(*)(eCAL::pb::rec_server::RecServerConfig&, const std::string&)>(SetDescription);

        // Call the wrapper function that will set the according setting
        return SetConfigDirectly<const std::string&>(hostname, remote_rec_server_service, param, f);
      }

      eCAL::rec::Error SetConfig::SetOneFilePerTopicEnabled (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, bool param)
      {
        // Select the correct overload of the function and bind it to an std::function object
        auto f = static_cast<eCAL::rec::Error(*)(eCAL::pb::rec_server::RecServerConfig&, bool)>(SetOneFilePerTopicEnabled);

        // Call the wrapper function that will set the according setting
        return SetConfigDirectly<bool>(hostname, remote_rec_server_service, param, f);
      }

      //////////////////////////////////////////////
      /// SetConfig functions (Protobuf config)
      //////////////////////////////////////////////

      eCAL::rec::Error SetConfig::SetClient                (eCAL::pb::rec_server::RecServerConfig& config_pb, const std::vector<std::string>& param)
      {
        std::map<std::string, eCAL::rec_server::ClientConfig> client_config_map;

        // Parse input parameters
        {
          auto parse_error = ParseParams_SetClient(param, client_config_map);
          if (parse_error)
            return parse_error;
        }

        // Apply to config
        {
          for (const auto& client : client_config_map)
          {
            auto existing_client_it = config_pb.mutable_enabled_clients_config()->find(client.first);
            if (existing_client_it != config_pb.mutable_enabled_clients_config()->end())
            {
              // Modify existing entry by only changing the host filter
              existing_client_it->second.clear_host_filter();
              for (const std::string& host : client.second.host_filter_)
              {
                existing_client_it->second.add_host_filter(host);
              }
            }
            else
            {
              // Create an entirely new entry
              (*config_pb.mutable_enabled_clients_config())[client.first] = eCAL::rec_server::proto_helpers::ToProtobuf(client.second);
            }
          }
        }
        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::SetAddons                (eCAL::pb::rec_server::RecServerConfig& config_pb, const std::vector<std::string>& param)
      {
        std::map<std::string, eCAL::rec_server::ClientConfig> client_config_map;

        // Parse input parameters
        {
          auto parse_error = ParseParams_SetAddons(param, client_config_map);
          if (parse_error)
            return parse_error;
        }

        // Apply to config
        {
          for (const auto& client : client_config_map)
          {
            auto existing_client_it = config_pb.mutable_enabled_clients_config()->find(client.first);
            if (existing_client_it != config_pb.mutable_enabled_clients_config()->end())
            {
              // Modify existing entry by only changing the host filter
              existing_client_it->second.clear_enabled_addons();
              for (const std::string& addon_id : client.second.enabled_addons_)
              {
                existing_client_it->second.add_enabled_addons(addon_id);
              }
            }
            else
            {
              return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::CLIENT_UNKNOWN, "Client \"" + client.first + "\" does not exist");
            }
          }
        }
        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::RemoveClient             (eCAL::pb::rec_server::RecServerConfig& config_pb, const std::vector<std::string>& param)
      {
        for (const std::string& client_hostname : param)
        {
          // Check if that hostname exists
          auto existing_rec_client_it = config_pb.mutable_enabled_clients_config()->find(client_hostname);
          if (existing_rec_client_it == config_pb.mutable_enabled_clients_config()->end())
            return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::CLIENT_UNKNOWN, "Client \"" + client_hostname + "\" does not exist and cannot be removed");

          config_pb.mutable_enabled_clients_config()->erase(existing_rec_client_it);
        }

        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::SetFtpServer             (eCAL::pb::rec_server::RecServerConfig& config_pb, const std::string& param)
      {
        eCAL::rec_server::UploadConfig upload_config;

        // Parse input parameters
        {
          auto parse_error = ParseParams_SetFtpServer(param, upload_config);
          if (parse_error)
            return parse_error;
        }

        // Apply to config
        {
          if (upload_config.type_ == eCAL::rec_server::UploadConfig::Type::FTP)
          {
            // External FTP
            config_pb.mutable_upload_config()->set_type     (eCAL::pb::rec_server::UploadConfig::Type::UploadConfig_Type_Ftp);
            config_pb.mutable_upload_config()->set_host     (upload_config.host_);
            config_pb.mutable_upload_config()->set_port     (upload_config.port_);
            config_pb.mutable_upload_config()->set_username (upload_config.username_);
            config_pb.mutable_upload_config()->set_password (upload_config.password_);
            config_pb.mutable_upload_config()->set_root_path(upload_config.root_path_);
          }
          else
          {
            // Default: Internal FTP
            config_pb.mutable_upload_config()->set_type     (eCAL::pb::rec_server::UploadConfig::Type::UploadConfig_Type_InternalFtp);
          }
        }

        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::SetDeleteAfterUpload     (eCAL::pb::rec_server::RecServerConfig& config_pb, bool param)
      {
        config_pb.mutable_upload_config()->set_delete_after_upload(param);
        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::SetBuiltInClientEnabled  (eCAL::pb::rec_server::RecServerConfig& config_pb, bool param)
      {
        config_pb.set_built_in_recorder_enabled(param);
        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::SetPreBuffer             (eCAL::pb::rec_server::RecServerConfig& config_pb, const std::string& param)
      {
        std::chrono::duration<double> pre_buffer_length{0};

        // Parse Parameter
        {
          auto error = ParseParams_SetPreBuffer(param, pre_buffer_length);
          if (error)
            return error;
        }

        // Apply to config
        if (pre_buffer_length > std::chrono::duration<double>(0.0))
        {
          config_pb.set_pre_buffer_enabled(true);
          config_pb.set_pre_buffer_length_nsecs(std::chrono::duration_cast<std::chrono::nanoseconds>(pre_buffer_length).count());
        }
        else
        {
          config_pb.set_pre_buffer_enabled(false);
        }
        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::SetBlacklist             (eCAL::pb::rec_server::RecServerConfig& config_pb, const std::string& param)
      {
        std::set<std::string> topic_list;

        // Parse param
        {
          auto error = ParseParams_TopicList(param, topic_list);
          if (error)
            return error;
        }

        // Apply to config
        {
          if (topic_list.empty())
          {
            config_pb.set_record_mode(eCAL::pb::rec_server::RecordMode::All);
          }
          else
          {
            config_pb.set_record_mode(eCAL::pb::rec_server::RecordMode::Blacklist);

            config_pb.clear_listed_topics();
            for (const std::string& topic : topic_list)
            {
              config_pb.add_listed_topics(topic);
            }
          }
        }

        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::SetWhitelist             (eCAL::pb::rec_server::RecServerConfig& config_pb, const std::string& param)
      {
        std::set<std::string> topic_list;

        // Parse param
        {
          auto error = ParseParams_TopicList(param, topic_list);
          if (error)
            return error;
        }

        // Apply to config
        {
          config_pb.set_record_mode(eCAL::pb::rec_server::RecordMode::Whitelist);

          config_pb.clear_listed_topics();
          for (const std::string& topic : topic_list)
          {
            config_pb.add_listed_topics(topic);
          }
        }

        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::SetMeasRootDir           (eCAL::pb::rec_server::RecServerConfig& config_pb, const std::string& param)
      {
        config_pb.set_root_dir(param);
        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::SetMeasName              (eCAL::pb::rec_server::RecServerConfig& config_pb, const std::string& param)
      {
        config_pb.set_meas_name(param);
        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::SetMaxFileSize           (eCAL::pb::rec_server::RecServerConfig& config_pb, const std::string& param)
      {
        unsigned int max_file_size_mib{0};

        // Parse Parameter
        {
          auto error = ParseParams_SetMaxFileSize(param, max_file_size_mib);
          if (error)
            return error;
        }

        // Apply to config
        config_pb.set_max_file_size_mib(max_file_size_mib);

        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::SetDescription           (eCAL::pb::rec_server::RecServerConfig& config_pb, const std::string& param)
      {
        config_pb.set_description(param);
        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::SetOneFilePerTopicEnabled(eCAL::pb::rec_server::RecServerConfig& config_pb, bool param)
      {
        config_pb.set_one_file_per_topic(param);
        return eCAL::rec::Error::OK;
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
          const size_t colon_pos = client.find_first_of(':');
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
              const std::string trimmed_host = EcalUtils::String::Trim(host);
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

        const bool success = rec_server_instance->SetEnabledRecClients(enabled_rec_clients);
        if (success)
          return eCAL::rec::Error::ErrorCode::OK;
        else
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR, "Failed to set rec clients");      
      }

      eCAL::rec::Error SetConfig::SetAddons           (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::vector<std::string>& param)
      {
        auto enabled_rec_clients = rec_server_instance->GetEnabledRecClients();

        for (const std::string& client : param)
        { 
          std::string           client_hostname;
          std::set<std::string> enabled_addons;

          // 1. Parse the input (Hostname:List,of,addon_ids)
          const size_t colon_pos = client.find_first_of(':');
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
              const std::string trimmed_addon_id = EcalUtils::String::Trim(host);
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

        const bool success = rec_server_instance->SetEnabledRecClients(enabled_rec_clients);
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

        const bool success = rec_server_instance->SetEnabledRecClients(enabled_rec_clients);
        if (success)
          return eCAL::rec::Error::ErrorCode::OK;
        else
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR, "Failed to set rec clients");
      }

      eCAL::rec::Error SetConfig::SetFtpServer        (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& param)
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

            const std::string port_string     = ftp_path_match[6].str();
            if (port_string.empty())
            {
              new_upload_config.port_ = 21;
            }
            else
            {
              try
              {
                const unsigned long port_long = std::stoul(port_string);
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

      eCAL::rec::Error SetConfig::SetDeleteAfterUpload(const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, bool param)
      {
        auto new_upload_config = rec_server_instance->GetUploadConfig();
        new_upload_config.delete_after_upload_ = param;
        rec_server_instance->SetUploadConfig(new_upload_config);
        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::SetBuiltInClientEnabled(const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, bool param)
      {
        if (rec_server_instance->IsRecording())
          return eCAL::rec::Error::ErrorCode::CURRENTLY_RECORDING;

        const bool success = rec_server_instance->SetUsingBuiltInRecorderEnabled(param);

        if (success)
          return eCAL::rec::Error::OK;
        else
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR, std::string("Failed to set built-in client enabled to ") + (param ? "true" : "false"));
      }

      eCAL::rec::Error SetConfig::SetPreBuffer        (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& param)
      {
        const char decimal_point = std::localeconv()->decimal_point[0]; // decimal point for std::stod()

        double pre_buffer_secs{0};
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

      eCAL::rec::Error SetConfig::SetBlacklist        (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& param)
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

      eCAL::rec::Error SetConfig::SetWhitelist        (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& param)
      {
        std::set<std::string> whitelist;
        EcalUtils::String::Split(param, ",", whitelist);

        for (std::string topic : whitelist)
        {
          topic = EcalUtils::String::Trim(topic);
        }

        const bool success = rec_server_instance->SetRecordMode(eCAL::rec::RecordMode::Whitelist, whitelist);

        if (!success)
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR, "Failed to set whitelist");
        else
          return eCAL::rec::Error::ErrorCode::OK;
      }

      eCAL::rec::Error SetConfig::SetMeasRootDir      (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& param)
      {
        rec_server_instance->SetMeasRootDir(param);
        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::SetMeasName         (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& param)
      {
        rec_server_instance->SetMeasName(param);
        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::SetMaxFileSize      (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& param)
      {
        if (param.empty())
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Value for max HDF5 file size must not  be empty");

        unsigned int max_file_size_mib{0};

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

      eCAL::rec::Error SetConfig::SetDescription      (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& param)
      {
        rec_server_instance->SetDescription(param);
        return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SetConfig::SetOneFilePerTopicEnabled (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, bool param)
      {
        rec_server_instance->SetOneFilePerTopicEnabled(param);
        return eCAL::rec::Error::OK;
      }
    }
  }
}
