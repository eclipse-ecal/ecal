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

#include "ecal_rec_service.h"

#include <rec_core/ecal_rec.h>
#include <rec_core/ecal_rec_logger.h>
#include <clocale>
#include <locale>

#include <EcalUtils/EcalUtils.h>

#include <ecal/ecal.h>


EcalRecService::EcalRecService(std::shared_ptr<eCAL::rec::EcalRec> ecal_rec)
  : eCAL::pb::rec::EcalRecService()
  , ecal_rec_(ecal_rec)
  , exit_command_(false)
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
  
  std::string max_pre_buffer_length_secs_string = std::to_string(std::chrono::duration_cast<std::chrono::duration<double>>(ecal_rec_->GetMaxPreBufferLength()).count());
  std::replace(max_pre_buffer_length_secs_string.begin(), max_pre_buffer_length_secs_string.end(), decimal_point, '.');
  (*config_item_map)["max_pre_buffer_length_secs"] = max_pre_buffer_length_secs_string;
  (*config_item_map)["pre_buffering_enabled"]      = (ecal_rec_->IsPreBufferingEnabled() ? "true" : "false");
  (*config_item_map)["host_filter"]                = EcalUtils::String::Join("\n", ecal_rec_->GetHostsFilter());
  std::string record_mode_string;
  switch (ecal_rec_->GetRecordMode())
  {
  case eCAL::rec::RecordMode::Blacklist:
    record_mode_string = "blacklist";
    break;
  case eCAL::rec::RecordMode::Whitelist:
    record_mode_string = "blacklist";
    break;
  default:
    record_mode_string = "all";
  }
  (*config_item_map)["record_mode"]            = record_mode_string;
  (*config_item_map)["listed_topics"]          = EcalUtils::String::Join("\n", ecal_rec_->GetListedTopics());

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

  //////////////////////////////////////
  // max_pre_buffer_length_secs           //
  //////////////////////////////////////
  if (config_item_map.find("max_pre_buffer_length_secs") != config_item_map.end())
  {
    std::string max_pre_buffer_length_secs_string = config_item_map["max_pre_buffer_length_secs"];
    std::replace(max_pre_buffer_length_secs_string.begin(), max_pre_buffer_length_secs_string.end(), '.', decimal_point);
    double max_pre_buffer_length_secs = 0.0;
    try
    {
      max_pre_buffer_length_secs = std::stod(max_pre_buffer_length_secs_string);
    }
    catch (const std::exception& e)
    {
      response->set_result(eCAL::pb::rec::eServiceResult::failed);
      response->set_error("Error parsing value \"" + max_pre_buffer_length_secs_string + "\": " + e.what());
      return;
    }

    auto max_buffer_length = std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(max_pre_buffer_length_secs));

    ecal_rec_->SetMaxPreBufferLength(max_buffer_length);
  }

  //////////////////////////////////////
  // pre_buffering_enabled                //
  //////////////////////////////////////
  if (config_item_map.find("pre_buffering_enabled") != config_item_map.end())
  {
    std::string pre_buffering_enabled_string = config_item_map["pre_buffering_enabled"];
    bool pre_buffering_enabled = strToBool(pre_buffering_enabled_string);
    
    ecal_rec_->SetPreBufferingEnabled(pre_buffering_enabled);
  }

  //////////////////////////////////////
  // host_filter                      //
  //////////////////////////////////////
  if (config_item_map.find("host_filter") != config_item_map.end())
  {
    std::string host_filter_string = config_item_map["host_filter"];

    std::set<std::string> host_filter;
    EcalUtils::String::Split(host_filter_string, "\n", host_filter);

    bool success = ecal_rec_->SetHostFilter(host_filter);

    if (!success)
    {
      response->set_result(eCAL::pb::rec::eServiceResult::failed);
      response->set_error("Error setting host filter. Stop the recorder and try again.");
      return;
    }
  }

  //////////////////////////////////////
  // record_mode and listed_topics    //
  //////////////////////////////////////

  
  // Record Mode

  eCAL::rec::RecordMode record_mode(eCAL::rec::RecordMode::All);
  bool record_mode_set = false;

  if (config_item_map.find("record_mode") != config_item_map.end())
  {
    std::string record_mode_string = config_item_map["record_mode"];
    record_mode_string = EcalUtils::String::Trim(record_mode_string);
    std::transform(record_mode_string.begin(), record_mode_string.end(), record_mode_string.begin(), ::tolower);
    
    if (record_mode_string == "all")
      record_mode = eCAL::rec::RecordMode::All;
    else if (record_mode_string == "blacklist")
      record_mode = eCAL::rec::RecordMode::Blacklist;
    else if (record_mode_string == "whitelist")
      record_mode = eCAL::rec::RecordMode::Whitelist;
    else
    {
      response->set_result(eCAL::pb::rec::eServiceResult::failed);
      response->set_error("Error parsing record mode \"" + record_mode_string + "\"");
      return;
    }
    record_mode_set = true;
  }

  // Listed topics

  std::set<std::string> listed_topics;
  bool listed_topics_set = false;

  if (config_item_map.find("listed_topics") != config_item_map.end())
  {
    std::string listed_topics_string = config_item_map["listed_topics"];
    
    EcalUtils::String::Split(listed_topics_string, "\n", listed_topics);
    listed_topics_set = true;
  }

  // Apply Record Mode, Listed topics or both

  if (record_mode_set && listed_topics_set)
  {
    bool success = ecal_rec_->SetRecordMode(record_mode, listed_topics);
    if (!success)
    {
      response->set_result(eCAL::pb::rec::eServiceResult::failed);
      response->set_error("Error setting record mode and listed topics. Stop the recorder and try again.");
      return;
    }
  }
  else if (record_mode_set)
  {
    bool success = ecal_rec_->SetRecordMode(record_mode);
    if (!success)
    {
      response->set_result(eCAL::pb::rec::eServiceResult::failed);
      response->set_error("Error setting record mode. Stop the recorder and try again.");
      return;
    }
  }
  else if (listed_topics_set)
  {
    bool success = ecal_rec_->SetListedTopics(listed_topics);
    if (!success)
    {
      response->set_result(eCAL::pb::rec::eServiceResult::failed);
      response->set_error("Error setting listed topics. Stop the recorder and try again.");
      return;
    }
  }

  // END
  response->set_result(eCAL::pb::rec::eServiceResult::success);
}

void EcalRecService::SetCommand(::google::protobuf::RpcController*       /*controller*/
                              , const ::eCAL::pb::rec::CommandRequest*   request
                              , ::eCAL::pb::rec::Response*               response
                              , ::google::protobuf::Closure*             /*done*/)
{
  auto command = request->command();

  //////////////////////////////////////
  // None                             //
  //////////////////////////////////////
  if (command == eCAL::pb::rec::CommandRequest::none)
  {
    response->set_result(eCAL::pb::rec::eServiceResult::success);
  }

  //////////////////////////////////////
  // initialize                       //
  //////////////////////////////////////
  else if (command == eCAL::pb::rec::CommandRequest::initialize)
  {
    ecal_rec_->ConnectToEcal();
    response->set_result(eCAL::pb::rec::eServiceResult::success);
  }

  //////////////////////////////////////
  // de_initialize                    //
  //////////////////////////////////////
  else if (command == eCAL::pb::rec::CommandRequest::de_initialize)
  {
    ecal_rec_->DisconnectFromEcal();
    response->set_result(eCAL::pb::rec::eServiceResult::success);
  }

  //////////////////////////////////////
  // start_recording                  //
  //////////////////////////////////////
  else if (command == eCAL::pb::rec::CommandRequest::start_recording)
  {
    // Parse the command params to a job configuration
    eCAL::rec::JobConfig job_config = toJobConfig(request->command_params(), response);
    if (response->result() == eCAL::pb::rec::eServiceResult::failed)
    {
      // Something has gone wrong while parsing the parameters
      return;
    }

    bool success = false;
    success = ecal_rec_->StartRecording(job_config);

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
    success = ecal_rec_->StopRecording();

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
  // save_pre_buffer                  //
  //////////////////////////////////////
  else if (command == eCAL::pb::rec::CommandRequest::save_pre_buffer)
  {
    // Parse the command params to a job configuration
    eCAL::rec::JobConfig job_config = toJobConfig(request->command_params(), response);
    if (response->result() == eCAL::pb::rec::eServiceResult::failed)
    {
      // Something has gone wrong while parsing the parameters
      return;
    }

    bool success = false;
    success = ecal_rec_->SavePreBufferedData(job_config);

    if (success)
    {
      response->set_result(eCAL::pb::rec::eServiceResult::success);
    }
    else
    {
      response->set_result(eCAL::pb::rec::eServiceResult::failed);
      response->set_error("Unable to save current pre-buffer.");
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
    exit_command_ = true;
    response->set_result(eCAL::pb::rec::eServiceResult::success);
  }

  else
  {
    response->set_result(eCAL::pb::rec::eServiceResult::failed);
    response->set_error("Unknown command: " + std::to_string(command));
  }
}

void EcalRecService::GetState(::google::protobuf::RpcController*         /*controller*/
                              , const ::eCAL::pb::rec::GetStateRequest*  /*request*/
                              , ::eCAL::pb::rec::State*                  response
                              , ::google::protobuf::Closure*             /*done*/)
{
#ifndef NEDBUG
  eCAL::rec::EcalRecLogger::Instance()->debug("EcalRecService::GetStateRequest");
#endif // !NEDBUG

  auto recorder_state = ecal_rec_->GetRecorderState();

  response->Clear();

  response->set_hostname                            (eCAL::Process::GetHostName());
  response->set_pid                                 (eCAL::Process::GetProcessID());
  response->set_initialized                         (recorder_state.initialized_);
  response->mutable_main_recorder_state()->CopyFrom (toPbWriterState(recorder_state.main_recorder_state_));
  response->set_pre_buffer_length_frames_count          (recorder_state.pre_buffer_length_.first);
  response->set_pre_buffer_length_secs                  (std::chrono::duration_cast<std::chrono::duration<double>>(recorder_state.pre_buffer_length_.second).count());
  
  for (const eCAL::rec::WriterState& writer_state : recorder_state.buffer_writers_)
    response->add_buffer_writer_states()->CopyFrom  (toPbWriterState(writer_state));

  for (const std::string& subscribed_topic : recorder_state.subscribed_topics_)
    response->add_subscribed_topics                 (subscribed_topic);
}

bool EcalRecService::IsExitCommandSet() const
{
  return exit_command_;
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

eCAL::pb::rec::State::WriterState EcalRecService::toPbWriterState(const eCAL::rec::WriterState writer_state)
{
  eCAL::pb::rec::State::WriterState writer_state_pb;

  writer_state_pb.set_recording                   (writer_state.recording_);
  writer_state_pb.set_flushing                    (writer_state.flushing_);
  writer_state_pb.set_recording_length_frame_count(writer_state.recording_length_.first);
  writer_state_pb.set_recording_length_secs       (std::chrono::duration_cast<std::chrono::duration<double>>(writer_state.recording_length_.second).count());
  writer_state_pb.set_queued_frames_count         (writer_state.recording_queue_.first);
  writer_state_pb.set_queued_secs                 (std::chrono::duration_cast<std::chrono::duration<double>>(writer_state.recording_queue_.second).count());

  return writer_state_pb;
}

::eCAL::rec::JobConfig EcalRecService::toJobConfig(const ::eCAL::pb::rec::Configuration& config, ::eCAL::pb::rec::Response* response)
{
  eCAL::rec::JobConfig job_config;

  //////////////////////////////////////
  // meas_root_dir                    //
  //////////////////////////////////////
  {
    auto it = config.items().find("meas_root_dir");
    if (it != config.items().end())
    {
      std::string meas_root_dir = it->second;
      job_config.SetMeasRootDir(meas_root_dir);
    }
  }

  //////////////////////////////////////
  // meas_name                        //
  //////////////////////////////////////
  {
    auto it = config.items().find("meas_name");
    if (it != config.items().end())
    {
      std::string meas_name = it->second;
      job_config.SetMeasName(meas_name);
    }
  }

  //////////////////////////////////////
  // max_file_size_mib                //
  //////////////////////////////////////
  {
    auto it = config.items().find("max_file_size_mib");
    if (it != config.items().end())
    {
      std::string max_file_size_mib_string = it->second;
      unsigned long long max_file_size = 0;
      try
      {
        max_file_size = std::stoull(max_file_size_mib_string);
      }
      catch (const std::exception& e)
      {
        response->set_result(eCAL::pb::rec::eServiceResult::failed);
        response->set_error("Error parsing value \"" + max_file_size_mib_string + "\": " + e.what());
        return  job_config;
      }

      // Check the input value, so we can savely cast it later
      if (max_file_size > SIZE_MAX)
      {
        response->set_result(eCAL::pb::rec::eServiceResult::failed);
        response->set_error("Error setting max file size to " + max_file_size_mib_string + "MiB: Value too large");
        return job_config;
      }

      size_t max_file_size_sizet = static_cast<size_t>(max_file_size);

      job_config.SetMaxFileSize(max_file_size_sizet);
    }
  }

  //////////////////////////////////////
  // description                      //
  //////////////////////////////////////
  {
    auto it = config.items().find("description");
    if (it != config.items().end())
    {
      std::string description = it->second;
      job_config.SetDescription(description);
    }
  }

  response->set_result(eCAL::pb::rec::eServiceResult::success);
  return job_config;
}
