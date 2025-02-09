/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
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

#include "command_executor.h"

#include <ecal_utils/ecal_utils.h>

#include <map>
#include <functional>
#include <memory>
#include <iostream>

#include <commands/activate.h>
#include <commands/comment.h>
#include <commands/deactivate.h>
#include <commands/delete.h>
#include <commands/exit.h>
#include <commands/get_config.h>
#include <commands/load_config.h>
#include <commands/record.h>
#include <commands/save_config.h>
#include <commands/save_pre_buffer.h>
#include <commands/set_config.h>
#include <commands/sleep.h>
#include <commands/status.h>
#include <commands/stop_recording.h>
#include <commands/upload.h>

namespace eCAL
{
  namespace rec_cli
  {

    /////////////////////////////////
    // Constructor & Destructor
    /////////////////////////////////

    CommandExecutor::CommandExecutor(const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::string& remote_hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service)
      : rec_server_instance_      (rec_server_instance)
      , remote_hostname_          (remote_hostname)
      , remote_rec_server_service_(remote_rec_server_service)
      , exit_command_called_      (false)
    {
      assert(bool(rec_server_instance) != bool(remote_rec_server_service)); // Make sure that either the ecalsys instance or the remote ecalsys service is set (and only one of them!)

      command_map_.emplace_back("load",              std::make_unique<command::LoadConfig>());
      command_map_.emplace_back("save",              std::make_unique<command::SaveConfig>());

      command_map_.emplace_back("getconfig",         std::make_unique<command::GetConfig>());
      command_map_.emplace_back("setconfig",         std::make_unique<command::SetConfig>());

      command_map_.emplace_back("activate",          std::make_unique<command::Activate>());
      command_map_.emplace_back("deactivate",        std::make_unique<command::DeActivate>());
      command_map_.emplace_back("rec",               std::make_unique<command::Record>());
      command_map_.emplace_back("savebuffer",        std::make_unique<command::SavePreBuffer>());
      command_map_.emplace_back("stop",              std::make_unique<command::StopRecording>());

      command_map_.emplace_back("status",            std::make_unique<command::Status>());

      command_map_.emplace_back("comment",           std::make_unique<command::Comment>());
      command_map_.emplace_back("upload",            std::make_unique<command::Upload>());
      command_map_.emplace_back("delete",            std::make_unique<command::Delete>());

      command_map_.emplace_back("sleep",             std::make_unique<command::Sleep>());

      command_map_.emplace_back("exit",              std::make_unique<command::Exit>());
    }

    CommandExecutor::~CommandExecutor()
    {}

    /////////////////////////////////
    // Generic Command Parsing
    /////////////////////////////////

    eCAL::rec::Error CommandExecutor::ExecuteCommand(const std::string& command_string)
    {
      std::vector<std::string> argv = EcalUtils::CommandLine::ToArgv(command_string);
      
      if (argv.empty())
      {
        return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::NO_COMMAND);
      }
      else
      {
        std::string command = argv[0];
        std::transform(command.begin(), command.end(), command.begin(), [](char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });

        std::vector<std::string> short_argv(std::next(argv.begin()), argv.end());

        if (command == "help")
          return ExecuteCommandHelp(short_argv);

        if (command == "exit")
        {
          exit_command_called_ = true;
          return eCAL::rec::Error::ErrorCode::OK;
        }

        auto command_it = std::find_if(command_map_.begin(), command_map_.end(), [&command](const auto& cmd_pair)->bool { return cmd_pair.first == command; });
        if (command_it == command_map_.end())
        {
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::UNKNOWN_COMMAND, command);
        }
        else
        {
          if (rec_server_instance_)
            return command_it->second->Execute(rec_server_instance_, short_argv);
          else
            return command_it->second->Execute(remote_hostname_, remote_rec_server_service_, short_argv);
        }
      }
    }


    eCAL::rec::Error CommandExecutor::ExecuteCommandHelp(const std::vector<std::string>& argv)
    {
      if (argv.empty())
      {
        std::cout << "Available commands:" << std::endl;
        std::cout << std::endl;
        for (const auto& command : command_map_)
        {
          std::cout << command.first << " "  << command.second->Usage() << std::endl;
        }
        std::cout << std::endl;
        std::cout << "For getting additional information, enter:" << std::endl << std::endl;
        std::cout << "  help <COMMAND_NAME | --all>" << std::endl;
      }
      else
      {
        if (argv[0] == std::string("--all"))
        {
          for (auto command_map_it = command_map_.begin(); command_map_it != command_map_.end(); command_map_it++)
          {
            if (command_map_it != command_map_.begin())
              std::cout << std::endl;

            std::cout << command_map_it->first << " " << command_map_it->second->Usage() << std::endl;
            std::cout << "  " << command_map_it->second->Help() << std::endl;
            std::cout << std::endl;
          }
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::OK);
        }
        else
        {
          std::string command = argv[0];
          std::transform(command.begin(), command.end(), command.begin(), [](char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });

          auto command_it = std::find_if(command_map_.begin(), command_map_.end(), [&command](const auto& cmd_pair)->bool { return cmd_pair.first == command; });
          if (command_it == command_map_.end())
          {
            return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::UNKNOWN_COMMAND, command);
          }
          else
          {
            std::cout << "Usage: " << command << " " << command_it->second->Usage() << std::endl;
            std::cout << std::endl;
            std::cout << command_it->second->Help() << std::endl;
            std::cout << std::endl;
            std::cout << "Example:" << std::endl;
            std::cout << "  " << command << " " << command_it->second->Example() << std::endl;
            std::cout << std::endl;
            return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::OK);
          }
        }
      }

      return eCAL::rec::Error::ErrorCode::OK;
    }

    void CommandExecutor::Interrupt() const
    {
      for (const auto& command : command_map_)
      {
        command.second->Interrupt();
      }
    }

    bool CommandExecutor::IsExitCommandCalled() const
    {
      return exit_command_called_;
    }
  }
}