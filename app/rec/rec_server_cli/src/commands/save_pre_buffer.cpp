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

#include "save_pre_buffer.h"

#include <rec_server_core/proto_helpers.h>

#include <ecal_utils/string.h>

#include <iostream>

namespace eCAL
{
  namespace rec_cli
  {
    namespace command
    {

      std::string SavePreBuffer::Usage() const
      {
        return "";
      }

      std::string SavePreBuffer::Help() const
      {
        return "Saves the content of the current pre-buffer to its own measurement.";
      }

      std::string SavePreBuffer::Example() const
      {
        return "";
      }

      eCAL::rec::Error SavePreBuffer::Execute(const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::vector<std::string>& argv) const
      {
        // Arg check
        if (argv.size() > 0)
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::TOO_MANY_PARAMETERS, EcalUtils::String::Join(" ", std::vector<std::string>(std::next(argv.begin()), argv.end())));

        // Status check
        if (!rec_server_instance->IsConnectedToEcal())
          return eCAL::rec::Error(eCAL::rec::Error::ACTION_SUPERFLOUS, "Not activated.");
        if (!rec_server_instance->GetPreBufferingEnabled())
          return eCAL::rec::Error(eCAL::rec::Error::ACTION_SUPERFLOUS, "Pre-buffering is disabled");

        bool success = rec_server_instance->SavePreBufferedData();
        if (!success)
          return eCAL::rec::Error(eCAL::rec::Error::GENERIC_ERROR, "Failed to save pre-buffer");
        else
          return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error SavePreBuffer::Execute(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::vector<std::string>& argv) const
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

        // Status check
        if (!status.activated_)
          return eCAL::rec::Error(eCAL::rec::Error::ACTION_SUPERFLOUS, "Not activated.");
        
        // TODO: Also check if pre-buffering was enabled (Currently not supported by the API)

        // Service call
        SServiceResponse                         service_response;
        eCAL::pb::rec_server::GenericRequest     request_pb;
        eCAL::pb::rec_server::JobStartedResponse response_pb;

        bool success = remote_rec_server_service->Call(hostname, "SaveBuffer", request_pb, service_response, response_pb);

        // Service call failed
        if (!success)
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::REMOTE_HOST_UNAVAILABLE, hostname);

        // Service call reported error
        {
          eCAL::rec::Error error = eCAL::rec_server::proto_helpers::FromProtobuf(response_pb.service_result());
          if (error)
            return error;
        }

        // Success!
        std::cout << "ID: " + std::to_string(response_pb.meas_id()) << std::endl;
        return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::OK, "ID: " + std::to_string(response_pb.meas_id()));
      }
    }
  }
}
