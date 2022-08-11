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

#include "command.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings 
#endif
#include <ecal/app/pb/rec/server_state.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <rec_server_core/proto_helpers.h>

namespace eCAL
{
  namespace rec_cli
  {
    namespace command
    {
      eCAL::rec::Error Command::GetRemoteStatus(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, eCAL::rec_server::RecServerStatus& status_output)
      {
        eCAL::pb::rec_server::Status status_pb;

        eCAL::rec::Error service_call_error = CallRemoteEcalrecService(remote_rec_server_service, hostname, "GetStatus", eCAL::pb::rec_server::GenericRequest(), status_pb);

        if (service_call_error)
        {
          return service_call_error;
        }
        else
        {
          eCAL::rec_server::proto_helpers::FromProtobuf(status_pb, status_output);
          return eCAL::rec::Error::OK;
        }
      }
    
      eCAL::rec::Error Command::GetRemoteConfig(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, eCAL::rec_server::RecServerConfig& config_output)
      {
        // Service call
        eCAL::pb::rec_server::GenericRequest            request_pb;
        eCAL::pb::rec_server::RecServerConfig           response_pb;

        eCAL::rec::Error service_call_error = CallRemoteEcalrecService(remote_rec_server_service, hostname, "GetConfig", request_pb, response_pb);

        // Service call failed
        if (service_call_error)
        {
          return service_call_error;
        }
        else
        {
          eCAL::rec_server::proto_helpers::FromProtobuf(response_pb, config_output);
          return eCAL::rec::Error::OK;
        }
      }

      eCAL::rec::Error Command::CallRemoteEcalrecService(const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_ecalsys_service
                                                        , const std::string&                hostname
                                                        , const std::string&                method_name
                                                        , const google::protobuf::Message&  request
                                                        , google::protobuf::Message&        response)
      {
        remote_ecalsys_service->SetHostName(hostname);

        eCAL::ServiceResponseVecT service_response_vec;
        constexpr int timeout_ms = 1000;

        if (remote_ecalsys_service->Call(method_name, request.SerializeAsString(), timeout_ms, &service_response_vec))
        {
          if (service_response_vec.size() > 0)
          {
            response.ParseFromString(service_response_vec[0].response);
            return eCAL::rec::Error::ErrorCode::OK;
          }
        }
        return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::REMOTE_HOST_UNAVAILABLE, hostname);
      }
    }
  }
}
