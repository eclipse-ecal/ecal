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

#include <rec_client_core/ecal_rec.h>
#include <rec_client_core/ecal_rec_logger.h>
#include <rec_client_core/proto_helpers.h>

#include <clocale>
#include <locale>

#include <ecal_utils/ecal_utils.h>

#include <ecal/ecal.h>


EcalRecService::EcalRecService(std::shared_ptr<eCAL::rec::EcalRec> ecal_rec)
  : eCAL::pb::rec_client::EcalRecClientService()
  , ecal_rec_(ecal_rec)
  , exit_command_(false)
{}

EcalRecService::~EcalRecService()
{}

void EcalRecService::GetConfig(::google::protobuf::RpcController*        /*controller*/
                              , const ::eCAL::pb::rec_client::GetConfigRequest* /*request*/
                              , ::eCAL::pb::rec_client::GetConfigResponse*      response
                              , ::google::protobuf::Closure*             /*done*/)
{
  eCAL::pb::rec_client::Configuration config;
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
    record_mode_string = "whitelist";
    break;
  default:
    record_mode_string = "all";
  }
  (*config_item_map)["record_mode"]            = record_mode_string;
  (*config_item_map)["listed_topics"]          = EcalUtils::String::Join("\n", ecal_rec_->GetListedTopics());
  (*config_item_map)["enabled_addons"]         = EcalUtils::String::Join("\n", ecal_rec_->GetEnabledAddons());

  response->mutable_config()->CopyFrom(config);
  response->set_result(eCAL::pb::rec_client::ServiceResult::success);
}

void EcalRecService::SetConfig(::google::protobuf::RpcController*        /*controller*/
                              , const ::eCAL::pb::rec_client::SetConfigRequest* request
                              , ::eCAL::pb::rec_client::Response*               response
                              , ::google::protobuf::Closure*             /*done*/)
{
  auto config_item_map = request->config().items();

  char decimal_point = std::localeconv()->decimal_point[0]; // decimal point for std::stod()

  //////////////////////////////////////
  // max_pre_buffer_length_secs       //
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
      response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
      response->set_error("Error parsing value \"" + max_pre_buffer_length_secs_string + "\": " + e.what());
      return;
    }

    auto max_buffer_length = std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(max_pre_buffer_length_secs));

    ecal_rec_->SetMaxPreBufferLength(max_buffer_length);
  }

  //////////////////////////////////////
  // pre_buffering_enabled            //
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
      response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
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
    std::transform(record_mode_string.begin(), record_mode_string.end(), record_mode_string.begin(), [](char c) {return static_cast<char>(::tolower(c)); });
    
    if (record_mode_string == "all")
      record_mode = eCAL::rec::RecordMode::All;
    else if (record_mode_string == "blacklist")
      record_mode = eCAL::rec::RecordMode::Blacklist;
    else if (record_mode_string == "whitelist")
      record_mode = eCAL::rec::RecordMode::Whitelist;
    else
    {
      response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
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
      response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
      response->set_error("Error setting record mode and listed topics. Stop the recorder and try again.");
      return;
    }
  }
  else if (record_mode_set)
  {
    bool success = ecal_rec_->SetRecordMode(record_mode);
    if (!success)
    {
      response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
      response->set_error("Error setting record mode. Stop the recorder and try again.");
      return;
    }
  }
  else if (listed_topics_set)
  {
    bool success = ecal_rec_->SetListedTopics(listed_topics);
    if (!success)
    {
      response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
      response->set_error("Error setting listed topics. Stop the recorder and try again.");
      return;
    }
  }

  //////////////////////////////////////
  // enabled_addons                   //
  //////////////////////////////////////

  if (config_item_map.find("enabled_addons") != config_item_map.end())
  {
    std::string enabled_addons_string = config_item_map["enabled_addons"];

    std::set<std::string> enabled_addons;
    EcalUtils::String::Split(enabled_addons_string, "\n", enabled_addons);

    bool success = ecal_rec_->SetEnabledAddons(enabled_addons);

    if (!success)
    {
      response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
      response->set_error("Error setting enabled addons. Stop the recorder and try again.");
      return;
    }
  }

  // END
  response->set_result(eCAL::pb::rec_client::ServiceResult::success);
}

void EcalRecService::SetCommand(::google::protobuf::RpcController*       /*controller*/
                              , const ::eCAL::pb::rec_client::CommandRequest*   request
                              , ::eCAL::pb::rec_client::Response*               response
                              , ::google::protobuf::Closure*             /*done*/)
{
  auto command = request->command();

  //////////////////////////////////////
  // None                             //
  //////////////////////////////////////
  if (command == eCAL::pb::rec_client::CommandRequest::none)
  {
    response->set_result(eCAL::pb::rec_client::ServiceResult::success);
  }

  //////////////////////////////////////
  // initialize                       //
  //////////////////////////////////////
  else if (command == eCAL::pb::rec_client::CommandRequest::initialize)
  {
    ecal_rec_->ConnectToEcal();
    response->set_result(eCAL::pb::rec_client::ServiceResult::success);
  }

  //////////////////////////////////////
  // de_initialize                    //
  //////////////////////////////////////
  else if (command == eCAL::pb::rec_client::CommandRequest::de_initialize)
  {
    ecal_rec_->DisconnectFromEcal();
    response->set_result(eCAL::pb::rec_client::ServiceResult::success);
  }

  //////////////////////////////////////
  // start_recording                  //
  //////////////////////////////////////
  else if (command == eCAL::pb::rec_client::CommandRequest::start_recording)
  {
    // Parse the command params to a job configuration
    eCAL::rec::JobConfig job_config = ToJobConfig(request->command_params(), response);
    if (response->result() == eCAL::pb::rec_client::ServiceResult::failed)
    {
      // Something has gone wrong while parsing the parameters
      return;
    }

    bool success = false;
    success = ecal_rec_->StartRecording(job_config);

    if (success)
    {
      response->set_result(eCAL::pb::rec_client::ServiceResult::success);
    }
    else
    {
      response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
      response->set_error("Unable to start recording.");
      return;
    }
  }

  //////////////////////////////////////
  // stop_recording                   //
  //////////////////////////////////////
  else if (command == eCAL::pb::rec_client::CommandRequest::stop_recording)
  {
    bool success = false;
    success = ecal_rec_->StopRecording();

    if (success)
    {
      response->set_result(eCAL::pb::rec_client::ServiceResult::success);
    }
    else
    {
      response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
      response->set_error("Unable to stop recording.");
      return;
    }
  }

  //////////////////////////////////////
  // save_pre_buffer                  //
  //////////////////////////////////////
  else if (command == eCAL::pb::rec_client::CommandRequest::save_pre_buffer)
  {
    // Parse the command params to a job configuration
    eCAL::rec::JobConfig job_config = ToJobConfig(request->command_params(), response);
    if (response->result() == eCAL::pb::rec_client::ServiceResult::failed)
    {
      // Something has gone wrong while parsing the parameters
      return;
    }

    bool success = false;
    success = ecal_rec_->SavePreBufferedData(job_config);

    if (success)
    {
      response->set_result(eCAL::pb::rec_client::ServiceResult::success);
    }
    else
    {
      response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
      response->set_error("Unable to save current pre-buffer.");
      return;
    }
  }

  //////////////////////////////////////
  // upload_measurement               //
  //////////////////////////////////////
  else if (command == eCAL::pb::rec_client::CommandRequest::upload_measurement)
  {
    // Parse the command params to a merge configuration
    eCAL::rec::UploadConfig upload_config = ToUploadConfig(request->command_params(), response);
    if (response->result() == eCAL::pb::rec_client::ServiceResult::failed)
    {
      // Something has gone wrong while parsing the parameters
      return;
    }

    eCAL::rec::Error error = ecal_rec_->UploadMeasurement(upload_config);

    if (!error)
    {
      response->set_result(eCAL::pb::rec_client::ServiceResult::success);
    }
    else
    {
      response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
      response->set_error("Unable to merge measurement: " + error.ToString());
      return;
    }
  }

  //////////////////////////////////////
  // add_comment                      //
  //////////////////////////////////////
  else if (command == eCAL::pb::rec_client::CommandRequest::add_comment)
  {
    int64_t meas_id = 0;
    std::string comment;

    // Parse meas_id
    {
      auto it = request->command_params().items().find("meas_id");
      if (it != request->command_params().items().end())
      {
        std::string meas_id_string = it->second;

        try
        {
          meas_id = std::stoll(meas_id_string);
        }
        catch (const std::exception& e)
        {
          response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
          response->set_error("Error parsing value \"" + meas_id_string + "\": " + e.what());
          return;
        }
      }
      else
      {
        response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
        response->set_error("Missing meas_id");
        return;
      }
    }

    // parse comment
    {
      auto it = request->command_params().items().find("comment");
      if (it != request->command_params().items().end())
      {
        comment = it->second;
      }
    }

    eCAL::rec::Error error = ecal_rec_->AddComment(meas_id, comment);
    if (!error)
    {
      response->set_result(eCAL::pb::rec_client::ServiceResult::success);
    }
    else
    {
      response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
      response->set_error("Error adding comment: " + error.ToString());
      return;
    }
  }

  else if (command == eCAL::pb::rec_client::CommandRequest::delete_measurement)
  {
    int64_t meas_id;

    // Parse meas_id
    {
      auto it = request->command_params().items().find("meas_id");
      if (it != request->command_params().items().end())
      {
        std::string meas_id_string = it->second;

        try
        {
          meas_id = std::stoll(meas_id_string);
        }
        catch (const std::exception& e)
        {
          response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
          response->set_error("Error parsing value \"" + meas_id_string + "\": " + e.what());
          return;
        }
      }
      else
      {
        response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
        response->set_error("Missing meas_id");
        return;
      }
    }

    eCAL::rec::Error error = ecal_rec_->DeleteMeasurement(meas_id);

    if (!error)
    {
      response->set_result(eCAL::pb::rec_client::ServiceResult::success);
    }
    else
    {
      response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
      response->set_error("Unable to delete measurement: " + error.ToString());
      return;
    }
  }

  //////////////////////////////////////
  // exit                             //
  //////////////////////////////////////
  else if (command == eCAL::pb::rec_client::CommandRequest::exit)
  {
    exit_command_ = true;
    response->set_result(eCAL::pb::rec_client::ServiceResult::success);
  }

  else
  {
    response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
    response->set_error("Unknown command: " + std::to_string(command));
  }
}

void EcalRecService::GetState(::google::protobuf::RpcController*         /*controller*/
                              , const ::eCAL::pb::rec_client::GetStateRequest*  /*request*/
                              , ::eCAL::pb::rec_client::State*                  response
                              , ::google::protobuf::Closure*             /*done*/)
{
#ifndef NEDBUG
  eCAL::rec::EcalRecLogger::Instance()->debug("EcalRecService::GetStateRequest");
#endif // !NEDBUG

  auto recorder_status = ecal_rec_->GetRecorderStatus();

  response->Clear();

  eCAL::rec::proto_helpers::ToProtobuf(recorder_status, eCAL::Process::GetHostName(), *response);
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

  std::transform(lower_string.begin(), lower_string.end(), lower_string.begin(), [](char c) {return static_cast<char>(::tolower(c)); });

  if (lower_string == "true")
    return true;
  else
    return false;
}

::eCAL::rec::JobConfig EcalRecService::ToJobConfig(const ::eCAL::pb::rec_client::Configuration& config, ::eCAL::pb::rec_client::Response* response)
{
  eCAL::rec::JobConfig job_config;

  //////////////////////////////////////
  // meas_id                          //
  //////////////////////////////////////
  {
    auto it = config.items().find("meas_id");
    if (it != config.items().end())
    {
      std::string meas_id_string = it->second;
      int64_t meas_id = 0;

      try
      {
        meas_id = std::stoll(meas_id_string);
      }
      catch (const std::exception& e)
      {
        response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
        response->set_error("Error parsing value \"" + meas_id_string + "\": " + e.what());
        return  job_config;
      }

      job_config.SetJobId(meas_id);
    }
    else
    {
      response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
      response->set_error("Missing meas_id");
      return  job_config;
    }
  }

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
        response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
        response->set_error("Error parsing value \"" + max_file_size_mib_string + "\": " + e.what());
        return  job_config;
      }

      // Check the input value, so we can savely cast it later
      if (max_file_size > std::numeric_limits<unsigned int>::max())
      {
        response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
        response->set_error("Error setting max file size to " + max_file_size_mib_string + "MiB: Value too large");
        return job_config;
      }

      job_config.SetMaxFileSize(static_cast<unsigned int>(max_file_size));
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

  response->set_result(eCAL::pb::rec_client::ServiceResult::success);
  return job_config;
}

::eCAL::rec::UploadConfig EcalRecService::ToUploadConfig(const ::eCAL::pb::rec_client::Configuration& config, ::eCAL::pb::rec_client::Response* response)
{
  eCAL::rec::UploadConfig upload_config;

  //////////////////////////////////////
  // protocol                         //
  //////////////////////////////////////
  {
    auto it = config.items().find("protocol");
    if (it != config.items().end())
    {
      std::string protocol_string = it->second;
      if (EcalUtils::String::Icompare(protocol_string, "ftp"))
      {
        upload_config.protocol_ = eCAL::rec::UploadConfig::Type::FTP;
      }
      else
      {
        response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
        response->set_error("Error parsing merge-config: Unknown Protocol \"" + protocol_string + "\"");
        return  upload_config;
      }
    }
    else
    {
      response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
      response->set_error("Error parsing merge-config: Not protocol given.");
      return  upload_config;
    }
  }

  //////////////////////////////////////
  // meas_id                          //
  //////////////////////////////////////
  {
    auto it = config.items().find("meas_id");
    if (it != config.items().end())
    {
      std::string meas_id_string = it->second;
      int64_t meas_id = 0;

      try
      {
        meas_id = std::stoll(meas_id_string);
      }
      catch (const std::exception& e)
      {
        response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
        response->set_error("Error parsing value \"" + meas_id_string + "\": " + e.what());
        return  upload_config;
      }

      upload_config.meas_id_ = meas_id;
    }
    else
    {
      response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
      response->set_error("Missing meas_id");
      return  upload_config;
    }
  }


  //////////////////////////////////////
  // username                         //
  //////////////////////////////////////
  {
    auto it = config.items().find("username");
    if (it != config.items().end())
    {
      std::string username = it->second;
      upload_config.username_ = username;
    }
    else
    {
      upload_config.username_ = "anonymous";
    }
  }

  //////////////////////////////////////
  // password                         //
  //////////////////////////////////////
  {
    auto it = config.items().find("password");
    if (it != config.items().end())
    {
      std::string password = it->second;
      upload_config.password_ = password;
    }
    else
    {
      upload_config.password_ = "anonymous";
    }
  }

  //////////////////////////////////////
  // host                             //
  //////////////////////////////////////
  {
    bool success = false;

    auto it = config.items().find("host");
    if (it != config.items().end())
    {
      std::string host = it->second;
      if (!host.empty())
      {
        upload_config.host_ = host;
        success = true;
      }
    }

    if (!success)
    {
      response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
      response->set_error("Error: No hostname is given");
      return  upload_config;
    }
  }

  //////////////////////////////////////
  // port                             //
  //////////////////////////////////////
  {
    auto it = config.items().find("port");
    if (it != config.items().end())
    {
      std::string port_string = it->second;
      int16_t port = 21; // = default FTP port

      try
      {
        port = static_cast<uint16_t>(std::stoul(port_string));
      }
      catch (const std::exception& e)
      {
        response->set_result(eCAL::pb::rec_client::ServiceResult::failed);
        response->set_error("Error parsing value \"" + port_string + "\": " + e.what());
        return  upload_config;
      }

      upload_config.port_ = port;
    }
  }


  //////////////////////////////////////
  // upload_path                      //
  //////////////////////////////////////
  {
    auto it = config.items().find("upload_path");
    if (it != config.items().end())
    {
      std::string upload_path = it->second;
      upload_config.upload_path_ = upload_path;
    }
    else
    {
      upload_config.upload_path_ = "/";
    }
  }

  //////////////////////////////////////
  // upload_metadata_files            //
  //////////////////////////////////////
  {
    auto it = config.items().find("upload_metadata_files");
    if (it != config.items().end())
    {
      std::string upload_metadata_files_string = it->second;
      upload_config.upload_metadata_files_ = strToBool(upload_metadata_files_string);
    }
    else
    {
      upload_config.upload_metadata_files_ = true;
    }
  }

  //////////////////////////////////////
  // delete_after_upload              //
  //////////////////////////////////////
  {
    auto it = config.items().find("delete_after_upload");
    if (it != config.items().end())
    {
      std::string upload_metadata_files_string = it->second;
      upload_config.delete_after_upload_ = strToBool(upload_metadata_files_string);
    }
    else
    {
      upload_config.delete_after_upload_ = false;
    }
  }

  response->set_result(eCAL::pb::rec_client::ServiceResult::success);
  return upload_config;
}
