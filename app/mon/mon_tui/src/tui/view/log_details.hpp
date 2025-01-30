/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"

#include "tui/view/component/non_wrapping_parameter.hpp"
#include "tui/view/view.hpp"
#include "tui/view/component/decorator.hpp"

#include "tui/viewmodel/logs.hpp"

class LogDetailsView : public View
{
  std::shared_ptr<LogsViewModel> view_model;

public:
  LogDetailsView(std::shared_ptr<LogsViewModel> vm)
    : View(), view_model{vm} {}

  class ViewModel &ViewModel() override
  {
    return *view_model;
  }

  ftxui::Element Render() override
  {
    using namespace ftxui;

    auto details = view_model->SelectedItem();
    if(details == nullptr) return emptyElement();

    return vbox (
      separatorEmpty(),
      text("Host name: " + details->host_name),
      text("PID: " + std::to_string(details->process_id)),
      text("Process path: " + details->process_path),
      separatorEmpty(),
      text("Message:"),
      nonWrappingParagraph(details->message)
    ) | indent | indentRight;
  }
};
