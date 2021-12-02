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

#include "stop_recording.h"

#include <ecal_utils/string.h>

#include <rec_server_core/proto_helpers.h>

namespace eCAL
{
  namespace rec_cli
  {
    namespace command
    {

      std::string StopRecording::Usage() const
      {
        return "";
      }

      std::string StopRecording::Help() const
      {
        return "Stops the current recording.";
      }

      std::string StopRecording::Example() const
      {
        return "";
      }

      eCAL::rec::Error StopRecording::Execute(const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::vector<std::string>& argv) const
      {
        // Arg check
        if (argv.size() > 0)
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::TOO_MANY_PARAMETERS, EcalUtils::String::Join(" ", std::vector<std::string>(std::next(argv.begin()), argv.end())));

        // Nothing to stop
        if (!rec_server_instance->IsRecording())
          return eCAL::rec::Error::NOT_RECORDING;

        bool success = rec_server_instance->StopRecording();
        if (!success)
          return eCAL::rec::Error(eCAL::rec::Error::GENERIC_ERROR, "Failed to stop recording");
        else
          return eCAL::rec::Error::OK;
      }

      eCAL::rec::Error StopRecording::Execute(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::vector<std::string>& argv) const
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

        // Nothing to stop
        if (status.recording_meas_id_ == 0)
          return eCAL::rec::Error::NOT_RECORDING;

        // Service call
        SServiceResponse                     service_response;
        eCAL::pb::rec_server::GenericRequest request_pb;
        eCAL::pb::rec_server::ServiceResult  response_pb;

        bool success = remote_rec_server_service->Call(hostname, "StopRecording", request_pb, service_response, response_pb);

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
