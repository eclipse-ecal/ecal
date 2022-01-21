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

#include <ecal/ecal.h>
#include <ecal/msg/protobuf/client.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100 4505 4800)
#endif
#include <ecal/pb/play/service.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <iostream>
#include <chrono>
#include <thread>

// callback for player service response
void OnPlayerResponse(const struct eCAL::SServiceResponse& service_response_)
{
  switch (service_response_.call_state)
  {
  // service successful executed
  case call_state_executed:
  {
    if (service_response_.method_name == "GetConfig")
    {
      eCAL::pb::play::GetConfigResponse response;
      response.ParseFromString(service_response_.response);
      std::cout << "PlayerService " << service_response_.method_name << " called successfully on host " << service_response_.host_name << std::endl;
      std::cout << response.DebugString();
    }
    else if (service_response_.method_name == "Response")
    {
      eCAL::pb::play::Response response;
      response.ParseFromString(service_response_.response);
      std::cout << "PlayerService " << service_response_.method_name << " called successfully on host " << service_response_.host_name << std::endl;
      std::cout << response.DebugString();
    }
    else if (service_response_.method_name == "GetState")
    {
        eCAL::pb::play::State response;
        response.ParseFromString(service_response_.response);
        std::cout << "PlayerService " << service_response_.method_name << " called successfully on host " << service_response_.host_name << std::endl;
        std::cout << response.DebugString();
    }
    else
    {
        std::cout << "PlayerService " << service_response_.method_name << " received unknown message on host " << service_response_.host_name << std::endl;
    }
  }
  break;
  // service execution failed
  case call_state_failed:
  {
    eCAL::pb::play::Response response;
    response.ParseFromString(service_response_.response);
    std::cout << "PlayerService " << service_response_.method_name << " failed with \"" << response.error() << "\" on host " << service_response_.host_name << std::endl;
    std::cout << response.DebugString();
  }
  break;
  default:
    break;
  }
}

// main entry
int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "ecalplayer client");

  // create player service client
  eCAL::protobuf::CServiceClient<eCAL::pb::play::EcalPlayService> player_service;
  player_service.AddResponseCallback(OnPlayerResponse);

  // sleep for service matching
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  // requests
  eCAL::pb::play::CommandRequest   command_request;
  eCAL::pb::play::GetConfigRequest get_config_request;
  eCAL::pb::play::SetConfigRequest set_config_request;

  while (eCAL::Ok())
  {
    // reset requests
    command_request.Clear();
    get_config_request.Clear();
    set_config_request.Clear();

    ////////////////////////////////////
    // "GetConfig"                    //
    ////////////////////////////////////
    std::cout << "eCAL::pb::play::EcalPlayService:GetConfig()" << std::endl;
    player_service.Call("GetConfig", get_config_request);
    std::cout << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    ////////////////////////////////////
    // "SetConfig"                    //
    ////////////////////////////////////
    std::cout << "eCAL::pb::play::EcalPlayService:SetConfig()" << std::endl;
    auto config = set_config_request.mutable_config()->mutable_items();

    (*config)["measurement_path"]              = "C:\\meas01";                  // Current measurement path (or empty string if no measurement is loaded)
    (*config)["limit_play_speed"]              = "true";                        // Whether the playback is limited to the play_speed value (if false, the player publishes all frames as fast as possible)
    (*config)["play_speed"]                    = "2.0";                         // Play speed factor (1.0 indicates realtime playback)
    (*config)["frame_dropping_allowed"]        = "false";                       // Whether the player will drop frames instead of slowing down the playback
    (*config)["enforce_delay_accuracy"]        = "false";                       // Whether the player tries to always keep the correct amount of time between two frames, even if this may slow down the measurement
    (*config)["repeat"]                        = "true";                        // Repeat playback from the beginning if the end has been reached
    (*config)["limit_interval_start_rel_secs"] = "1.0";                         // Start the playback from this time (relative value in seconds, 0.0 indicates the begin of the measurement)
    (*config)["limit_interval_end_rel_secs"]   = "8.0";                         // End the playback at this time (relative value in seconds)

    player_service.Call("SetConfig", set_config_request);
    std::cout << set_config_request.DebugString() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    player_service.Call("GetState", eCAL::pb::play::Empty());
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    ////////////////////////////////////
    // "SetCommand"                   //
    ////////////////////////////////////

    // "SetCommand - initialize"
    std::cout << "eCAL::pb::play::EcalPlayService:SetCommand() - initialize" << std::endl;
    command_request.set_command(eCAL::pb::play::CommandRequest::initialize);
    player_service.Call("SetCommand", command_request);
    std::cout << command_request.DebugString() << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    command_request.Clear();

    // "SetCommand - jump_to"
    std::cout << "eCAL::pb::play::EcalPlayService:SetCommand() - jum_to 2.0s" << std::endl;
    command_request.set_command(eCAL::pb::play::CommandRequest::jump_to);
    command_request.set_rel_time_secs(2.0);
    player_service.Call("SetCommand", command_request);
    std::cout << command_request.DebugString() << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    command_request.Clear();

    // "SetCommand - play"
    std::cout << "eCAL::pb::play::EcalPlayService:SetCommand() - play" << std::endl;
    command_request.set_command(eCAL::pb::play::CommandRequest::play);
    player_service.Call("SetCommand", command_request);
    std::cout << command_request.DebugString() << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    command_request.Clear();

    // "SetCommand - pause"
    std::cout << "eCALPB.Play.GUIService:SetCommand() - pause" << std::endl;
    command_request.set_command(eCAL::pb::play::CommandRequest::pause);
    player_service.Call("SetCommand", command_request);
    std::cout << command_request.DebugString() << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    command_request.Clear();

    // "SetCommand - step"
    std::cout << "eCALPB.Play.GUIService:SetCommand() - step" << std::endl;
    command_request.set_command(eCAL::pb::play::CommandRequest::step);
    player_service.Call("SetCommand", command_request);
    std::cout << command_request.DebugString() << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    command_request.Clear();

    // "SetCommand - step_channel"
    std::cout << "eCALPB.Play.GUIService:SetCommand() - step_channel \"VehiclePosePb\"" << std::endl;
    command_request.set_command(eCAL::pb::play::CommandRequest::step_channel);
    command_request.set_step_reference_channel("VehiclePosePb");
    player_service.Call("SetCommand", command_request);
    std::cout << command_request.DebugString() << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    command_request.Clear();

    // "SetCommand - de_initialize"
    std::cout << "eCAL::pb::play::EcalPlayService:SetCommand() - de_initialize" << std::endl;
    command_request.set_command(eCAL::pb::play::CommandRequest::de_initialize);
    player_service.Call("SetCommand", command_request);
    std::cout << command_request.DebugString() << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    command_request.Clear();
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
