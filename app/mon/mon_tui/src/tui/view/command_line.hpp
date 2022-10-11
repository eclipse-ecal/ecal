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

#include "tui/viewmodel/command_line.hpp"

class CommandLineView : public View
{
  std::shared_ptr<CommandLineViewModel> view_model;

  ftxui::Element InputIndicator()
  {
    using namespace ftxui;
    switch(view_model->mode)
    {
      case CommandLineViewModel::Mode::COMMAND:
        return text("COMMAND:");
      case CommandLineViewModel::Mode::FILTER:
        return text("FILTER:");
      case CommandLineViewModel::Mode::SEARCH:
        return text("SEARCH:");
      default:
        return emptyElement();
    }
  }

  ftxui::Element CommandPart()
  {
    using namespace ftxui;

    if(view_model->mode == CommandLineViewModel::Mode::DEFAULT)
    {
      return text(view_model->status.text)
        | style->command_line.status(view_model->status.level);
    }

    return hbox (
      InputIndicator()
        | style->command_line.mode(view_model->mode),
      text(view_model->input),
      text("|")
    );
  }
public:
  CommandLineView(std::shared_ptr<CommandLineViewModel> view_model)
    : view_model{view_model} { }

  class ViewModel &ViewModel() override
  {
    return *view_model;
  }

  bool OnEvent(ftxui::Event event) override
  {
    using namespace ftxui;

    auto command = KeyCommand(event);
    switch(command)
    {
      case Command::ACTION:
        view_model->PerformCommand();
        return true;
      case Command::CANCEL:
        view_model->ResetInputMode();
        return true;
      default: break;
    }
    if(event.is_character())
    {
      view_model->AddToInput(event.input());
      return true;
    }
    if(event == Event::Backspace)
    {
      view_model->DecrementInput();
      return true;
    }

    return false;
  }

  virtual bool Focusable() const override
  {
    return true;
  }

  ftxui::Element Render() override
  {
    using namespace ftxui;

    return hbox(
      separatorEmpty(),
      CommandPart() | flex,
      separatorEmpty(),
      separatorCharacter("|"),
      separatorEmpty(),
      text("Time: " + view_model->current_ecal_time),
      separatorEmpty(),
      separatorCharacter("|"),
      separatorEmpty(),
      text("Network: " + view_model->network_mode),
      separatorEmpty()
    );
  }
};
