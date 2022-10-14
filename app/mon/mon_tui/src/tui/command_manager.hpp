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

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include "utils/container.hpp"
#include "utils/string.hpp"

enum class Command
{
  UNSUPPORTED,
  FIND,
  FIND_NEXT,
  FIND_PREVIOUS,
  FILTER,
  FILTER_BY,
  ORDER_BY,
  ORDER_BY_ASC,
  ORDER_BY_DESC,
  GROUP_BY,
  AUTO_SCROLL_ENABLE,
  AUTO_SCROLL_DISABLE,
  CLEAR,
  DETAILS,
  TOPICS,
  SERVICES,
  HOSTS,
  PROCESSES,
  LOGS,
  SYSTEM_INFORMATION,
  HELP,
  QUIT,

  START_COMMAND,
  START_FIND,
  START_FILTER,
  ACTION,
  CANCEL,
  NAVIGATE_TAB1,
  NAVIGATE_TAB2,
  NAVIGATE_TAB3,
  NAVIGATE_TAB4,
  NAVIGATE_TAB5,
  NAVIGATE_TAB6,
  NAVIGATE_TAB7,
  MOVE_UP,
  MOVE_DOWN,
  MOVE_RIGHT,
  MOVE_LEFT,
  FOCUS_NEXT,
  FOCUS_PREVIOUS
};

struct CommandDetails
{
  Command command;
  std::vector<std::string> args;
};

using CommandDescriptionMap = std::unordered_map<Command, std::string>;
using CommandMap = std::unordered_map<std::string, Command>;

class CommandManager
{
  std::shared_ptr<CommandMap> supported_commands;
  std::shared_ptr<CommandDescriptionMap> command_descriptions;

  bool IsCommandSupported(const std::string &cmd)
  {
    return Contains(*supported_commands, cmd);
  }

  Command GetCommand(const std::string &cmd)
  {
    if(IsCommandSupported(cmd))
    {
      return supported_commands->at(cmd);
    }
    else return Command::UNSUPPORTED;
  }

public:
  CommandManager(std::shared_ptr<CommandMap> supported_commands_,
                 std::shared_ptr<CommandDescriptionMap> command_descriptions_)
    : supported_commands{supported_commands_},
      command_descriptions{command_descriptions_} {}

  const CommandMap& SupportedCommands()
  {
    return *supported_commands;
  }

  const CommandDescriptionMap& CommandDescriptions()
  {
    return *command_descriptions;
  }

  CommandDetails ParseCommand(const std::string &input)
  {
    auto args = SplitWords(input);
    auto command = GetCommand(args[0]);
    args.erase(args.begin());
    return CommandDetails { command, args };
  }
};

std::shared_ptr<CommandMap> CreateCommandMap()
{
  return std::make_shared<CommandMap>(CommandMap
  {
    {"find", Command::FIND},
    {"findnext", Command::FIND_NEXT},
    {"next", Command::FIND_NEXT},
    {"findprev", Command::FIND_PREVIOUS},
    {"prev", Command::FIND_NEXT},
    {"filter", Command::FILTER},
    {"filterby", Command::FILTER_BY},
    {"orderby", Command::ORDER_BY},
    {"orderbyasc", Command::ORDER_BY_ASC},
    {"orderbydesc", Command::ORDER_BY_DESC},
    {"groupby", Command::GROUP_BY},
    {"autoscrollenable", Command::AUTO_SCROLL_ENABLE},
    {"autoscrolldisable", Command::AUTO_SCROLL_DISABLE},
    {"details", Command::DETAILS},
    {"clear", Command::CLEAR},
    {"quit", Command::QUIT},
    {"topics", Command::TOPICS},
    {"services", Command::SERVICES},
    {"hosts", Command::HOSTS},
    {"processes", Command::PROCESSES},
    {"logs", Command::LOGS},
    {"help", Command::HELP},
    {"sysinfo", Command::SYSTEM_INFORMATION},
  });
}

std::shared_ptr<CommandDescriptionMap> CreateCommandDescriptionMap()
{
  return std::make_shared<CommandDescriptionMap>(CommandDescriptionMap
  {
    {Command::FIND, "Searches current table in current view and jumps to first occurance."},
    {Command::FIND_NEXT, "Find next occurance based on find command."},
    {Command::FIND_PREVIOUS, "Finds previous occurance based on find command."},
    {Command::FILTER, "Filter data in current tab."},
    {Command::FILTER_BY, "Filter by data from specific column in current tab."},
    {Command::ORDER_BY, "Order data in current tab by a column, running this command second time on same column will reverse the order of data"},
    {Command::ORDER_BY_ASC, "Order data in current tab by a column in ascending order"},
    {Command::ORDER_BY_DESC, "Order data in current tab by a column in descending order"},
    {Command::GROUP_BY, "Group data by specific column"},
    {Command::AUTO_SCROLL_ENABLE, "Enable table autoscroll"},
    {Command::AUTO_SCROLL_DISABLE,  "Disable table autoscroll"},
    {Command::DETAILS, "Toggle details view for selected table item"},
    {Command::CLEAR, "Clears log data."},
    {Command::QUIT, "Quit the application"},
    {Command::TOPICS, "Jump to topics view."},
    {Command::SERVICES, "Jump to services view."},
    {Command::HOSTS, "Jump to hosts view."},
    {Command::PROCESSES, "Jump to processes view."},
    {Command::LOGS, "Jump to logs view."},
    {Command::HELP, "Show help view"},
    {Command::SYSTEM_INFORMATION, "Show system info view"},
  });
};
