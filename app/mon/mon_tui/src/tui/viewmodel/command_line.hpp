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

#include <atomic>
#include <ecal/ecal_log.h>
#include <string>
#include <vector>
#include <functional>
#include <thread>

#include <ecal/ecal.h>

#include "utils/string.hpp"
#include "utils/container.hpp"

#include "tui/command_manager.hpp"
#include "tui/event_loop.hpp"
#include "tui/notification_manager.hpp"

#include "tui/viewmodel/viewmodel.hpp"

using OnCommandCallback = std::function<void(CommandDetails)>;

class CommandLineViewModel : public ViewModel
{
public:
  enum class Mode
  {
    DEFAULT, COMMAND, SEARCH, FILTER
  };

private:
  const Status DEFAULT_STATUS_VALUE
  {
    "Press \":\" to input command, \"/\" to search data or \"\\\" to filter data, for list of all possible commands use :help command",
    StatusLevel::DEFAULT
  };

  std::shared_ptr<CommandManager> command_manager;
  OnCommandCallback on_command;

  void PerformCommand(const CommandDetails &command)
  {
    if(on_command) on_command(command);
  }

  void ResetStatus()
  {
    status = DEFAULT_STATUS_VALUE;
  }

  void UpdateMode(Mode new_mode)
  {
    if(new_mode != mode)
    {
      mode = new_mode;
      input = "";
      NotifyDataUpdated();
    }
  }

  void OnNotification(const Status &status_)
  {
    status = status_;
  }

  void OnNotificationExpired()
  {
    ResetStatus();
  }

  void UpdateCurrentEcalTime()
  {
    while(is_polling_time)
    {
      current_ecal_time = EcalTimestampToString(eCAL::Time::GetMicroSeconds());
      NotifyDataUpdated();

      eCAL::Process::SleepMS(1000);
    }
  }

public:
  Mode mode = Mode::DEFAULT;
  std::string input;
  Status status = DEFAULT_STATUS_VALUE;

  std::string network_mode;

  std::atomic_bool is_polling_time;
  std::thread time_updater;
  std::string current_ecal_time;

  CommandLineViewModel(std::shared_ptr<CommandManager> command_manager_)
    : command_manager{command_manager_} {}

  void Init() override
  {
    using namespace std::placeholders;

    network_mode = eCAL::Config::IsNetworkEnabled() ? "cloud" : "local";
    is_polling_time = true;
    time_updater = std::thread(std::bind(&CommandLineViewModel::UpdateCurrentEcalTime, this));

    notification_manager->SetNotifyCallback(std::bind(&CommandLineViewModel::OnNotification, this, _1));
    notification_manager->SetExpiredCallback(std::bind(&CommandLineViewModel::OnNotificationExpired, this));
  }

  void ResetInputMode()
  {
    UpdateMode(Mode::DEFAULT);
  }

  void EnterCommandMode()
  {
    UpdateMode(Mode::COMMAND);
  }

  void EnterSearchMode()
  {
    UpdateMode(Mode::SEARCH);
  }

  void EnterFilterMode()
  {
    UpdateMode(Mode::FILTER);
  }

  void AddToInput(const std::string &data)
  {
    if(mode != Mode::DEFAULT)
    {
      input += data;
    }
  }

  void DecrementInput()
  {
    if(input.size() == 0)
    {
      ResetInputMode();
    }
    else
    {
      input = input.substr(0, input.size() - 1);
    }
  }

  void PerformCommand()
  {
    CommandDetails command;
    switch(mode)
    {
      case Mode::SEARCH:
        command = command_manager->ParseCommand("find " + input);
        break;
      case Mode::FILTER:
        command = command_manager->ParseCommand("filter " + input);
        break;
      default:
        command = command_manager->ParseCommand(input);
        break;
    }
    if(command.command == Command::UNSUPPORTED)
    {
      NotifyUser(StatusLevel::ERROR, "Unsupported command.");
    }
    else
    {
      PerformCommand(command);
    }
    ResetInputMode();
  }

  void SetOnCommandCallback(OnCommandCallback callback)
  {
    on_command = callback;
  }

  ~CommandLineViewModel()
  {
    is_polling_time = false;
    time_updater.join();
  }
};
