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

#include "comment.h"

#include <rec_server_core/proto_helpers.h>

#include <ecal_utils/string.h>

namespace eCAL
{
  namespace rec_cli
  {
    namespace command
    {

      std::string Comment::Usage() const
      {
        return "[MeasID] <Comment>";
      }

      std::string Comment::Help() const
      {
        return "Adds a comment to a measurement. If no ID is given, the comment will be added to the last measurement.";
      }

      std::string Comment::Example() const
      {
        return "1234 \"My first comment\"";
      }

      eCAL::rec::Error Comment::Execute(const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::vector<std::string>& argv) const
      {
        // Arg check
        if (argv.size() < 1)
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Comment required");
        if (argv.size() > 2)
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::TOO_MANY_PARAMETERS, EcalUtils::String::Join(" ", std::vector<std::string>(std::next(argv.begin()), argv.end())));
        
        if (argv.size() == 2)
        {
          // (Try to) parse measurement ID
          int64_t meas_id = 0;
          try
          {
            meas_id = std::stoll(argv[0]);
          }
          catch (const std::exception& e)
          {
            return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Unable to parse ID " + argv[0] + ": " + std::string(e.what()));
          }

          return rec_server_instance->AddComment(meas_id, argv[1]);
        }
        else
        {
          auto job_history = rec_server_instance->GetJobHistory();
          if (job_history.empty())
          {
            return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR, "No recent recordings");
          }
          else
          {
            return rec_server_instance->AddComment(job_history.back().local_evaluated_job_config_.GetJobId(), argv[0]);
          }
        }
      }

      eCAL::rec::Error Comment::Execute(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::vector<std::string>& argv) const
      {
        // Arg check
        if (argv.size() < 1)
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Comment required");
        if (argv.size() > 2)
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::TOO_MANY_PARAMETERS, EcalUtils::String::Join(" ", std::vector<std::string>(std::next(argv.begin()), argv.end())));

        // (Try to) parse measurement ID
        int64_t meas_id = 0;
        const std::string* comment;
        if (argv.size() == 2)
        {
          try
          {
            meas_id = std::stoll(argv[0]);
          }
          catch (const std::exception& e)
          {
            return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Unable to parse ID " + argv[0] + ": " + std::string(e.what()));
          }
          comment = &argv[1];
        }
        else
        {
          comment = &argv[0];
        }
        
        // Service call
        SServiceResponse                        service_response;
        eCAL::pb::rec_server::AddCommentRequest request_pb;
        eCAL::pb::rec_server::ServiceResult     response_pb;

        request_pb.set_meas_id(meas_id);
        request_pb.set_comment(*comment);

        bool success = remote_rec_server_service->Call(hostname, "AddComment", request_pb, service_response, response_pb);

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
