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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100 4505 4800)
#endif
#include <ecal/msg/protobuf/client.h>
#include <ecal/pb/rec/client_service.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <chrono>
#include <thread>
#include <iostream>

// callback for recorder service response
void OnRecorderResponse(const struct eCAL::SServiceInfo& service_info_, const std::string& response_)
{
  switch (service_info_.call_state)
  {
  // service successful executed
  case call_state_executed:
  {
    if (service_info_.method_name == "GetConfig")
    {
      eCAL::pb::rec_client::GetConfigResponse response;
      response.ParseFromString(response_);
      std::cout << "RecorderService " << service_info_.method_name << " called successfully on host " << service_info_.host_name << std::endl;
      std::cout << response.DebugString();
    }
    else
    {
      eCAL::pb::rec_client::Response response;
      response.ParseFromString(response_);
      std::cout << "RecorderService " << service_info_.method_name << " called successfully on host " << service_info_.host_name << std::endl;
      std::cout << response.DebugString();
    }
  }
  break;
  // service execution failed
  case call_state_failed:
  {
    if (service_info_.method_name == "GetConfig")
    {
      eCAL::pb::rec_client::GetConfigResponse response;
      response.ParseFromString(response_);
      std::cout << "RecorderService " << service_info_.method_name << " failed with \"" << response.error() << "\" on host " << service_info_.host_name << std::endl;
      std::cout << response.DebugString();
    }

    if (service_info_.method_name == "SetConfig")
    {
      eCAL::pb::rec_client::Response response;
      response.ParseFromString(response_);
      std::cout << "RecorderService " << service_info_.method_name << " failed with \"" << response.error() << "\" on host " << service_info_.host_name << std::endl;
      std::cout << response.DebugString();
    }

    if (service_info_.method_name == "SetCommand")
    {
      eCAL::pb::rec_client::Response response;
      response.ParseFromString(response_);
      std::cout << "RecorderService " << service_info_.method_name << " failed with \"" << response.error() << "\" on host " << service_info_.host_name << std::endl;
      std::cout << response.DebugString();
    }
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
  eCAL::Initialize(argc, argv, "RecClientServiceCli");

  // create recorder service client
  eCAL::protobuf::CServiceClient<eCAL::pb::rec_client::EcalRecClientService> recorder_service;
  recorder_service.AddResponseCallback(OnRecorderResponse);

  // sleep for service matching
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  // "GetConfig"
  eCAL::pb::rec_client::GetConfigRequest get_config_request;
  std::cout << "eCAL.pb.rec.EcalRecService:GetConfig()" << std::endl;
  recorder_service.Call("GetConfig", get_config_request);
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

  recorder_service.Call("SetConfig", set_config_request);
  std::cout << set_config_request.DebugString() << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  // state request
  eCAL::pb::rec_client::CommandRequest state_request;

  (*state_request.mutable_command_params()->mutable_items())["meas_id"]           = "1234";                                                                // An ID to identify the measurement later on
  (*state_request.mutable_command_params()->mutable_items())["meas_root_dir"]     = "$TARGET{OSSELECT WIN \"C:\" LINUX \"$TARGET{ENV HOME}\"}/ecal_meas";  // The root directory to save the measurement to (un-evaluated format)
  (*state_request.mutable_command_params()->mutable_items())["meas_name"]         = "meas_${TIME}";                                                        // The name of the measurement (un-evaluated format)
  (*state_request.mutable_command_params()->mutable_items())["description"]       = "This is my description :)";                                           // The description that will be saved to the measurement's doc folder (un-evaluated format).
  (*state_request.mutable_command_params()->mutable_items())["max_file_size_mib"] = "775";                                                                 // The maximum HDF5 file size (When exceeding the file size, the measurement will be splitted into multiple files).

  // "initialize"
  std::cout << "eCAL.pb.rec.EcalRecService:SetCommand()" << std::endl;
  state_request.set_command(eCAL::pb::rec_client::CommandRequest::initialize);
  recorder_service.Call("SetCommand", state_request);
  std::cout << state_request.DebugString() << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(5000));

  // "de_initialize"
  std::cout << "eCAL.pb.rec.EcalRecService:SetCommand()" << std::endl;
  state_request.set_command(eCAL::pb::rec_client::CommandRequest::de_initialize);
  recorder_service.Call("SetCommand", state_request);
  std::cout << state_request.DebugString() << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(5000));

  // "start_recording"
  std::cout << "eCAL.pb.rec.EcalRecService:SetCommand()" << std::endl;
  state_request.set_command(eCAL::pb::rec_client::CommandRequest::start_recording);
  recorder_service.Call("SetCommand", state_request);
  std::cout << state_request.DebugString() << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(5000));

  // "stop_recording"
  std::cout << "eCAL.pb.rec.EcalRecService:SetCommand()" << std::endl;
  state_request.set_command(eCAL::pb::rec_client::CommandRequest::stop_recording);
  recorder_service.Call("SetCommand", state_request);
  std::cout << state_request.DebugString() << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(5000));

  // "save_pre_buffer"
  std::cout << "eCAL.pb.rec.EcalRecService:SetCommand()" << std::endl;
  state_request.set_command(eCAL::pb::rec_client::CommandRequest::save_pre_buffer);
  recorder_service.Call("SetCommand", state_request);
  std::cout << state_request.DebugString() << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(5000));

  // "exit"
  std::cout << "eCAL.pb.rec.EcalRecService:SetCommand()" << std::endl;
  state_request.set_command(eCAL::pb::rec_client::CommandRequest::exit);
  recorder_service.Call("SetCommand", state_request);
  std::cout << state_request.DebugString() << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
