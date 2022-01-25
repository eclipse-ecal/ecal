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

#include "load_config.h"

#include <rec_server_core/proto_helpers.h>

#include <ecal_utils/string.h>

#include <iostream>

namespace eCAL
{
  namespace rec_cli
  {
    namespace command
    {

      std::string LoadConfig::Usage() const
      {
        return "<Path>";
      }

      std::string LoadConfig::Help() const
      {
        return "Loads an eCAL Rec configuration file.";
      }

      std::string LoadConfig::Example() const
      {
        return "\"/path/to/config.ecalrec\"";
      }

      eCAL::rec::Error LoadConfig::Execute(const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::vector<std::string>& argv) const
      {
        // Arg check
        if (argv.empty())
          return eCAL::rec::Error::ErrorCode::PARAMETER_ERROR;
        else if (argv.size() > 1)
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::TOO_MANY_PARAMETERS, EcalUtils::String::Join(" ", std::vector<std::string>(std::next(argv.begin()), argv.end())));

        // Currently recording
        if (rec_server_instance->IsRecording())
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::CURRENTLY_RECORDING);

        bool success = rec_server_instance->LoadConfigFromFile(argv[0]);

        if (!success)
        {
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::RESOURCE_UNAVAILABLE, "Failed to load config file \"" + argv[0] + "\"");
        }
        else
        {
          if (rec_server_instance->GetLoadedConfigVersion() < rec_server_instance->GetNativeConfigVersion())
            std::cout << "WARNING: This config file has been created with an old version of eCAL rec. When saving, the config will be converted to the new format. Old eCAL Rec versions may not be able to open that file." << std::endl;
          else if (rec_server_instance->GetLoadedConfigVersion() > rec_server_instance->GetNativeConfigVersion())
            std::cout << "WARNING: This config file has been created with a newer version of eCAL rec. When saving the configuration, all unrecognized settings will be lost." << std::endl;

          return eCAL::rec::Error::ErrorCode::OK;
        }
      }

      eCAL::rec::Error LoadConfig::Execute(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::vector<std::string>& argv) const
      {
        // Arg check
        if (argv.empty())
          return eCAL::rec::Error::ErrorCode::PARAMETER_ERROR;
        else if (argv.size() > 1)
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::TOO_MANY_PARAMETERS, EcalUtils::String::Join(" ", std::vector<std::string>(std::next(argv.begin()), argv.end())));

        // Retrieve Status
        eCAL::rec_server::RecServerStatus status;
        {
          eCAL::rec::Error error = GetRemoteStatus(hostname, remote_rec_server_service, status);
          if (error)
            return error;
        }

        // Currently recording
        if (status.recording_meas_id_ != 0)
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::CURRENTLY_RECORDING);

        // Service call
        SServiceResponse                         service_response;
        eCAL::pb::rec_server::LoadConfigRequest  request_pb;
        eCAL::pb::rec_server::ServiceResult      response_pb;

        request_pb.set_config_path(argv[0]);

        bool success = remote_rec_server_service->Call(hostname, "LoadConfigFile", request_pb, service_response, response_pb);

        // Service call failed
        if (!success)
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::REMOTE_HOST_UNAVAILABLE, hostname);

        // Service call reported error
        {
          eCAL::rec::Error error = eCAL::rec_server::proto_helpers::FromProtobuf(response_pb);
          if (error)
            return error;
        }

        // Success!
        return eCAL::rec::Error::ErrorCode::OK;
      }
    }
  }
}
