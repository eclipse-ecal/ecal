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

#include <commands/start_tasks.h>
#include <commands/stop_tasks.h>
#include <commands/restart_tasks.h>
#include <commands/list.h>
#include <commands/sleep.h>
#include <commands/exit.h>
#include <commands/load_config.h>
#include <commands/update_from_cloud.h>

namespace eCAL
{
  namespace sys
  {

    /////////////////////////////////
    // Constructor & Destructor
    /////////////////////////////////

    CommandExecutor::CommandExecutor(const std::shared_ptr<EcalSys>& ecalsys_instance, const std::string& remote_hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::sys::Service>>& remote_ecalsys_service)
      : ecalsys_instance_      (ecalsys_instance)
      , remote_hostname_       (remote_hostname)
      , remote_ecalsys_service_(remote_ecalsys_service)
    {
      assert(bool(ecalsys_instance) != bool(remote_ecalsys_service)); // Make sure that either the ecalsys instance or the remote ecalsys service is set (and only one of them!)

      command_map_.emplace(std::make_pair<std::string, std::unique_ptr<eCAL::sys::command::Command>>("start",             std::make_unique<command::StartTask>()));
      command_map_.emplace(std::make_pair<std::string, std::unique_ptr<eCAL::sys::command::Command>>("stop",              std::make_unique<command::StopTask>()));
      command_map_.emplace(std::make_pair<std::string, std::unique_ptr<eCAL::sys::command::Command>>("restart",           std::make_unique<command::RestartTask>()));
      command_map_.emplace(std::make_pair<std::string, std::unique_ptr<eCAL::sys::command::Command>>("list",              std::make_unique<command::List>()));
      command_map_.emplace(std::make_pair<std::string, std::unique_ptr<eCAL::sys::command::Command>>("sleep",             std::make_unique<command::Sleep>()));
      command_map_.emplace(std::make_pair<std::string, std::unique_ptr<eCAL::sys::command::Command>>("exit",              std::make_unique<command::Exit>()));
      command_map_.emplace(std::make_pair<std::string, std::unique_ptr<eCAL::sys::command::Command>>("load",              std::make_unique<command::LoadConfig>()));
      command_map_.emplace(std::make_pair<std::string, std::unique_ptr<eCAL::sys::command::Command>>("update_from_cloud", std::make_unique<command::UpdateFromCloud>()));
    }

    CommandExecutor::~CommandExecutor()
    {}

    /////////////////////////////////
    // Generic Command Parsing
    /////////////////////////////////

    Error CommandExecutor::ExecuteCommand(const std::string& command_string)
    {
      std::vector<std::string> argv = EcalUtils::CommandLine::ToArgv(command_string);
      
      if (argv.empty())
      {
        return Error(Error::ErrorCode::NO_COMMAND);
      }
      else
      {
        std::string command = argv[0];
        std::transform(command.begin(), command.end(), command.begin(), [](char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });

        std::vector<std::string> short_argv(std::next(argv.begin()), argv.end());

        if (command == "help")
          return ExecuteCommandHelp(short_argv);

        auto command_it = command_map_.find(command);
        if (command_it == command_map_.end())
        {
          return Error(Error::ErrorCode::UNKNOWN_COMMAND, command);
        }
        else
        {
          if (ecalsys_instance_)
            return command_it->second->Execute(ecalsys_instance_, short_argv);
          else
            return command_it->second->Execute(remote_hostname_, remote_ecalsys_service_, short_argv);
        }
      }
    }


    Error CommandExecutor::ExecuteCommandHelp(const std::vector<std::string>& argv)
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
          return Error(Error::ErrorCode::OK);
        }
        else
        {
          std::string command = argv[0];
          std::transform(command.begin(), command.end(), command.begin(), [](char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });

          auto command_it = command_map_.find(command);
          if (command_it == command_map_.end())
          {
            return Error(Error::ErrorCode::UNKNOWN_COMMAND, command);
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
            return Error(Error::ErrorCode::OK);
          }
        }
      }

      return Error::ErrorCode::OK;
    }
  }
}