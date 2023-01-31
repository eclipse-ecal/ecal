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

#include <vector>
#include <string>
#include <algorithm>

#include <ftxui/component/component.hpp>

#include "tui/view/view.hpp"
#include "tui/view/topics.hpp"
#include "tui/view/services.hpp"
#include "tui/view/hosts.hpp"
#include "tui/view/processes.hpp"
#include "tui/view/logs.hpp"
#include "tui/view/system_information.hpp"
#include "tui/view/help.hpp"
#include "tui/view/command_line.hpp"

#include "tui/viewmodel/shell.hpp"

class ShellView : public View
{
  std::shared_ptr<ShellViewModel> view_model;

  std::vector<std::string> tab_titles;

  ftxui::Component tab_toggle;
  ftxui::Component tab_container;
  ftxui::Component command_line;

  std::shared_ptr<View> CreateTab(ShellViewModel::Tab &tab)
  {
    switch(tab.type)
    {
      case ShellViewModel::TabType::TOPICS:
        return CreateSubView<TopicsView>(std::dynamic_pointer_cast<TopicsViewModel>(tab.view_model));
      case ShellViewModel::TabType::SERVICES:
        return CreateSubView<ServicesView>(std::dynamic_pointer_cast<ServicesViewModel>(tab.view_model));
      case ShellViewModel::TabType::HOSTS:
        return CreateSubView<HostsView>(std::dynamic_pointer_cast<HostsViewModel>(tab.view_model));
      case ShellViewModel::TabType::PROCESSES:
        return CreateSubView<ProcessesView>(std::dynamic_pointer_cast<ProcessesViewModel>(tab.view_model));
      case ShellViewModel::TabType::LOGS:
        return CreateSubView<LogsView>(std::dynamic_pointer_cast<LogsViewModel>(tab.view_model));
      case ShellViewModel::TabType::SYSTEM_INFORMATION:
        return CreateSubView<SystemInformationView>(std::dynamic_pointer_cast<SystemInformationViewModel>(tab.view_model));
      case ShellViewModel::TabType::HELP:
        return CreateSubView<HelpView>(std::dynamic_pointer_cast<HelpViewModel>(tab.view_model));
      default:
        return nullptr;
    }
  }

  void GenerateTabs()
  {
    using namespace ftxui;

    Components tab_views;

    auto index = 1;
    for(auto &tab: view_model->tabs)
    {
      auto view = CreateTab(tab);
      tab_views.push_back(view);
      auto tab_title = std::to_string(index++) + ": " + view->ViewModel().title;
      tab_titles.push_back(tab_title);
    }

    tab_toggle = Menu(&tab_titles, &view_model->tab_selected, style->tab);
    tab_container = Container::Tab(tab_views, &view_model->tab_selected);
  }

public:
  ShellView(std::shared_ptr<ShellViewModel> view_model)
    : view_model{view_model} { }

  void Init() override
  {
    command_line = CreateSubView<CommandLineView>(view_model->command_line_view_model);

    GenerateTabs();
    Add(tab_container);
    Add(tab_toggle);
  }

  class ViewModel &ViewModel() override
  {
    return *view_model;
  }

  bool OnEvent(ftxui::Event event) override
  {
    if(view_model->command_line_view_model->mode != CommandLineViewModel::Mode::DEFAULT)
    {
      return command_line->OnEvent(event);
    }

    auto command = KeyCommand(event);
    switch(command)
    {
      case Command::NAVIGATE_TAB1:
        view_model->NavigateToTab(0);
        return true;
      case Command::NAVIGATE_TAB2:
        view_model->NavigateToTab(1);
        return true;
      case Command::NAVIGATE_TAB3:
        view_model->NavigateToTab(2);
        return true;
      case Command::NAVIGATE_TAB4:
        view_model->NavigateToTab(3);
        return true;
      case Command::NAVIGATE_TAB5:
        view_model->NavigateToTab(4);
        return true;
      case Command::NAVIGATE_TAB6:
        view_model->NavigateToTab(5);
        return true;
      case Command::NAVIGATE_TAB7:
        view_model->NavigateToTab(6);
        return true;
      case Command::START_COMMAND:
        view_model->EnterCommandMode();
        return true;
      case Command::START_FIND:
        view_model->EnterSearchMode();
        return true;
      case Command::START_FILTER:
        view_model->EnterFilterMode();
        return true;
      default: return ComponentBase::OnEvent(event);
    }
  }

  ftxui::Element Render() override
  {
    using namespace ftxui;

    return vbox(
      tab_container->Render() | flex,
      tab_toggle->Render(),
      separator(),
      command_line->Render(),
      separatorEmpty()
    );
  }
};
