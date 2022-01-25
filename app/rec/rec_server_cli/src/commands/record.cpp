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

#include "record.h"

#include <rec_server_core/proto_helpers.h>

#include <ecal_utils/string.h>

#include "stop_recording.h"

#include <iostream>

namespace eCAL
{
  namespace rec_cli
  {
    namespace command
    {

      std::string Record::Usage() const
      {
        return "[Seconds]";
      }

      std::string Record::Help() const
      {
        return "Starts a recording. If SECS_TO_RECORD is given, the recording will be stopped afterwards.";
      }

      std::string Record::Example() const
      {
        return "3.5";
      }

      eCAL::rec::Error Record::Execute(const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::vector<std::string>& argv) const
      {
        std::chrono::duration<double> time_to_rec(0);

        // Parse Command line
        {
          auto error = parseCmdLine(argv, time_to_rec);
          if (error)
            return error;
        }

        return Execute(rec_server_instance, time_to_rec);
      }

      eCAL::rec::Error Record::Execute(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::vector<std::string>& argv) const
      {
        std::chrono::duration<double> time_to_rec(0);

        // Parse Command line
        {
          auto error = parseCmdLine(argv, time_to_rec);
          if (error)
            return error;
        }

        return Execute(hostname, remote_rec_server_service, time_to_rec);
      }

      eCAL::rec::Error Record::Execute(const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, std::chrono::duration<double> time_to_rec) const
      {

        // Check if recording
        if (rec_server_instance->IsRecording())
          return eCAL::rec::Error(eCAL::rec::Error::CURRENTLY_RECORDING, "Cannot start two recordings at the same time");

        // Start recording
        {
          bool success = rec_server_instance->StartRecording();
          if (!success)
            return eCAL::rec::Error(eCAL::rec::Error::GENERIC_ERROR, "Failed to start recording");
        }

        if (time_to_rec > std::chrono::duration<double>(0))
        {
          // Wait while recording is running
          {
            auto error = sleep_command.Execute(time_to_rec);
            if (error)
              return error;
          }

          // Stop recording
          {
            auto error = StopRecording().Execute(rec_server_instance, {});
            if (error)
              return error;
          }
        }

        return eCAL::rec::Error::ErrorCode::OK;
      }

      eCAL::rec::Error Record::Execute(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, std::chrono::duration<double> time_to_rec) const
      {
        // Retrieve Status
        eCAL::rec_server::RecServerStatus status;
        {
          eCAL::rec::Error error = GetRemoteStatus(hostname, remote_rec_server_service, status);
          if (error)
            return error;
        }

        // Check if recording
        if(status.recording_meas_id_ != 0)
          return eCAL::rec::Error(eCAL::rec::Error::CURRENTLY_RECORDING, "Cannot start two recordings at the same time");

        // Start recording
        {
          SServiceResponse                         service_response;
          eCAL::pb::rec_server::GenericRequest     request_pb;
          eCAL::pb::rec_server::JobStartedResponse response_pb;

          bool success = remote_rec_server_service->Call(hostname, "StartRecording", request_pb, service_response, response_pb);

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
        }


        if (time_to_rec > std::chrono::duration<double>(0))
        {
          // Wait while recording is running
          {
            auto error = sleep_command.Execute(time_to_rec);
            if (error)
              return error;
          }

          // Stop recording
          {
            auto error = StopRecording().Execute(hostname, remote_rec_server_service, {});
            if (error)
              return error;
          }
        }

        return eCAL::rec::Error::ErrorCode::OK;
      }

      void Record::Interrupt() const
      {
        sleep_command.Interrupt();
      }

      eCAL::rec::Error Record::parseCmdLine(const std::vector<std::string>& argv, std::chrono::duration<double>& time_to_rec_out)
      {
        if (argv.size() == 0)
        {
          time_to_rec_out = std::chrono::duration<double>(0);
          return eCAL::rec::Error::ErrorCode::OK;
        }
        else if (argv.size() == 1)
        {
          std::chrono::duration<double> time_out;
          eCAL::rec::Error error = Sleep::parseTime(argv[0], time_out);

          if (error)
            return error;

          if (time_out <= std::chrono::duration<double>(0.0))
            return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Time to record must be greater than 0 seconds");

          time_to_rec_out = time_out;
          return error;
        }
        else
        {
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::TOO_MANY_PARAMETERS, EcalUtils::String::Join(" ", std::vector<std::string>(std::next(argv.begin()), argv.end())));
        }
      }
    }
  }
}
