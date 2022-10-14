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
#include <functional>

#include "model/monitor.hpp"
#include "model/log.hpp"

#include "tui/event_loop.hpp"
#include "tui/command_manager.hpp"
#include "tui/notification_manager.hpp"

#include "utils/string.hpp"
#include "utils/container.hpp"

#include "tui/viewmodel/viewmodel.hpp"
#include "tui/viewmodel/command_line.hpp"
#include "tui/viewmodel/topics.hpp"
#include "tui/viewmodel/services.hpp"
#include "tui/viewmodel/hosts.hpp"
#include "tui/viewmodel/processes.hpp"
#include "tui/viewmodel/logs.hpp"
#include "tui/viewmodel/system_information.hpp"
#include "tui/viewmodel/help.hpp"

class ShellViewModel : public ViewModel
{
public:
  enum class TabType
  {
    TOPICS,
    SERVICES,
    PROCESSES,
    HOSTS,
    LOGS,
    SYSTEM_INFORMATION,
    HELP
  };

  struct Tab
  {
    TabType type;
    std::shared_ptr<ViewModel> view_model;
  };

private:
  std::shared_ptr<ViewModel> GetCurrentTab()
  {
    return tabs[tab_selected].view_model;
  }

  void PropagateCommandToCurrentTab(const CommandDetails &command)
  {
    GetCurrentTab()->OnCommand(command);
  }

public:
  int tab_selected = 0;
  std::vector<Tab> tabs;
  std::shared_ptr<CommandLineViewModel> command_line_view_model;
  std::function<void()> init_func;

  ShellViewModel(std::shared_ptr<MonitorModel> model,
    std::shared_ptr<LogModel> logs_model,
    std::shared_ptr<CommandManager> command_manager)
  {
    init_func = [this, model, logs_model, command_manager] {
      using namespace std::placeholders;
      tabs = {
        { TabType::TOPICS, CreateViewModel<TopicsViewModel>(model) },
        { TabType::SERVICES, CreateViewModel<ServicesViewModel>(model) },
        { TabType::HOSTS, CreateViewModel<HostsViewModel>(model) },
        { TabType::PROCESSES, CreateViewModel<ProcessesViewModel>(model) },
        { TabType::LOGS, CreateViewModel<LogsViewModel>(logs_model) },
        { TabType::SYSTEM_INFORMATION, CreateViewModel<SystemInformationViewModel>() },
        { TabType::HELP, CreateViewModel<HelpViewModel>(command_manager) }
      };
      command_line_view_model = CreateViewModel<CommandLineViewModel>(command_manager);
      command_line_view_model->SetOnCommandCallback(std::bind(&ShellViewModel::OnCommand, this, _1));
    };
  }

  void Init() final override
  {
    init_func();
  }

  void OnCommand(const CommandDetails &command) final override
  {
    switch(command.command)
    {
      case Command::TOPICS:
        tab_selected = 0;
        break;
      case Command::SERVICES:
        tab_selected = 1;
        break;
      case Command::HOSTS:
        tab_selected = 2;
        break;
      case Command::PROCESSES:
        tab_selected = 3;
        break;
      case Command::LOGS:
        tab_selected = 4;
        break;
      case Command::SYSTEM_INFORMATION:
        tab_selected = 5;
        break;
      case Command::HELP:
        tab_selected = 6;
        break;
      case Command::QUIT:
        StopApplication();
        break;
      default:
        PropagateCommandToCurrentTab(command);
    }
  }

  void NavigateToTab(size_t index)
  {
    if(index < tabs.size())
    {
      tab_selected = static_cast<int>(index);
    }
  }

  void EnterCommandMode()
  {
    command_line_view_model->EnterCommandMode();
  }

  void EnterSearchMode()
  {
    command_line_view_model->EnterSearchMode();
  }

  void EnterFilterMode()
  {
    command_line_view_model->EnterFilterMode();
  }

};
