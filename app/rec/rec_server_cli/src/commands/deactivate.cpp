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

#include "deactivate.h"

#include <rec_server_core/proto_helpers.h>

#include <ecal_utils/string.h>

namespace eCAL
{
  namespace rec_cli
  {
    namespace command
    {

      std::string DeActivate::Usage() const
      {
        return "";
      }

      std::string DeActivate::Help() const
      {
        return "Deactivates all clients. They keep running but will not subscribe to any channels and buffer data until activated again.";
      }

      std::string DeActivate::Example() const
      {
        return "";
      }

      eCAL::rec::Error DeActivate::Execute(const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::vector<std::string>& argv) const
      {
        // Arg check
        if (argv.size() > 0)
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::TOO_MANY_PARAMETERS, EcalUtils::String::Join(" ", std::vector<std::string>(std::next(argv.begin()), argv.end())));

        // Currently recording
        if (rec_server_instance->IsRecording())
          return eCAL::rec::Error::CURRENTLY_RECORDING;

        bool success = rec_server_instance->SetConnectionToClientsActive(false);
        if (!success)
          return eCAL::rec::Error(eCAL::rec::Error::GENERIC_ERROR, "Failed deactivating recorders");
        else
          return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error DeActivate::Execute(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::vector<std::string>& argv) const
      {
        // Arg check
        if (argv.size() > 0)
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::TOO_MANY_PARAMETERS, EcalUtils::String::Join(" ", std::vector<std::string>(std::next(argv.begin()), argv.end())));

        // Retrieve Status
        eCAL::rec_server::RecServerStatus status;
        {
          eCAL::rec::Error error = GetRemoteStatus(hostname, remote_rec_server_service, status);
          if (error)
            return error;
        }

        // Currently Reording
        if (status.recording_meas_id_ != 0)
          return eCAL::rec::Error::CURRENTLY_RECORDING;

        // Service call
        SServiceResponse                     service_response;
        eCAL::pb::rec_server::GenericRequest request_pb;
        eCAL::pb::rec_server::ServiceResult  response_pb;

        bool success = remote_rec_server_service->Call(hostname, "DeActivate", request_pb, service_response, response_pb);

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
