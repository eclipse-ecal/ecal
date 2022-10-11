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

#include <initializer_list>
#include <vector>

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>

#include "tui/view/view.hpp"
#include "tui/view/component/scroller.hpp"
#include "tui/view/component/decorator.hpp"

#include "tui/viewmodel/help.hpp"

#include "ecal_mon_tui_defs.h"

class HelpView : public View
{
  std::shared_ptr<HelpViewModel> view_model;

  ftxui::Component view;

  ftxui::Element GenerateCommands()
  {
    using namespace ftxui;
    std::vector<std::vector<Element>> data;

    for(auto &cmd: view_model->commands)
    {
      data.emplace_back(std::initializer_list<Element> {
        text(std::get<0>(cmd)),
        text(" " + std::get<1>(cmd))
      });
    }
    return vbox(
      text("You can invoke commands by pressing \":\" and typing them out."),
      separator(),
      text("Available commands:"),
      Table(data).Render()
    );
  }

  ftxui::Element GenerateKeybinds()
  {
    using namespace ftxui;
    std::vector<std::vector<Element>> keybinds;
    auto add_row = [&keybinds] (const std::string &val,
                                const std::string &desc) {
      keybinds.push_back(std::initializer_list<Element> { text(val), text(desc) });
    };
    add_row("arrows/hjkl  ", "Move around current view");
    add_row("tab", "Navigate between views, ex. between table and details");
    add_row("shift+tab", "Reverse navigate between views");
    add_row("1...7", "Switch between tabs");
    add_row(":", "Enter command mode");
    add_row("/", "Enter search mode");
    add_row("\\", "Enter filter mode");
    add_row("return", "Perform context specific action ex. open details view when in table or expand node when browsing tree views");
    add_row("esc", "Cancel");
    return vbox(
      text("Available keybinds:"),
      Table(keybinds).Render()
    );
  }

public:
  HelpView(std::shared_ptr<HelpViewModel> vm)
    : view_model{vm} { }

  void Init() override
  {
    using namespace ftxui;
    auto commands = GenerateCommands();
    auto keybinds = GenerateKeybinds();
    view = Scroller(Renderer([commands, keybinds]{
      return vbox(
        text(ECAL_MON_TUI_NAME " v" ECAL_MON_TUI_VERSION),
        separatorEmpty(),
        commands,
        separatorEmpty(),
        keybinds
      ) | indent;
    }));
    Add(view);
  }

  class ViewModel& ViewModel() override
  {
    return *view_model;
  }

  ftxui::Element Render() override
  {
    using namespace ftxui;
    return view->Render();
  }
};
