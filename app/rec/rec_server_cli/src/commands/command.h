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

#include <vector>
#include <string>
#include <memory>

#include <rec_server_core/rec_server.h>
#include <rec_server_core/status.h>
#include <rec_client_core/rec_error.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings 
#endif
#include <ecal/msg/protobuf/client.h>
#include <ecal/pb/rec/server_service.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif


namespace eCAL
{
  namespace rec_cli
  {
    namespace command
    {
      class Command
      {
      public:
        virtual ~Command() {};

        virtual std::string Usage()    const = 0;
        virtual std::string Help()     const = 0;
        virtual std::string Example()  const = 0;

        virtual eCAL::rec::Error Execute(const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::vector<std::string>& argv) const = 0;
        virtual eCAL::rec::Error Execute(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::vector<std::string>& argv) const = 0;

        virtual void Interrupt() const {};

        static eCAL::rec::Error GetRemoteStatus(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, eCAL::rec_server::RecServerStatus& status_output);
        static eCAL::rec::Error GetRemoteConfig(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, eCAL::rec_server::RecServerConfig& config_output);
      };
    }
  }
}