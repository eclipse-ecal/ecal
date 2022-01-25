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
#include <ecal/pb/rec/server_state.pb.h>
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
        SServiceResponse service_response;
        eCAL::pb::rec_server::Status status_pb;

        bool success = remote_rec_server_service->Call(hostname, "GetStatus", eCAL::pb::rec_server::GenericRequest(), service_response, status_pb);

        if (!success)
        {
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::REMOTE_HOST_UNAVAILABLE, hostname);
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
        SServiceResponse                                service_response;
        eCAL::pb::rec_server::GenericRequest            request_pb;
        eCAL::pb::rec_server::RecServerConfig           response_pb;

        bool success = remote_rec_server_service->Call(hostname, "GetConfig", request_pb, service_response, response_pb);

        // Service call failed
        if (!success)
        {
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::REMOTE_HOST_UNAVAILABLE, hostname);
        }
        else
        {
          eCAL::rec_server::proto_helpers::FromProtobuf(response_pb, config_output);
          return eCAL::rec::Error::OK;
        }
      }
    }
  }
}
