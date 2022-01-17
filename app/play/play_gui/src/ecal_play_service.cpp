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

#include "q_ecal_play.h"
#include <clocale>
#include <locale>

#include <QMetaObject>

EcalPlayService::EcalPlayService()
  : eCAL::pb::play::EcalPlayService()
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

  (*config_item_map)["measurement_path"]              = QEcalPlay::instance()->measurementPath().toStdString();
  (*config_item_map)["limit_play_speed"]              = (QEcalPlay::instance()->isLimitPlaySpeedEnabled()       ? "true" : "false");
  (*config_item_map)["play_speed"]                    = std::to_string(QEcalPlay::instance()->playSpeed());
  (*config_item_map)["frame_dropping_allowed"]        = (QEcalPlay::instance()->isFrameDroppingAllowed()        ? "true" : "false");
  (*config_item_map)["enforce_delay_accuracy"]        = (QEcalPlay::instance()->isEnforceDelayAccuracyEnabled() ? "true" : "false");
  (*config_item_map)["repeat"]                        = (QEcalPlay::instance()->isRepeatEnabled()               ? "true" : "false");

  auto measurement_boundaries = QEcalPlay::instance()->measurementBoundaries();
  auto limit_interval_indices = QEcalPlay::instance()->limitInterval();

  auto limit_interval_start = std::chrono::duration_cast<std::chrono::duration<double>>(QEcalPlay::instance()->timestampOf(limit_interval_indices.first)  - measurement_boundaries.first);
  auto limit_interval_end   = std::chrono::duration_cast<std::chrono::duration<double>>(QEcalPlay::instance()->timestampOf(limit_interval_indices.second) - measurement_boundaries.first);

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

  // Note: All methods have to be executed in the main thread, as they may create widgets, which is only possible in the main thread.

  //////////////////////////////////////
  // measurement path                 //
  //////////////////////////////////////
  if (config_item_map.find("measurement_path") != config_item_map.end())
  {
    std::string measurement_path = config_item_map["measurement_path"];

    if (measurement_path.empty())
    {
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
      QMetaObject::invokeMethod(QEcalPlay::instance(), "closeMeasurement", Qt::BlockingQueuedConnection, Q_ARG(bool, true));
#else
      QMetaObject::invokeMethod(QEcalPlay::instance(), []() {return QEcalPlay::instance()->closeMeasurement(true); }, Qt::BlockingQueuedConnection);
#endif
    }
    else
    {
      bool success = false;
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
      QMetaObject::invokeMethod(QEcalPlay::instance(), "loadMeasurement", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, success), Q_ARG(QString, QString(measurement_path.c_str())), Q_ARG(bool, true));
#else
      QMetaObject::invokeMethod(QEcalPlay::instance(), [measurement_path]() {return QEcalPlay::instance()->loadMeasurement(measurement_path.c_str(), true); }, Qt::BlockingQueuedConnection, &success);
#endif
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
    auto limit_interval_indexes = QEcalPlay::instance()->limitInterval();
    auto limit_interval_times   = std::make_pair(QEcalPlay::instance()->timestampOf(limit_interval_indexes.first), QEcalPlay::instance()->timestampOf(limit_interval_indexes.second));
    auto measurement_boundaries = QEcalPlay::instance()->measurementBoundaries();

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

    bool success = false;
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QGenericArgument times_qarg("std::pair<eCAL::Time::ecal_clock::time_point,eCAL::Time::ecal_clock::time_point>", static_cast<const void*>(&limit_interval_times)); // The Q_ARG macro aparently does not work with the templated std::map
    QMetaObject::invokeMethod(QEcalPlay::instance(), "setLimitInterval", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, success), times_qarg);
#else
    QMetaObject::invokeMethod(QEcalPlay::instance(), [limit_interval_times]() {return QEcalPlay::instance()->setLimitInterval(limit_interval_times); }, Qt::BlockingQueuedConnection, &success);
#endif
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
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QMetaObject::invokeMethod(QEcalPlay::instance(), "setLimitPlaySpeedEnabled", Qt::BlockingQueuedConnection, Q_ARG(bool, limit_play_speed_enabled));
#else
    QMetaObject::invokeMethod(QEcalPlay::instance(), [limit_play_speed_enabled]() {return QEcalPlay::instance()->setLimitPlaySpeedEnabled(limit_play_speed_enabled); }, Qt::BlockingQueuedConnection);
#endif
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

#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QMetaObject::invokeMethod(QEcalPlay::instance(), "setPlaySpeed", Qt::BlockingQueuedConnection, Q_ARG(double, play_speed));
#else
    QMetaObject::invokeMethod(QEcalPlay::instance(), [play_speed]() {return QEcalPlay::instance()->setPlaySpeed(play_speed); }, Qt::BlockingQueuedConnection);
#endif

  }

  //////////////////////////////////////
  // frame_dropping_allowed           //
  //////////////////////////////////////
  if (config_item_map.find("frame_dropping_allowed") != config_item_map.end())
  {
    bool frame_dropping_allowed = strToBool(config_item_map["frame_dropping_allowed"]);
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QMetaObject::invokeMethod(QEcalPlay::instance(), "setFrameDroppingAllowed", Qt::BlockingQueuedConnection, Q_ARG(bool, frame_dropping_allowed));
#else
    QMetaObject::invokeMethod(QEcalPlay::instance(), [frame_dropping_allowed]() {return QEcalPlay::instance()->setFrameDroppingAllowed(frame_dropping_allowed); }, Qt::BlockingQueuedConnection);
#endif
  }

  //////////////////////////////////////
  // enforce_delay_accuracy           //
  //////////////////////////////////////
  if (config_item_map.find("enforce_delay_accuracy") != config_item_map.end())
  {
    bool enforce_delay_accuracy = strToBool(config_item_map["enforce_delay_accuracy"]);
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QMetaObject::invokeMethod(QEcalPlay::instance(), "setEnforceDelayAccuracyEnabled", Qt::BlockingQueuedConnection, Q_ARG(bool, enforce_delay_accuracy));
#else
    QMetaObject::invokeMethod(QEcalPlay::instance(), [enforce_delay_accuracy]() {return QEcalPlay::instance()->setEnforceDelayAccuracyEnabled(enforce_delay_accuracy); }, Qt::BlockingQueuedConnection);
#endif
  }

  //////////////////////////////////////
  // repeat                           //
  //////////////////////////////////////
  if (config_item_map.find("repeat") != config_item_map.end())
  {
    bool repeat = strToBool(config_item_map["repeat"]);
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QMetaObject::invokeMethod(QEcalPlay::instance(), "setRepeatEnabled", Qt::BlockingQueuedConnection, Q_ARG(bool, repeat));
#else
    QMetaObject::invokeMethod(QEcalPlay::instance(), [repeat]() {return QEcalPlay::instance()->setRepeatEnabled(repeat); }, Qt::BlockingQueuedConnection);
#endif
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
      for (const auto& source_channel : QEcalPlay::instance()->channelNames())
      {
        channel_mapping[source_channel] = source_channel;
      }
    }
    else
    {
      auto available_channels = QEcalPlay::instance()->channelNames();
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

    bool success = false;

#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    // Set the channel mapping
    QGenericArgument mapping_qarg("std::map<std::string,std::string>", static_cast<const void*>(&channel_mapping)); // The Q_ARG macro aparently does not work with the templated std::map
    QMetaObject::invokeMethod(QEcalPlay::instance(), "setChannelMapping", Qt::BlockingQueuedConnection, mapping_qarg);
    // initialize eCAL publishers
    QMetaObject::invokeMethod(QEcalPlay::instance(), "initializePublishers", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, success), Q_ARG(bool, true));
#else
    // Set the channel mapping
    QMetaObject::invokeMethod(QEcalPlay::instance(), [channel_mapping]() {return QEcalPlay::instance()->setChannelMapping(channel_mapping); }, Qt::BlockingQueuedConnection);
    // initialize eCAL publishers
    QMetaObject::invokeMethod(QEcalPlay::instance(), [channel_mapping]() {return QEcalPlay::instance()->initializePublishers(true); }, Qt::BlockingQueuedConnection, &success);
#endif

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
    bool success = false;
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QMetaObject::invokeMethod(QEcalPlay::instance(), "deInitializePublishers", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, success));
#else
    QMetaObject::invokeMethod(QEcalPlay::instance(), []() {return QEcalPlay::instance()->deInitializePublishers(); }, Qt::BlockingQueuedConnection, &success);
#endif
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
    auto measurement_boundaries = QEcalPlay::instance()->measurementBoundaries();
    auto target_time = measurement_boundaries.first + rel_time;

    bool success;
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QMetaObject::invokeMethod(QEcalPlay::instance(), "jumpTo", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, success), Q_ARG(eCAL::Time::ecal_clock::time_point, target_time));
#else
    QMetaObject::invokeMethod(QEcalPlay::instance(), [target_time]() {return QEcalPlay::instance()->jumpTo(target_time); }, Qt::BlockingQueuedConnection, &success);
#endif
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
    bool success;
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QMetaObject::invokeMethod(QEcalPlay::instance(), "play", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, success), Q_ARG(long long, -1), Q_ARG(bool, true));
#else
    QMetaObject::invokeMethod(QEcalPlay::instance(), []() {return QEcalPlay::instance()->play(-1, true); }, Qt::BlockingQueuedConnection, &success);
#endif
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
    bool success;
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QMetaObject::invokeMethod(QEcalPlay::instance(), "pause", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, success));
#else
    QMetaObject::invokeMethod(QEcalPlay::instance(), []() {return QEcalPlay::instance()->pause(); }, Qt::BlockingQueuedConnection, &success);
#endif
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
    bool success;
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QMetaObject::invokeMethod(QEcalPlay::instance(), "stepForward", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, success), Q_ARG(bool, true));
#else
    QMetaObject::invokeMethod(QEcalPlay::instance(), []() {return QEcalPlay::instance()->stepForward(true); }, Qt::BlockingQueuedConnection, &success);
#endif
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

    auto channel_list = QEcalPlay::instance()->channelNames();
    if (channel_list.find(step_reference_channel) == channel_list.end())
    {
      response->set_result(eCAL::pb::play::eServiceResult::failed);
      response->set_error("Step reference channel \"" + step_reference_channel + "\" is no valid channel");
      return;
    }
    
    bool success;
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QMetaObject::invokeMethod(QEcalPlay::instance(), "setStepReferenceChannel", Qt::BlockingQueuedConnection, Q_ARG(QString, QString(step_reference_channel.c_str())));
    QMetaObject::invokeMethod(QEcalPlay::instance(), "stepChannel", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, success), Q_ARG(bool, true));
#else
    QMetaObject::invokeMethod(QEcalPlay::instance(),
        [step_reference_channel]()
        {
          QEcalPlay::instance()->setStepReferenceChannel(step_reference_channel.c_str());
          return QEcalPlay::instance()->stepChannel(true);
        }
        , Qt::BlockingQueuedConnection, &success);
#endif
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
    response->set_result(eCAL::pb::play::eServiceResult::success);
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QMetaObject::invokeMethod(QEcalPlay::instance(), "exit", Qt::BlockingQueuedConnection, Q_ARG(bool, true));
#else
    QMetaObject::invokeMethod(QEcalPlay::instance(), []() {return QEcalPlay::instance()->exit(true); }, Qt::BlockingQueuedConnection);
#endif
  }
  else
  {
    response->set_result(eCAL::pb::play::eServiceResult::failed);
    response->set_error("Unknown command: " + std::to_string(command));
  }
}

void EcalPlayService::GetState(google::protobuf::RpcController* /*controller*/, const eCAL::pb::play::Empty*  /*request*/, eCAL::pb::play::State *response, google::protobuf::Closure*  /* done*/)
{
    EcalPlayState state = QEcalPlay::instance()->currentPlayState();

    eCAL::pb::play::State& state_pb = *response;

    state_pb.set_host_name(eCAL::Process::GetHostName());
    state_pb.set_process_id(eCAL::Process::GetProcessID());
    state_pb.set_playing(state.playing_);
    state_pb.set_measurement_loaded(QEcalPlay::instance()->isMeasurementLoaded());
    state_pb.set_actual_speed(state.actual_play_rate_);
    state_pb.set_current_measurement_index(state.current_frame_index);
    state_pb.set_current_measurement_timestamp_nsecs(std::chrono::duration_cast<std::chrono::nanoseconds>(state.current_frame_timestamp.time_since_epoch()).count());

    if (state_pb.measurement_loaded())
    {
        auto meas_info = state_pb.mutable_measurement_info();
        meas_info->set_path                 (QEcalPlay::instance()->measurementPath().toStdString());
        meas_info->set_frame_count          (QEcalPlay::instance()->frameCount());

        auto meas_boundaries = QEcalPlay::instance()->measurementBoundaries();
        meas_info->set_first_timestamp_nsecs(std::chrono::duration_cast<std::chrono::nanoseconds>(meas_boundaries.first.time_since_epoch()).count());
        meas_info->set_last_timestamp_nsecs (std::chrono::duration_cast<std::chrono::nanoseconds>(meas_boundaries.second.time_since_epoch()).count());
    }

    auto settings = state_pb.mutable_settings();
    settings->set_play_speed                    (QEcalPlay::instance()->playSpeed());
    settings->set_limit_play_speed              (QEcalPlay::instance()->isLimitPlaySpeedEnabled());
    settings->set_repeat_enabled                (QEcalPlay::instance()->isRepeatEnabled());
    settings->set_framedropping_allowed         (QEcalPlay::instance()->isFrameDroppingAllowed());
    settings->set_enforce_delay_accuracy_enabled(QEcalPlay::instance()->isEnforceDelayAccuracyEnabled());
    auto limit_interval = QEcalPlay::instance()->limitInterval();
    settings->set_limit_interval_lower_index    (limit_interval.first);
    settings->set_limit_interval_upper_index    (limit_interval.second);
}

bool EcalPlayService::strToBool(const std::string& str)
{
  if (str == "1")
    return true;

  std::string lower_string = str;

#if 0
  std::transform(lower_string.begin(), lower_string.end(), lower_string.begin(), ::tolower);
#else
  struct convert
  {
    unsigned char operator()(unsigned char const &c) {return ::tolower(c);}
  };
  std::transform(lower_string.begin(), lower_string.end(), lower_string.begin(), convert());
#endif

  if (lower_string == "true")
    return true;
  else
    return false;
}
