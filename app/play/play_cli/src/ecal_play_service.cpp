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

#include "ecal_play_service.h"

#include "ecal_play.h"

#include <algorithm>
#include <clocale>
#include <locale>

EcalPlayService::EcalPlayService(std::shared_ptr<EcalPlay> ecal_player)
  : eCAL::pb::play::EcalPlayService()
  , ecal_player_(ecal_player)
  , exit_request_(false)
{}

EcalPlayService::~EcalPlayService()
{}

void EcalPlayService::GetConfig(::google::protobuf::RpcController*        /*controller*/
                              , const ::eCAL::pb::play::GetConfigRequest* /*request*/
                              , ::eCAL::pb::play::GetConfigResponse*      response
                              , ::google::protobuf::Closure*              /*done*/)
{
  eCAL::pb::play::Configuration config;
  auto config_item_map = config.mutable_items();

  (*config_item_map)["measurement_path"]              = ecal_player_->GetMeasurementPath();
  (*config_item_map)["limit_play_speed"]              = (ecal_player_->IsLimitPlaySpeedEnabled()       ? "true" : "false");
  (*config_item_map)["play_speed"]                    = std::to_string(ecal_player_->GetPlaySpeed());
  (*config_item_map)["frame_dropping_allowed"]        = (ecal_player_->IsFrameDroppingAllowed()        ? "true" : "false");
  (*config_item_map)["enforce_delay_accuracy"]        = (ecal_player_->IsEnforceDelayAccuracyEnabled() ? "true" : "false");
  (*config_item_map)["repeat"]                        = (ecal_player_->IsRepeatEnabled()               ? "true" : "false");

  auto measurement_boundaries = ecal_player_->GetMeasurementBoundaries();
  auto limit_interval_indices = ecal_player_->GetLimitInterval();

  auto limit_interval_start = std::chrono::duration_cast<std::chrono::duration<double>>(ecal_player_->GetTimestampOf(limit_interval_indices.first)  - measurement_boundaries.first);
  auto limit_interval_end   = std::chrono::duration_cast<std::chrono::duration<double>>(ecal_player_->GetTimestampOf(limit_interval_indices.second) - measurement_boundaries.first);

  (*config_item_map)["limit_interval_start_rel_secs"] = std::to_string(limit_interval_start.count());
  (*config_item_map)["limit_interval_end_rel_secs"]   = std::to_string(limit_interval_end.count());

  response->mutable_config()->CopyFrom(config);
  response->set_result(eCAL::pb::play::eServiceResult::success);
}

void EcalPlayService::SetConfig(::google::protobuf::RpcController*        /*controller*/
                              , const ::eCAL::pb::play::SetConfigRequest* request
                              , ::eCAL::pb::play::Response*               response
                              , ::google::protobuf::Closure*              /*done*/)
{
  auto config_item_map = request->config().items();

  char decimal_point = std::localeconv()->decimal_point[0]; // decimal point for std::stod()

  //////////////////////////////////////
  // measurement path                 //
  //////////////////////////////////////
  if (config_item_map.find("measurement_path") != config_item_map.end())
  {
    std::string measurement_path = config_item_map["measurement_path"];

    if (measurement_path.empty())
    {
      ecal_player_->CloseMeasurement();
    }
    else
    {
      bool success = ecal_player_->LoadMeasurement(measurement_path);
      if (!success)
      {
        response->set_result(eCAL::pb::play::eServiceResult::failed);
        response->set_error("Unable to load measurement from " + measurement_path);
        return;
      }
    }
  }

  //////////////////////////////////////
  // limit interval                   //
  //////////////////////////////////////
  bool set_limit_interval = ((config_item_map.find("limit_interval_start_rel_secs") != config_item_map.end())
                          || (config_item_map.find("limit_interval_end_rel_secs")   != config_item_map.end()));
  if (set_limit_interval)
  {
    auto limit_interval_indexes = ecal_player_->GetLimitInterval();
    auto limit_interval_times   = std::make_pair(ecal_player_->GetTimestampOf(limit_interval_indexes.first), ecal_player_->GetTimestampOf(limit_interval_indexes.second));
    auto measurement_boundaries = ecal_player_->GetMeasurementBoundaries();

    if (config_item_map.find("limit_interval_start_rel_secs") != config_item_map.end())
    {
      std::string start_rel_seconds_string = config_item_map["limit_interval_start_rel_secs"];
      std::replace(start_rel_seconds_string.begin(), start_rel_seconds_string.end(), '.', decimal_point);
      double start_rel_seconds_double = 0.0;
      try
      {
        start_rel_seconds_double = std::stod(start_rel_seconds_string);
      }
      catch (const std::exception& e)
      {
        response->set_result(eCAL::pb::play::eServiceResult::failed);
        response->set_error("Error parsing value \"" + start_rel_seconds_string + "\": " + e.what());
        return;
      }
      auto start_rel = std::chrono::duration_cast<eCAL::Time::ecal_clock::duration>(std::chrono::duration<double>(start_rel_seconds_double));
      limit_interval_times.first = measurement_boundaries.first + start_rel;
    }

    if (config_item_map.find("limit_interval_end_rel_secs") != config_item_map.end())
    {
      std::string end_rel_seconds_string = config_item_map["limit_interval_end_rel_secs"];
      std::replace(end_rel_seconds_string.begin(), end_rel_seconds_string.end(), '.', decimal_point);
      double end_rel_seconds_double = 0.0;
      try
      {
        end_rel_seconds_double = std::stod(end_rel_seconds_string);
      }
      catch (const std::exception& e)
      {
        response->set_result(eCAL::pb::play::eServiceResult::failed);
        response->set_error("Error parsing value \"" + end_rel_seconds_string + "\": " + e.what());
        return;
      }
      auto end_rel = std::chrono::duration_cast<eCAL::Time::ecal_clock::duration>(std::chrono::duration<double>(end_rel_seconds_double));
      limit_interval_times.second = measurement_boundaries.first + end_rel;
    }

    bool success = ecal_player_->SetLimitInterval(limit_interval_times);
    if (!success)
    {
      response->set_result(eCAL::pb::play::eServiceResult::failed);
      response->set_error("Unable to set limit interval.");
      return;
    }
  }

  //////////////////////////////////////
  // limit_play_speed                 //
  //////////////////////////////////////
  if (config_item_map.find("limit_play_speed") != config_item_map.end())
  {
    bool limit_play_speed_enabled = strToBool(config_item_map["limit_play_speed"]);
    ecal_player_->SetLimitPlaySpeedEnabled(limit_play_speed_enabled);
  }

  //////////////////////////////////////
  // play_speed                       //
  //////////////////////////////////////
  if (config_item_map.find("play_speed") != config_item_map.end())
  {
    std::string play_speed_string = config_item_map["play_speed"];
    std::replace(play_speed_string.begin(), play_speed_string.end(), '.', decimal_point);
    double play_speed = 1.0;
    try
    {
      play_speed = std::stod(play_speed_string);
    }
    catch (const std::exception& e)
    {
      response->set_result(eCAL::pb::play::eServiceResult::failed);
      response->set_error("Error parsing value \"" + play_speed_string + "\": " + e.what());
      return;
    }
    play_speed = std::max(0.0, play_speed);
    ecal_player_->SetPlaySpeed(play_speed);
  }

  //////////////////////////////////////
  // frame_dropping_allowed           //
  //////////////////////////////////////
  if (config_item_map.find("frame_dropping_allowed") != config_item_map.end())
  {
    bool frame_dropping_allowed = strToBool(config_item_map["frame_dropping_allowed"]);
    ecal_player_->SetFrameDroppingAllowed(frame_dropping_allowed);
  }

  //////////////////////////////////////
  // enforce_delay_accuracy           //
  //////////////////////////////////////
  if (config_item_map.find("enforce_delay_accuracy") != config_item_map.end())
  {
    bool enforce_delay_accuracy = strToBool(config_item_map["enforce_delay_accuracy"]);
    ecal_player_->SetEnforceDelayAccuracyEnabled(enforce_delay_accuracy);
  }

  //////////////////////////////////////
  // repeat                           //
  //////////////////////////////////////
  if (config_item_map.find("repeat") != config_item_map.end())
  {
    bool repeat = strToBool(config_item_map["repeat"]);
    ecal_player_->SetRepeatEnabled(repeat);
  }

  // Return success
  response->set_result(eCAL::pb::play::eServiceResult::success);
}

void EcalPlayService::SetCommand(::google::protobuf::RpcController*       /*controller*/
                              , const ::eCAL::pb::play::CommandRequest*   request
                              , ::eCAL::pb::play::Response*               response
                              , ::google::protobuf::Closure*              /*done*/)
{
  auto command = request->command();

  if (command == eCAL::pb::play::CommandRequest::initialize)
  {
    auto channel_mapping_pb = request->channel_mapping();
    std::map<std::string, std::string> channel_mapping;

    // Transform protobuf map to std::map
    if (channel_mapping_pb.empty())
    {
      for (const auto& source_channel : ecal_player_->GetChannelNames())
      {
        channel_mapping[source_channel] = source_channel;
      }
    }
    else
    {
      auto available_channels = ecal_player_->GetChannelNames();
      for (const auto& channel : channel_mapping_pb)
      {
        // Check if this is even a valid channel
        if (available_channels.find(channel.first) != available_channels.end())
        {
          channel_mapping[channel.first] = channel.second;
        }
      }

      // If the channel mapping is empty at this point, the user did not include any valid channel
      if (channel_mapping.empty())
      {
        response->set_result(eCAL::pb::play::eServiceResult::failed);
        response->set_error("Channel mapping does not contain any valid channel.");
        return;
      }
    }

    bool success = ecal_player_->InitializePublishers(channel_mapping);

    if (success)
    {
      response->set_result(eCAL::pb::play::eServiceResult::success);
    }
    else
    {
      response->set_result(eCAL::pb::play::eServiceResult::failed);
      response->set_error("Unable to initialize eCAL Publishers");
    }
  }
  else if (command == eCAL::pb::play::CommandRequest::de_initialize)
  {
    bool success = ecal_player_->DeInitializePublishers();
    if (success)
    {
      response->set_result(eCAL::pb::play::eServiceResult::success);
    }
    else
    {
      response->set_result(eCAL::pb::play::eServiceResult::failed);
      response->set_error("Unable to de-initialize eCAL publishers");
    }
  }
  else if (command == eCAL::pb::play::CommandRequest::jump_to)
  {
    auto rel_time = std::chrono::duration_cast<eCAL::Time::ecal_clock::duration>(std::chrono::duration<double>(request->rel_time_secs()));
    auto measurement_boundaries = ecal_player_->GetMeasurementBoundaries();
    auto target_time = measurement_boundaries.first + rel_time;

    bool success = ecal_player_->JumpTo(target_time);
    if (success)
    {
      response->set_result(eCAL::pb::play::eServiceResult::success);
    }
    else
    {
      response->set_result(eCAL::pb::play::eServiceResult::failed);
      response->set_error("Unable to jump to " + std::to_string(request->rel_time_secs()) + " s");
    }
  }
  else if (command == eCAL::pb::play::CommandRequest::play)
  {
    bool success = ecal_player_->Play();
    if (success)
    {
      response->set_result(eCAL::pb::play::eServiceResult::success);
    }
    else
    {
      response->set_result(eCAL::pb::play::eServiceResult::failed);
      response->set_error("Unable to start playback");
    }
  }
  else if (command == eCAL::pb::play::CommandRequest::pause)
  {
    bool success = ecal_player_->Pause();
    if (success)
    {
      response->set_result(eCAL::pb::play::eServiceResult::success);
    }
    else
    {
      response->set_result(eCAL::pb::play::eServiceResult::failed);
      response->set_error("Unable to pause playback");
    }
  }
  else if (command == eCAL::pb::play::CommandRequest::step)
  {
    bool success = ecal_player_->StepForward();
    if (success)
    {
      response->set_result(eCAL::pb::play::eServiceResult::success);
    }
    else
    {
      response->set_result(eCAL::pb::play::eServiceResult::failed);
      response->set_error("Unable to step frame");
    }
  }
  else if (command == eCAL::pb::play::CommandRequest::step_channel)
  {
    std::string step_reference_channel = request->step_reference_channel();
    if (step_reference_channel.empty())
    {
      response->set_result(eCAL::pb::play::eServiceResult::failed);
      response->set_error("Step reference channel is empty");
      return;
    }

    auto channel_list = ecal_player_->GetChannelNames();
    if (channel_list.find(step_reference_channel) == channel_list.end())
    {
      response->set_result(eCAL::pb::play::eServiceResult::failed);
      response->set_error("Step reference channel \"" + step_reference_channel + "\" is no valid channel");
      return;
    }
    
    bool success = ecal_player_->PlayToNextOccurenceOfChannel(step_reference_channel);
    if (success)
    {
      response->set_result(eCAL::pb::play::eServiceResult::success);
    }
    else
    {
      response->set_result(eCAL::pb::play::eServiceResult::failed);
      response->set_error("Unable to step channel");
    }
  }
  else if (command == eCAL::pb::play::CommandRequest::exit)
  {
    exit_request_ = true;
  }
  else
  {
    response->set_result(eCAL::pb::play::eServiceResult::failed);
    response->set_error("Unknown command: " + std::to_string(command));
  }
}

void EcalPlayService::GetState(::google::protobuf::RpcController* /*controller*/,
                      const ::eCAL::pb::play::Empty*              /*request*/,
                      ::eCAL::pb::play::State*                    response,
                      ::google::protobuf::Closure*                /*done*/)
{
    EcalPlayState state = ecal_player_->GetCurrentPlayState();

    eCAL::pb::play::State& state_pb = *response;

    state_pb.set_host_name(eCAL::Process::GetHostName());
    state_pb.set_process_id(eCAL::Process::GetProcessID());
    state_pb.set_playing(state.playing_);
    state_pb.set_measurement_loaded(ecal_player_->IsMeasurementLoaded());
    state_pb.set_actual_speed(state.actual_play_rate_);
    state_pb.set_current_measurement_index(state.current_frame_index);
    state_pb.set_current_measurement_timestamp_nsecs(std::chrono::duration_cast<std::chrono::nanoseconds>(state.current_frame_timestamp.time_since_epoch()).count());

    if (state_pb.measurement_loaded())
    {
        auto meas_info = state_pb.mutable_measurement_info();
        meas_info->set_path                 (ecal_player_->GetMeasurementPath());
        meas_info->set_frame_count          (ecal_player_->GetFrameCount());

        auto meas_boundaries = ecal_player_->GetMeasurementBoundaries();
        meas_info->set_first_timestamp_nsecs(std::chrono::duration_cast<std::chrono::nanoseconds>(meas_boundaries.first.time_since_epoch()).count());
        meas_info->set_last_timestamp_nsecs (std::chrono::duration_cast<std::chrono::nanoseconds>(meas_boundaries.second.time_since_epoch()).count());
    }

    auto settings = state_pb.mutable_settings();
    settings->set_play_speed                    (ecal_player_->GetPlaySpeed());
    settings->set_limit_play_speed              (ecal_player_->IsLimitPlaySpeedEnabled());
    settings->set_repeat_enabled                (ecal_player_->IsRepeatEnabled());
    settings->set_framedropping_allowed         (ecal_player_->IsFrameDroppingAllowed());
    settings->set_enforce_delay_accuracy_enabled(ecal_player_->IsEnforceDelayAccuracyEnabled());
    auto limit_interval = ecal_player_->GetLimitInterval();
    settings->set_limit_interval_lower_index    (limit_interval.first);
    settings->set_limit_interval_upper_index    (limit_interval.second);
}

bool EcalPlayService::IsExitRequested() const
{
  return exit_request_;
}

bool EcalPlayService::strToBool(const std::string& str)
{
  if (str == "1")
    return true;

  std::string lower_string = str;
  std::transform(lower_string.begin(), lower_string.end(), lower_string.begin(), [](char c) {return static_cast<char>(::tolower(c)); });

  if (lower_string == "true")
    return true;
  else
    return false;
}
