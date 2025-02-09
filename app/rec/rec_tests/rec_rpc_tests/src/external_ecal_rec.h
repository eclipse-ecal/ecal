/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings 
#endif
#include <ecal/msg/protobuf/client.h>
#include <ecal/app/pb/rec/server_service.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <string>
#include <mutex>
#include <rec_server_core/rec_server_config.h>
#include <rec_client_core/rec_error.h>

// Lock preventing multiple instances of this class
static std::mutex ecal_rec_cli_instance_lock;

class ExternalEcalRecInstance
{
///////////////////////////////////////////////
// Constructor, destructor etc.
///////////////////////////////////////////////
public:
  ExternalEcalRecInstance(bool gui = false);

  // Copy
  ExternalEcalRecInstance(const ExternalEcalRecInstance&)            = delete;
  ExternalEcalRecInstance& operator=(const ExternalEcalRecInstance&) = delete;

  // Move
  ExternalEcalRecInstance& operator=(ExternalEcalRecInstance&&)      = default;
  ExternalEcalRecInstance(ExternalEcalRecInstance&&)                 = default;

  ~ExternalEcalRecInstance();

///////////////////////////////////////////////
// RPC Wrappers
///////////////////////////////////////////////
public:
  eCAL::rec::Error GetConfigViaRpc(eCAL::rec_server::RecServerConfig&     config_output);
  eCAL::rec::Error GetConfigViaRpc(eCAL::pb::rec_server::RecServerConfig& config_pb_output);

  eCAL::rec::Error SetConfigViaRpc(const eCAL::rec_server::RecServerConfig&     config);
  eCAL::rec::Error SetConfigViaRpc(const eCAL::pb::rec_server::RecServerConfig& config_pb);

///////////////////////////////////////////////
// Member Variables
///////////////////////////////////////////////
private:
  int process_id;
  std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>> remote_rec_server_service;
};
