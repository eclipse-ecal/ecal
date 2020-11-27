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

#pragma once

#include <memory>
#include <vector>
#include <map>
#include <ecalsys/ecal_sys.h>

#include <sys_error.h>

#include <commands/command.h>

namespace eCAL
{
  namespace sys
  {
    class CommandExecutor
    {

      /////////////////////////////////
      // Constructor & Destructor
      /////////////////////////////////
    public:
      CommandExecutor(const std::shared_ptr<EcalSys>& ecalsys_instance);
      ~CommandExecutor();

      /////////////////////////////////
      // Generic Command Parsing
      /////////////////////////////////
    public:
      Error ExecuteCommand(const std::string& command_string);

      Error ExecuteCommandHelp(const std::vector<std::string>& argv);

      /////////////////////////////////
      // EcalSys Commands
      /////////////////////////////////

      //Error ExecuteCommandStartTasks(const std::vector<std::string>& argv);
      //Error ExecuteCommandStartTasks(const std::vector<std::shared_ptr<EcalSysTask>>& tasks);

      //Error ExecuteCommandStopTasks(const std::vector<std::string>& argv);
      //Error ExecuteCommandStopTasks(const std::vector<std::shared_ptr<EcalSysTask>>& tasks);    

      //Error ExecuteCommandRestartTasks(const std::vector<std::string>& argv);
      //Error ExecuteCommandRestartTasks(const std::vector<std::shared_ptr<EcalSysTask>>& tasks);

      //Error ExecuteCommandListTasks(const std::vector<std::string>& argv);
      //Error ExecuteCommandListAllTasks();
      //Error ExecuteCommandListTask(const std::shared_ptr<EcalSysTask>& task);

    /////////////////////////////////
    // Member variables
    /////////////////////////////////
    private:
      std::shared_ptr<EcalSys> ecalsys_instance_;

      std::map<std::string, std::unique_ptr<eCAL::sys::command::Command>> command_map_;
    };
  }
}