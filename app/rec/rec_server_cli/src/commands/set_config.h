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

#pragma once

#include "command.h"


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4505) // disable tclap warning (unreferenced local function has been removed)
#endif
#include "tclap/CmdLine.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <custom_tclap/advanced_tclap_output.h>
#include <custom_tclap/fuzzy_value_switch_arg_bool.h>

#include <rec_server_core/proto_helpers.h>

namespace eCAL
{
  namespace rec_cli
  {
    namespace command
    {
      class SetConfig : public Command
      {
      //////////////////////////////////////////////
      /// Job History Entry evaluation methods
      //////////////////////////////////////////////
        struct CmdLine
        {
          CmdLine()
            : cmd                  ("setconfig", ' ', "", false)
            , advanced_tclap_output(std::vector<std::ostream*>{ &tclap_output_stream }, 75)

            , unlabled_arg            ("unlabled_arg",      "", false, "")
            , set_client_arg          ("", "set-client",    "Sets which hosts shall be recorded by the given client. Use the syntax \"Hostname:Host,To,Record\". For instance, to let PC1 record itself and PC2, use \"PC1:PC1,PC2\". If no tailing list is provided, the client will record topcis from all hosts by default.", false, "Hostname:Hosts,To,Record")
            , set_addons_arg          ("", "set-addons",    "Sets the addons that the given client shall enable. Use the syntax \"Hostname:List,Of,Addon,IDs\". You can only set enabled addons for clients that have already been added; i.e. the client will not be added automatically. If no tailing list of addon IDs is provided, all addons will be disabled.",        false, "Hostname:List,Of,Addon,IDs")
            , remove_client_arg       ("", "remove-client", "Removes the given client. If the client had any addons enabled, those are removed as well.",        false, "Hostname")
            , pre_buffer_secs_arg     ("", "pre-buffer",    "Set pre-buffer length to the given amount of seconds. Use a value <= 0 to turn pre-buffering off.", false, "", "Seconds")
            , blacklist_arg           ("", "blacklist",     "Record all topics except the listed ones (Comma separated list, e.g.: \"Topic1,Topic2\"). Use an empty blacklist to record all topics.",         false, "", "list")
            , whitelist_arg           ("", "whitelist",     "Only record these topics (Comma separated list, e.g.: \"Topic1,Topic2\").",                         false, "", "List")
            , meas_root_dir_arg       ("", "meas-root-dir", "Root dir used for recording",                                                                       false, "", "Path")
            , meas_name_arg           ("", "meas-name",     "Name of the measurement",                                                                           false, "", "String")
            , max_file_size_mib_arg   ("", "max-file-size", "Maximum file size of the recording files",                                                          false, "", "Megabytes")
            , description_arg         ("", "description",   "Description stored in the measurement folder.",                                                     false, "", "String")
            , enable_one_file_per_topic_arg ("", "enable-one-file-per-topic", "Whether to separate each topic in HDF5 file.", false, false, "yes/no")
            , ftp_server_arg          ("", "ftp-server",    "The server where to upload to when uploading a measurement. Use \"internal\" for the integrated FTP Server. When using an external FTP Server, provide it in the following form: ftp://USERNAME:PASSWORD@HOSTNAME:PORT/path/to/root_dir", false, "", "FTP_Server")
            , delete_after_upload_arg ("", "delete-after-upload", "Whether to delete the local measurement files after they have been uploaded to an FTP server.", false, false, "yes/no")
            , enable_built_in_client_arg("", "enable-built-in-client", "Whether the built-in recorder client of the host-application shall be used for recording. If turned off, the host application will rely on the presence of a separate rec-client for localhost recording.", false, false, "yes/no")
            , help_arg                ("", "help",          "Print additional help",                                                                             false)
          {
            std::vector<TCLAP::Arg*> arg_vector =
            {
              // WARNING: Wenn adding parameters here, they have also been added to the Usage() function
              &set_client_arg,
              &set_addons_arg,
              &remove_client_arg,
              &pre_buffer_secs_arg,
              &blacklist_arg,
              &whitelist_arg,
              &meas_root_dir_arg,
              &meas_name_arg,
              &max_file_size_mib_arg,
              &description_arg,
              &ftp_server_arg,
              &delete_after_upload_arg,
              &enable_one_file_per_topic_arg,
              &enable_built_in_client_arg,
              &help_arg,
              &unlabled_arg,
            };

            for (auto arg_iterator = arg_vector.rbegin(); arg_iterator != arg_vector.rend(); ++arg_iterator)
              cmd.add(*arg_iterator);

            advanced_tclap_output.setArgumentHidden(&unlabled_arg, true);
            cmd.setOutput(&advanced_tclap_output);
          }

          void parse(const std::vector<std::string>& argv)
          {
            std::vector<std::string> argv_with_program_name;
            argv_with_program_name.reserve(argv.size() + 1);
            argv_with_program_name.push_back("");

            for (size_t i = 0; i < argv.size(); i++)
              argv_with_program_name.push_back(argv[i]);

            cmd.parse(argv_with_program_name);
          }

          std::string help()
          {
            tclap_output_stream.clear();
            advanced_tclap_output.usage(cmd);
            return tclap_output_stream.str();
          }

          TCLAP::CmdLine                   cmd;
          std::stringstream                tclap_output_stream;
          CustomTclap::AdvancedTclapOutput advanced_tclap_output;

          TCLAP::UnlabeledMultiArg<std::string> unlabled_arg;
          TCLAP::MultiArg<std::string>          set_client_arg;
          TCLAP::MultiArg<std::string>          set_addons_arg;
          TCLAP::MultiArg<std::string>          remove_client_arg;
          TCLAP::ValueArg<std::string>          pre_buffer_secs_arg;   // => double!
          TCLAP::ValueArg<std::string>          blacklist_arg;
          TCLAP::ValueArg<std::string>          whitelist_arg;
          TCLAP::ValueArg<std::string>          meas_root_dir_arg;
          TCLAP::ValueArg<std::string>          meas_name_arg;
          TCLAP::ValueArg<std::string>          max_file_size_mib_arg; // => unsigned int!
          TCLAP::ValueArg<std::string>          description_arg;
          CustomTclap::FuzzyValueSwitchArgBool  enable_one_file_per_topic_arg;
          TCLAP::ValueArg<std::string>          ftp_server_arg;
          CustomTclap::FuzzyValueSwitchArgBool  delete_after_upload_arg;
          CustomTclap::FuzzyValueSwitchArgBool  enable_built_in_client_arg;
          TCLAP::SwitchArg                      help_arg;
        };

      //////////////////////////////////////////////
      /// Command functions (-> overrides)
      //////////////////////////////////////////////
      public:
        std::string Usage()    const override;
        std::string Help()     const override;
        std::string Example()  const override;

        eCAL::rec::Error Execute(const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::vector<std::string>& argv) const override;
        eCAL::rec::Error Execute(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::vector<std::string>& argv) const override;

      //////////////////////////////////////////////
      /// Parse Param Functions
      //////////////////////////////////////////////
      
      private:
        static eCAL::rec::Error ParseParams_SetClient     (const std::vector<std::string>& param, std::map<std::string, eCAL::rec_server::ClientConfig>& client_configs_out);
        static eCAL::rec::Error ParseParams_SetAddons     (const std::vector<std::string>& param, std::map<std::string, eCAL::rec_server::ClientConfig>& client_configs_out);
        static eCAL::rec::Error ParseParams_SetFtpServer  (const std::string& param,              eCAL::rec_server::UploadConfig&                        upload_config_out);
        static eCAL::rec::Error ParseParams_SetPreBuffer  (const std::string& param,              std::chrono::duration<double>&                         pre_buffer_length_out);
        static eCAL::rec::Error ParseParams_TopicList     (const std::string& param,              std::set<std::string>&                                 topic_list_out);
        static eCAL::rec::Error ParseParams_SetMaxFileSize(const std::string& param,              unsigned int&                                          max_file_size_mib_out);

      //////////////////////////////////////////////
      /// SetConfig functions (remote directly)
      //////////////////////////////////////////////
      private:
        template <typename param_t>
        static eCAL::rec::Error SetConfigDirectly(const std::string& hostname
                                                , const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service
                                                , param_t param
                                                , const std::function<eCAL::rec::Error(eCAL::pb::rec_server::RecServerConfig&, param_t)> set_config_function)
        {
          const eCAL::pb::rec_server::GenericRequest      request_pb;
          eCAL::pb::rec_server::RecServerConfig           rec_server_config_pb;

          // Get current config
          {
            eCAL::rec::Error service_call_error = CallRemoteEcalrecService(remote_rec_server_service, hostname, "GetConfig", request_pb, rec_server_config_pb);
            if (service_call_error)
              return service_call_error;
          }
          
          // Apply the given changes
          {
            eCAL::rec::Error error = set_config_function(rec_server_config_pb, param);
            if (error)
              return error;
          }
          
          // Send changes to remote server
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

          return eCAL::rec::Error::OK;
        }

      public:
        static eCAL::rec::Error SetClient                (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::vector<std::string>& param);
        static eCAL::rec::Error SetAddons                (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::vector<std::string>& param);
        static eCAL::rec::Error RemoveClient             (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::vector<std::string>& param);
        static eCAL::rec::Error SetFtpServer             (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::string& param);
        static eCAL::rec::Error SetDeleteAfterUpload     (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, bool param);
        static eCAL::rec::Error SetBuiltInClientEnabled  (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, bool param);
        static eCAL::rec::Error SetPreBuffer             (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::string& param);
        static eCAL::rec::Error SetBlacklist             (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::string& param);
        static eCAL::rec::Error SetWhitelist             (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::string& param);
        static eCAL::rec::Error SetMeasRootDir           (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::string& param);
        static eCAL::rec::Error SetMeasName              (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::string& param);
        static eCAL::rec::Error SetMaxFileSize           (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::string& param);
        static eCAL::rec::Error SetDescription           (const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::string& param);
        static eCAL::rec::Error SetOneFilePerTopicEnabled(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, bool param);

      //////////////////////////////////////////////
      /// SetConfig functions (Protobuf config)
      //////////////////////////////////////////////
      private:
        static eCAL::rec::Error SetClient                (eCAL::pb::rec_server::RecServerConfig& config_pb, const std::vector<std::string>& param);
        static eCAL::rec::Error SetAddons                (eCAL::pb::rec_server::RecServerConfig& config_pb, const std::vector<std::string>& param);
        static eCAL::rec::Error RemoveClient             (eCAL::pb::rec_server::RecServerConfig& config_pb, const std::vector<std::string>& param);
        static eCAL::rec::Error SetFtpServer             (eCAL::pb::rec_server::RecServerConfig& config_pb, const std::string& param);
        static eCAL::rec::Error SetDeleteAfterUpload     (eCAL::pb::rec_server::RecServerConfig& config_pb, bool param);
        static eCAL::rec::Error SetBuiltInClientEnabled  (eCAL::pb::rec_server::RecServerConfig& config_pb, bool param);
        static eCAL::rec::Error SetPreBuffer             (eCAL::pb::rec_server::RecServerConfig& config_pb, const std::string& param);
        static eCAL::rec::Error SetBlacklist             (eCAL::pb::rec_server::RecServerConfig& config_pb, const std::string& param);
        static eCAL::rec::Error SetWhitelist             (eCAL::pb::rec_server::RecServerConfig& config_pb, const std::string& param);
        static eCAL::rec::Error SetMeasRootDir           (eCAL::pb::rec_server::RecServerConfig& config_pb, const std::string& param);
        static eCAL::rec::Error SetMeasName              (eCAL::pb::rec_server::RecServerConfig& config_pb, const std::string& param);
        static eCAL::rec::Error SetMaxFileSize           (eCAL::pb::rec_server::RecServerConfig& config_pb, const std::string& param);
        static eCAL::rec::Error SetDescription           (eCAL::pb::rec_server::RecServerConfig& config_pb, const std::string& param);
        static eCAL::rec::Error SetOneFilePerTopicEnabled(eCAL::pb::rec_server::RecServerConfig& config_pb, bool param);

      //////////////////////////////////////////////
      /// SetConfig functions (local)
      //////////////////////////////////////////////
      public:
        static eCAL::rec::Error SetClient           (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::vector<std::string>& param);
        static eCAL::rec::Error SetAddons           (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::vector<std::string>& param);
        static eCAL::rec::Error RemoveClient        (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::vector<std::string>& param);
        static eCAL::rec::Error SetFtpServer        (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& param);
        static eCAL::rec::Error SetDeleteAfterUpload(const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, bool param);
        static eCAL::rec::Error SetBuiltInClientEnabled(const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, bool param);
        static eCAL::rec::Error SetPreBuffer        (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& param);
        static eCAL::rec::Error SetBlacklist        (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& param);
        static eCAL::rec::Error SetWhitelist        (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& param);
        static eCAL::rec::Error SetMeasRootDir      (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& param);
        static eCAL::rec::Error SetMeasName         (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& param);
        static eCAL::rec::Error SetMaxFileSize      (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& param);
        static eCAL::rec::Error SetDescription      (const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& param);
        static eCAL::rec::Error SetOneFilePerTopicEnabled(const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, bool param);
      };
    }
  }
}
