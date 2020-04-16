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

#include "ecalrec_service.h"

#include "qecalrec.h"
#include <clocale>
#include <locale>

#include <ecal_utils/ecal_utils.h>

#include <QMetaObject>

EcalRecService::EcalRecService()
  : eCAL::pb::rec::EcalRecService()
{}

EcalRecService::~EcalRecService()
{}

void EcalRecService::GetConfig(::google::protobuf::RpcController*        /*controller*/
                              , const ::eCAL::pb::rec::GetConfigRequest* /*request*/
                              , ::eCAL::pb::rec::GetConfigResponse*      response
                              , ::google::protobuf::Closure*             /*done*/)
{
  eCAL::pb::rec::Configuration config;
  auto config_item_map = config.mutable_items();

  char decimal_point = std::localeconv()->decimal_point[0]; // Locale decimal point for making float strings locale independent

  (*config_item_map)["meas_root_dir"]          = QEcalRec::instance()->measurementRootDirectory();
  (*config_item_map)["meas_name"]              = QEcalRec::instance()->measurementName();
  std::string max_buffer_length_secs_string = std::to_string(std::chrono::duration_cast<std::chrono::duration<double>>(QEcalRec::instance()->maxBufferLength()).count());
  std::replace(max_buffer_length_secs_string.begin(), max_buffer_length_secs_string.end(), decimal_point, '.');
  (*config_item_map)["max_buffer_length_secs"] = max_buffer_length_secs_string;
  (*config_item_map)["buffering_enabled"]      = (QEcalRec::instance()->isBufferingEnabled() ? "true" : "false");
  (*config_item_map)["max_file_size_mib"]      = std::to_string(QEcalRec::instance()->maxFileSize());
  (*config_item_map)["host_filter"]            = EcalUtils::String::Join("\n", QEcalRec::instance()->hostsFilter());
  (*config_item_map)["topic_blacklist"]        = EcalUtils::String::Join("\n", QEcalRec::instance()->topicBlacklist());
  (*config_item_map)["topic_whitelist"]        = EcalUtils::String::Join("\n", QEcalRec::instance()->topicWhitelist());

  response->mutable_config()->CopyFrom(config);
  response->set_result(eCAL::pb::rec::eServiceResult::success);
}

void EcalRecService::SetConfig(::google::protobuf::RpcController*        /*controller*/
                              , const ::eCAL::pb::rec::SetConfigRequest* request
                              , ::eCAL::pb::rec::Response*               response
                              , ::google::protobuf::Closure*             /*done*/)
{
  auto config_item_map = request->config().items();

  char decimal_point = std::localeconv()->decimal_point[0]; // decimal point for std::stod()

  // Note: All methods have to be executed in the main thread, as they may create widgets, which is only possible in the main thread.

  //TODO: Revise the order, as some configuratoin items my influence each other

  //////////////////////////////////////
  // meas_root_dir                    //
  //////////////////////////////////////
  if (config_item_map.find("meas_root_dir") != config_item_map.end())
  {
    std::string meas_root_dir = config_item_map["meas_root_dir"];

#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QMetaObject::invokeMethod(QEcalRec::instance(), "setMeasurementRootDirectory", Qt::BlockingQueuedConnection, Q_ARG(QString, QString(meas_root_dir.c_str())));
#else
    QMetaObject::invokeMethod(QEcalRec::instance(), [meas_root_dir]() {return QEcalRec::instance()->setMeasurementRootDirectory(meas_root_dir.c_str()); }, Qt::BlockingQueuedConnection);
#endif
  }

  //////////////////////////////////////
  // meas_name                        //
  //////////////////////////////////////
  if (config_item_map.find("meas_name") != config_item_map.end())
  {
    std::string meas_name = config_item_map["meas_name"];

#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QMetaObject::invokeMethod(QEcalRec::instance(), "setMeasurementName", Qt::BlockingQueuedConnection, Q_ARG(QString, QString(meas_name.c_str())));
#else
    QMetaObject::invokeMethod(QEcalRec::instance(), [meas_name]() {return QEcalRec::instance()->setMeasurementName(meas_name.c_str()); }, Qt::BlockingQueuedConnection);
#endif
  }

  //////////////////////////////////////
  // max_buffer_length_secs           //
  //////////////////////////////////////
  if (config_item_map.find("max_buffer_length_secs") != config_item_map.end())
  {
    std::string max_buffer_length_secs_string = config_item_map["max_buffer_length_secs"];
    std::replace(max_buffer_length_secs_string.begin(), max_buffer_length_secs_string.end(), '.', decimal_point);
    double max_buffer_length_secs = 0.0;
    try
    {
      max_buffer_length_secs = std::stod(max_buffer_length_secs_string);
    }
    catch (const std::exception& e)
    {
      response->set_result(eCAL::pb::rec::eServiceResult::failed);
      response->set_error("Error parsing value \"" + max_buffer_length_secs_string + "\": " + e.what());
      return;
    }

    auto max_buffer_length = std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(max_buffer_length_secs));

#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QMetaObject::invokeMethod(QEcalRec::instance(), "setMaxBufferLength", Qt::BlockingQueuedConnection, Q_ARG(std::chrono::steady_clock::duration, max_buffer_length));
#else
    QMetaObject::invokeMethod(QEcalRec::instance(), [max_buffer_length]() {return QEcalRec::instance()->setMaxBufferLength(max_buffer_length); }, Qt::BlockingQueuedConnection);
#endif
  }

  //////////////////////////////////////
  // buffering_enabled                //
  //////////////////////////////////////
  if (config_item_map.find("buffering_enabled") != config_item_map.end())
  {
    std::string buffering_enabled_string = config_item_map["buffering_enabled"];
    bool buffering_enabled = strToBool(buffering_enabled_string);

#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QMetaObject::invokeMethod(QEcalRec::instance(), "setBufferingEnabled", Qt::BlockingQueuedConnection, Q_ARG(bool, buffering_enabled));
#else
    QMetaObject::invokeMethod(QEcalRec::instance(), [buffering_enabled]() {return QEcalRec::instance()->setBufferingEnabled(buffering_enabled); }, Qt::BlockingQueuedConnection);
#endif
  }

  //////////////////////////////////////
  // max_file_size_mib                //
  //////////////////////////////////////
  if (config_item_map.find("max_file_size_mib") != config_item_map.end())
  {
    std::string max_file_size_mib_string = config_item_map["max_file_size_mib"];
    unsigned long long max_file_size = 0;
    try
    {
      max_file_size = std::stoull(max_file_size_mib_string);
    }
    catch (const std::exception& e)
    {
      response->set_result(eCAL::pb::rec::eServiceResult::failed);
      response->set_error("Error parsing value \"" + max_file_size_mib_string + "\": " + e.what());
      return;
    }

    // Check the input value, so we can savely cast it later
    if (max_file_size > SIZE_MAX)
    {
      response->set_result(eCAL::pb::rec::eServiceResult::failed);
      response->set_error("Error setting max file size to " + max_file_size_mib_string + "MiB: Value too large");
      return;
    }

    size_t max_file_size_sizet = static_cast<size_t>(max_file_size);

#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QMetaObject::invokeMethod(QEcalRec::instance(), "setMaxFileSize", Qt::BlockingQueuedConnection, Q_ARG(size_t, max_file_size_sizet));
#else
    QMetaObject::invokeMethod(QEcalRec::instance(), [max_file_size_sizet]() {return QEcalRec::instance()->setMaxFileSize(max_file_size_sizet); }, Qt::BlockingQueuedConnection);
#endif
  }

  //////////////////////////////////////
  // host_filter                      //
  //////////////////////////////////////
  if (config_item_map.find("host_filter") != config_item_map.end())
  {
    std::string host_filter_string = config_item_map["host_filter"];

    std::set<std::string> host_filter;
    EcalUtils::String::Split(host_filter_string, "\n", host_filter);

    bool success = false;
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QGenericArgument host_filter_arg("std::set<std::string>", &host_filter); // The Q_ARG macro aparently does not work with the templated types
    QMetaObject::invokeMethod(QEcalRec::instance(), "setHostFilter", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, success), host_filter_arg, Q_ARG(bool, true));
#else
    QMetaObject::invokeMethod(QEcalRec::instance(), [&host_filter]() {return QEcalRec::instance()->setHostFilter(host_filter, true); }, Qt::BlockingQueuedConnection, &success);
#endif

    if (!success)
    {
      response->set_result(eCAL::pb::rec::eServiceResult::failed);
      response->set_error("Error setting host filter. Stop the recorder and try again.");
    }
  }

  //////////////////////////////////////
  // topic_blacklist                  //
  //////////////////////////////////////
  if (config_item_map.find("topic_blacklist") != config_item_map.end())
  {
    std::string topic_blacklist_string = config_item_map["topic_blacklist"];

    std::set<std::string> topic_blacklist;
    EcalUtils::String::Split(topic_blacklist_string, "\n", topic_blacklist);

    bool success = false;
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QGenericArgument topic_blacklist_arg("std::set<std::string>", &topic_blacklist); // The Q_ARG macro aparently does not work with the templated types
    QMetaObject::invokeMethod(QEcalRec::instance(), "setTopicBlacklist", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, success), topic_blacklist_arg, Q_ARG(bool, true));
#else
    QMetaObject::invokeMethod(QEcalRec::instance(), [&topic_blacklist]() {return QEcalRec::instance()->setTopicBlacklist(topic_blacklist, true); }, Qt::BlockingQueuedConnection, &success);
#endif

    if (!success)
    {
      response->set_result(eCAL::pb::rec::eServiceResult::failed);
      response->set_error("Error setting topic blacklist. Stop the recorder and try again.");
    }
  }

  //////////////////////////////////////
  // topic_whitelist                  //
  //////////////////////////////////////
  if (config_item_map.find("topic_whitelist") != config_item_map.end())
  {
    std::string topic_whitelist_string = config_item_map["topic_whitelist"];

    std::set<std::string> topic_whitelist;
    EcalUtils::String::Split(topic_whitelist_string, "\n", topic_whitelist);

    bool success = false;
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QGenericArgument topic_whitelist_arg("std::set<std::string>", &topic_whitelist); // The Q_ARG macro aparently does not work with the templated types
    QMetaObject::invokeMethod(QEcalRec::instance(), "setTopicWhitelist", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, success), topic_whitelist_arg, Q_ARG(bool, true));
#else
    QMetaObject::invokeMethod(QEcalRec::instance(), [&topic_whitelist]() {return QEcalRec::instance()->setTopicWhitelist(topic_whitelist, true); }, Qt::BlockingQueuedConnection, &success);
#endif

    if (!success)
    {
      response->set_result(eCAL::pb::rec::eServiceResult::failed);
      response->set_error("Error setting topic whitelist. Stop the recorder and try again.");
    }
  }
}

void EcalRecService::SetCommand(::google::protobuf::RpcController*       /*controller*/
                              , const ::eCAL::pb::rec::CommandRequest*   request
                              , ::eCAL::pb::rec::Response*               response
                              , ::google::protobuf::Closure*             /*done*/)
{
  auto command = request->command();

  //////////////////////////////////////
  // initialize                       //
  //////////////////////////////////////
  if (command == eCAL::pb::rec::CommandRequest::initialize)
  {
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QMetaObject::invokeMethod(QEcalRec::instance(), "connectToEcal", Qt::BlockingQueuedConnection);
#else
    QMetaObject::invokeMethod(QEcalRec::instance(), []() {return QEcalRec::instance()->connectToEcal(); }, Qt::BlockingQueuedConnection);
#endif
    response->set_result(eCAL::pb::rec::eServiceResult::success);
  }

  //////////////////////////////////////
  // de_initialize                    //
  //////////////////////////////////////
  else if (command == eCAL::pb::rec::CommandRequest::de_initialize)
  {
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QMetaObject::invokeMethod(QEcalRec::instance(), "disconnectFromEcal", Qt::BlockingQueuedConnection);
#else
    QMetaObject::invokeMethod(QEcalRec::instance(), []() {return QEcalRec::instance()->disconnectFromEcal(); }, Qt::BlockingQueuedConnection);
#endif
    response->set_result(eCAL::pb::rec::eServiceResult::success);
  }

  //////////////////////////////////////
  // start_recording                  //
  //////////////////////////////////////
  else if (command == eCAL::pb::rec::CommandRequest::start_recording)
  {
    bool success = false;
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QMetaObject::invokeMethod(QEcalRec::instance(), "startRecording", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, success), Q_ARG(bool, true));
#else
    QMetaObject::invokeMethod(QEcalRec::instance(), []() {return QEcalRec::instance()->startRecording(true); }, Qt::BlockingQueuedConnection, &success);
#endif

    if (success)
    {
      response->set_result(eCAL::pb::rec::eServiceResult::success);
    }
    else
    {
      response->set_result(eCAL::pb::rec::eServiceResult::failed);
      response->set_error("Unable to start recording.");
      //TODO: improve error message
      return;
    }
  }

  //////////////////////////////////////
  // stop_recording                   //
  //////////////////////////////////////
  else if (command == eCAL::pb::rec::CommandRequest::stop_recording)
  {
    bool success = false;
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QMetaObject::invokeMethod(QEcalRec::instance(), "stopRecording", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, success), Q_ARG(bool, true));
#else
    QMetaObject::invokeMethod(QEcalRec::instance(), []() {return QEcalRec::instance()->stopRecording(true); }, Qt::BlockingQueuedConnection, &success);
#endif

    if (success)
    {
      response->set_result(eCAL::pb::rec::eServiceResult::success);
    }
    else
    {
      response->set_result(eCAL::pb::rec::eServiceResult::failed);
      response->set_error("Unable to stop recording.");
      //TODO: improve error message
      return;
    }
  }

  //////////////////////////////////////
  // save_buffer                      //
  //////////////////////////////////////
  else if (command == eCAL::pb::rec::CommandRequest::save_buffer)
  {
    bool success = false;
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QMetaObject::invokeMethod(QEcalRec::instance(), "saveCurrentBuffer", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, success), Q_ARG(bool, true));
#else
    QMetaObject::invokeMethod(QEcalRec::instance(), []() {return QEcalRec::instance()->saveCurrentBuffer(true); }, Qt::BlockingQueuedConnection, &success);
#endif

    if (success)
    {
      response->set_result(eCAL::pb::rec::eServiceResult::success);
    }
    else
    {
      response->set_result(eCAL::pb::rec::eServiceResult::failed);
      response->set_error("Unable to save buffer.");
      //TODO: improve error message
      return;
    }
  }

  //////////////////////////////////////
  // add_scenario                     //
  //////////////////////////////////////
  else if (command == eCAL::pb::rec::CommandRequest::add_scenario)
  {
    response->set_result(eCAL::pb::rec::eServiceResult::failed);
    response->set_error("TODO: Implement!");
  }

  //////////////////////////////////////
  // exit                             //
  //////////////////////////////////////
  else if (command == eCAL::pb::rec::CommandRequest::exit)
  {
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QMetaObject::invokeMethod(QEcalRec::instance(), "exit", Qt::BlockingQueuedConnection);
#else
    QMetaObject::invokeMethod(QEcalRec::instance(), []() {return QEcalRec::instance()->exit(); }, Qt::BlockingQueuedConnection);
#endif
    response->set_result(eCAL::pb::rec::eServiceResult::success);
  }

  else
  {
    response->set_result(eCAL::pb::rec::eServiceResult::failed);
    response->set_error("Unknown command: " + std::to_string(command));
  }
}

bool EcalRecService::strToBool(const std::string& str)
{
  if (str == "1")
    return true;

  std::string lower_string = str;
  std::transform(lower_string.begin(), lower_string.end(), lower_string.begin(), ::tolower);

  if (lower_string == "true")
    return true;
  else
    return false;
}
