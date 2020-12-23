/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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

/**
 * @brief  eCAL rpc globals
**/

#pragma once

#include <ecal/msg/protobuf/client.h>
#include <ecal/pb/rpcservice/service.pb.h>

namespace eCAL
{
  namespace RPC
  {
    class CGlobals
    {
    public:
      CGlobals();
      ~CGlobals();

      int Initialize();
      int IsInitialized();

      int Finalize();

      const std::unique_ptr<protobuf::CServiceClient<eCAL::pb::rpcservice::Service>>& rpc() { return rpc_instance; };

    private:
      std::unique_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rpcservice::Service>> rpc_instance;
    };
  }
}
