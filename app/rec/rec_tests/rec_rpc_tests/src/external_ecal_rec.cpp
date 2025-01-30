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

#include "external_ecal_rec.h"

#include <rec_server_core/proto_helpers.h>

#include <ecal/ecal.h>
#include <ecal/msg/protobuf/client.h>
#include <ecal/msg/protobuf/server.h>

#include <string>
#include <iostream>
#include <thread>

///////////////////////////////////////////////
// Constructor, destructor etc.
///////////////////////////////////////////////

ExternalEcalRecInstance::ExternalEcalRecInstance(bool gui)
  : process_id(0)
{
  ecal_rec_cli_instance_lock.lock();

  eCAL::Initialize("Ecal Rec Tester");
  
  remote_rec_server_service = std::make_shared<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>();
  remote_rec_server_service->SetHostName(eCAL::Process::GetHostName());

  if (gui)
  {
    std::cout << "Starting " << "\"" << ECAL_REC_GUI_PATH << "\"" << std::endl;
    process_id = eCAL::Process::StartProcess(ECAL_REC_GUI_PATH, "", "", false, eCAL::Process::eStartMode::minimized, false);
  }
  else
  {
    std::cout << "Starting " << "\"" << ECAL_REC_CLI_PATH << "\"" << std::endl;
    process_id = eCAL::Process::StartProcess(ECAL_REC_CLI_PATH, "--interactive-dont-exit --no-default", "", false, eCAL::Process::eStartMode::hidden, false);
  }

  if (process_id != 0)
  {
    std::cout << "Successfully started eCAL Rec " << (gui ? "GUI" : "CLI") << " with PID " << process_id << std::endl;
  }
  else
  {
    std::cerr << "Error starting eCAL Rec " << (gui ? "GUI" : "CLI") << std::endl;
    return;
  }

  for (int i = 0; i < 20; ++i)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    eCAL::pb::rec_server::RecServerConfig config_pb;
    auto error = GetConfigViaRpc(config_pb);
    if (!error)
    {
      break;
    }
  }

  if (gui)
  {
    auto error = SetConfigViaRpc(eCAL::rec_server::RecServerConfig());
    if (error)
      std::cerr << "Error initializing empty config in rec GUI: " << error.ToString() << std::endl;
  }
}


ExternalEcalRecInstance::~ExternalEcalRecInstance()
{
  if (process_id > 0)
    eCAL::Process::StopProcess(process_id);
  eCAL::Finalize();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  ecal_rec_cli_instance_lock.unlock();
}

///////////////////////////////////////////////
// RPC Wrappers
///////////////////////////////////////////////

eCAL::rec::Error ExternalEcalRecInstance::GetConfigViaRpc(eCAL::rec_server::RecServerConfig& config_output)
{
  eCAL::pb::rec_server::RecServerConfig config_pb;
  
  {
    auto error = GetConfigViaRpc(config_pb);
    if (error)
    {
      std::cerr << error.ToString() << std::endl;
      return error;
    }
  }

  eCAL::rec_server::proto_helpers::FromProtobuf(config_pb, config_output);

  return eCAL::rec::Error::OK;
}

eCAL::rec::Error ExternalEcalRecInstance::GetConfigViaRpc(eCAL::pb::rec_server::RecServerConfig& config_pb_output)
{
  eCAL::pb::rec_server::GenericRequest request;
  eCAL::v5::ServiceResponseVecT            service_response_vec;

  constexpr int timeout_ms = 1000;

  if (remote_rec_server_service->Call("GetConfig", request.SerializeAsString(), timeout_ms, &service_response_vec))
  {
    if (service_response_vec.size() > 0)
    {
      config_pb_output.ParseFromString(service_response_vec[0].response);
      return eCAL::rec::Error::ErrorCode::OK;
    }
  }
  return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::REMOTE_HOST_UNAVAILABLE);
}


eCAL::rec::Error ExternalEcalRecInstance::SetConfigViaRpc(const eCAL::rec_server::RecServerConfig& config)
{
  return SetConfigViaRpc(eCAL::rec_server::proto_helpers::ToProtobuf(config));
}

eCAL::rec::Error ExternalEcalRecInstance::SetConfigViaRpc(const eCAL::pb::rec_server::RecServerConfig& config_pb)
{
  eCAL::v5::ServiceResponseVecT service_response_vec;

  constexpr int timeout_ms = 1000;

  if (remote_rec_server_service->Call("SetConfig", config_pb.SerializeAsString(), timeout_ms, &service_response_vec))
  {
    if (service_response_vec.size() > 0)
    {
      eCAL::pb::rec_server::ServiceResult response_pb;

      response_pb.ParseFromString(service_response_vec[0].response);

      if (response_pb.error_code() != eCAL::pb::rec_server::ServiceResult_ErrorCode_no_error)
      {
        return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR, response_pb.info_message());
      }
      else
      {
        return eCAL::rec::Error::ErrorCode::OK;
      }
    }
  }

  return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::REMOTE_HOST_UNAVAILABLE);
}
