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
#include <queue>
#include <string>
#include <tuple>
#include <unordered_map>

#include "tui/viewmodel/viewmodel.hpp"

#include "tui/command_manager.hpp"

class HelpViewModel : public ViewModel
{
public:
  std::vector<std::tuple<std::string, std::string>> commands;

  HelpViewModel(std::shared_ptr<CommandManager> command_manager)
  {
    title = "Help";
    auto &supported_commands = command_manager->SupportedCommands();
    auto &command_descriptions = command_manager->CommandDescriptions();

    for(auto &command: supported_commands)
    {
      std::string desc = "";
      auto find_desc = command_descriptions.find(command.second);
      if(find_desc != command_descriptions.end())
      {
        desc = find_desc->second;
      }

      commands.push_back(std::make_tuple(command.first, desc));
    }
  }
};
