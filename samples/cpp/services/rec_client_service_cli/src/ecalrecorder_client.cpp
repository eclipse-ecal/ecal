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

#include <ecal/ecal.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100 4505 4800)
#endif
#include <ecal/msg/protobuf/client.h>
#include <ecal/app/pb/rec/client_service.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <chrono>
#include <thread>
#include <iostream>

// callback for recorder service response
void OnRecorderResponse(const struct eCAL::SServiceResponse& service_response_)
{
  const std::string& method_name = service_response_.service_method_information.method_name;
  const std::string& host_name   = service_response_.server_id.service_id.host_name;
  
  switch (service_response_.call_state)
  {
  // service successful executed
  case eCAL::eCallState::executed:
  {
    if (method_name == "GetConfig")
    {
      eCAL::pb::rec_client::GetConfigResponse response;
      response.ParseFromString(service_response_.response);
      std::cout << "RecorderService " << method_name << " called successfully on host " << host_name << std::endl;
      std::cout << response.DebugString();
    }
    else
    {
      eCAL::pb::rec_client::Response response;
      response.ParseFromString(service_response_.response);
      std::cout << "RecorderService " << method_name << " called successfully on host " << host_name << std::endl;
      std::cout << response.DebugString();
    }
  }
  break;
  // service execution failed
  case eCAL::eCallState::failed:
  {
    if (method_name == "GetConfig")
    {
      eCAL::pb::rec_client::GetConfigResponse response;
      response.ParseFromString(service_response_.response);
      std::cout << "RecorderService " << method_name << " failed with \"" << response.error() << "\" on host " << host_name << std::endl;
      std::cout << response.DebugString();
    }

    if (method_name == "SetConfig")
    {
      eCAL::pb::rec_client::Response response;
      response.ParseFromString(service_response_.response);
      std::cout << "RecorderService " << method_name << " failed with \"" << response.error() << "\" on host " << host_name << std::endl;
      std::cout << response.DebugString();
    }

    if (method_name == "SetCommand")
    {
      eCAL::pb::rec_client::Response response;
      response.ParseFromString(service_response_.response);
      std::cout << "RecorderService " << method_name << " failed with \"" << response.error() << "\" on host " << host_name << std::endl;
      std::cout << response.DebugString();
    }
  }
  break;
  default:
    break;
  }
}

// main entry
int main()
{
  // initialize eCAL API
  eCAL::Initialize("RecClientServiceCli");

  // create recorder service client
  eCAL::protobuf::CServiceClient<eCAL::pb::rec_client::EcalRecClientService> recorder_service;

  // waiting for service
  while (!recorder_service.IsConnected())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Waiting for the service .." << std::endl;
  }

  // "GetConfig"
  eCAL::pb::rec_client::GetConfigRequest get_config_request;
  std::cout << "eCAL.pb.rec.EcalRecService:GetConfig()" << std::endl;
  recorder_service.CallWithCallback("GetConfig", get_config_request, OnRecorderResponse);
  std::cout << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  // "SetConfig"
  eCAL::pb::rec_client::SetConfigRequest set_config_request;
  std::cout << "eCAL.pb.rec.EcalRecService:SetConfig()" << std::endl;
  auto config = set_config_request.mutable_config()->mutable_items();

  (*config)["max_pre_buffer_length_secs"] = "42";                             // The maximum amount of time to keep in the pre-buffer
  (*config)["pre_buffering_enabled"]      = "true";                           // Whether pre-buffering is enabled
  (*config)["host_filter"]                = "";                               // List of hosts (\n separated). The recorder will only record channels published by these hosts. If empty, all hosts are allowed.
  (*config)["record_mode"]                = "blacklist";                      // Whether to record all topics or use a blacklist / whitelist to only record some topics. Changing the mode will clear the listed_topics, so it is advisable to also provide a new listed_topics list.
  (*config)["listed_topics"]              = "topic1\ntopic2\ntopic3";         // Whitelist / blacklist, when topic_mode is set accordingly (\n separated). If topic_mode is "all", this setting will be ignored.
  (*config)["enabled_addons"]             = "";                               // List of addon-IDs that shall be enabled (\n separated).

  recorder_service.CallWithCallback("SetConfig", set_config_request, OnRecorderResponse);
  std::cout << set_config_request.DebugString() << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  // state request
  eCAL::pb::rec_client::CommandRequest state_request;

  (*state_request.mutable_command_params()->mutable_items())["meas_id"]              = "1234";                                                                // An ID to identify the measurement later on
  (*state_request.mutable_command_params()->mutable_items())["meas_root_dir"]        = "$TARGET{OSSELECT WIN \"C:\" LINUX \"$TARGET{ENV HOME}\"}/ecal_meas";  // The root directory to save the measurement to (un-evaluated format)
  (*state_request.mutable_command_params()->mutable_items())["meas_name"]            = "meas_${TIME}";                                                        // The name of the measurement (un-evaluated format)
  (*state_request.mutable_command_params()->mutable_items())["description"]          = "This is my description :)";                                           // The description that will be saved to the measurement's doc folder (un-evaluated format).
  (*state_request.mutable_command_params()->mutable_items())["max_file_size_mib"]    = "775";                                                                 // The maximum HDF5 file size (When exceeding the file size, the measurement will be splitted into multiple files).
  (*state_request.mutable_command_params()->mutable_items())["one_file_per_topic"]   = "false";                                                               // Whether the recorder shall create 1 hdf5 file per channel


  // "initialize"
  std::cout << "eCAL.pb.rec.EcalRecService:SetCommand()" << std::endl;
  state_request.set_command(eCAL::pb::rec_client::CommandRequest::initialize);
  recorder_service.CallWithCallback("SetCommand", state_request, OnRecorderResponse);
  std::cout << state_request.DebugString() << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(5000));

  // "de_initialize"
  std::cout << "eCAL.pb.rec.EcalRecService:SetCommand()" << std::endl;
  state_request.set_command(eCAL::pb::rec_client::CommandRequest::de_initialize);
  recorder_service.CallWithCallback("SetCommand", state_request, OnRecorderResponse);
  std::cout << state_request.DebugString() << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(5000));

  // "start_recording"
  std::cout << "eCAL.pb.rec.EcalRecService:SetCommand()" << std::endl;
  state_request.set_command(eCAL::pb::rec_client::CommandRequest::start_recording);
  recorder_service.CallWithCallback("SetCommand", state_request, OnRecorderResponse);
  std::cout << state_request.DebugString() << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(5000));

  // "stop_recording"
  std::cout << "eCAL.pb.rec.EcalRecService:SetCommand()" << std::endl;
  state_request.set_command(eCAL::pb::rec_client::CommandRequest::stop_recording);
  recorder_service.CallWithCallback("SetCommand", state_request, OnRecorderResponse);
  std::cout << state_request.DebugString() << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(5000));

  // "save_pre_buffer"
  std::cout << "eCAL.pb.rec.EcalRecService:SetCommand()" << std::endl;
  state_request.set_command(eCAL::pb::rec_client::CommandRequest::save_pre_buffer);
  recorder_service.CallWithCallback("SetCommand", state_request, OnRecorderResponse);
  std::cout << state_request.DebugString() << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(5000));

  // "exit"
  std::cout << "eCAL.pb.rec.EcalRecService:SetCommand()" << std::endl;
  state_request.set_command(eCAL::pb::rec_client::CommandRequest::exit);
  recorder_service.CallWithCallback("SetCommand", state_request, OnRecorderResponse);
  std::cout << state_request.DebugString() << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
