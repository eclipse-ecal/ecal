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

#include "delete.h"

#include <rec_server_core/proto_helpers.h>

#include <ecal_utils/string.h>

namespace eCAL
{
  namespace rec_cli
  {
    namespace command
    {
      std::string Delete::Usage() const
      {
        return "<MeasID>";
      }

      std::string Delete::Help() const
      {
        return "Deletes the given measurement from all machines.";
      }

      std::string Delete::Example() const
      {
        return "1234";
      }

      eCAL::rec::Error Delete::Execute(const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::vector<std::string>& argv) const
      {
        // Arg check
        if (argv.size() == 0)
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "ID required");
        if (argv.size() > 1)
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::TOO_MANY_PARAMETERS, EcalUtils::String::Join(" ", std::vector<std::string>(std::next(argv.begin()), argv.end())));

        // (Try to) parse measurement ID
        int64_t meas_id = 0;
        // if (argv.size() == 1) this is always true
        {
          try
          {
            meas_id = std::stoll(argv[0]);
          }
          catch (const std::exception& e)
          {
            return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Unable to parse ID " + argv[0] + ": " + std::string(e.what()));
          }
        }
        
        // Delete Measurement
        return rec_server_instance->DeleteMeasurement(meas_id);
      }

      eCAL::rec::Error Delete::Execute(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::vector<std::string>& argv) const
      {
        // Arg check
        if (argv.size() == 0)
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "ID required");
        if (argv.size() > 1)
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::TOO_MANY_PARAMETERS, EcalUtils::String::Join(" ", std::vector<std::string>(std::next(argv.begin()), argv.end())));

        // (Try to) parse measurement ID
        int64_t meas_id = 0;
        // if (argv.size() == 1) this is always true
        {
          try
          {
            meas_id = std::stoll(argv[0]);
          }
          catch (const std::exception& e)
          {
            return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Unable to parse ID " + argv[0] + ": " + std::string(e.what()));
          }
        }
        
        // Check if measurement ID exists
        eCAL::rec_server::RecServerStatus status;
        {
          auto error = GetRemoteStatus(hostname, remote_rec_server_service, status);
          if (error)
            return error;
        }

        bool meas_id_found(false);
        for (const auto& job_history_entry : status.job_history_)
        {
          if (job_history_entry.local_evaluated_job_config_.GetJobId() == meas_id)
          {
            meas_id_found = true;
            break;
          }
        }

        if (!meas_id_found)
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::MEAS_ID_NOT_FOUND, std::to_string(meas_id));

        // Service call
        SServiceResponse                                service_response;
        eCAL::pb::rec_server::GenericMeasurementRequest request_pb;
        eCAL::pb::rec_server::ServiceResult             response_pb;

        request_pb.set_meas_id(meas_id);
        bool success = remote_rec_server_service->Call(hostname, "DeleteMeasurement", request_pb, service_response, response_pb);

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
