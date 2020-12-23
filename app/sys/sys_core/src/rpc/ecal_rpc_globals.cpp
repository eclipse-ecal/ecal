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

#include "ecal_rpc_globals.h"

namespace eCAL
{
  namespace RPC
  {
    CGlobals::CGlobals()
    {}

    CGlobals::~CGlobals()
    {
      Finalize();
    };

    int CGlobals::Initialize()
    {
      if (rpc_instance != nullptr) return(1);
      rpc_instance = std::unique_ptr<protobuf::CServiceClient<eCAL::pb::rpcservice::Service>>(new protobuf::CServiceClient<eCAL::pb::rpcservice::Service>);
      return(0);
    }

    int CGlobals::IsInitialized()
    {
      return(rpc_instance != nullptr);
    }

    int CGlobals::Finalize()
    {
      if (rpc_instance == nullptr) return(1);
      rpc_instance = nullptr;
      return(0);
    }
  }
}
