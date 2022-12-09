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

#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"

#include "tui/view/view.hpp"
#include "tui/view/component/decorator.hpp"

#include "tui/viewmodel/processes.hpp"

class ProcessDetailsView : public View
{
  std::shared_ptr<ProcessesViewModel> view_model;

public:
  ProcessDetailsView(std::shared_ptr<ProcessesViewModel> vm)
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
      text("Process path: " + details->name),
      text("Command: " + details->params),
      separatorEmpty(),
      text("Timesync state: " + std::to_string(details->time_sync_state)),
      text("Timesync mod name: " + details->time_sync_mod_name),
      separatorEmpty(),
      text("Initialized components: " + details->component_init_info),
      separatorEmpty(),
      text("eCAL runtime: " + details->ecal_runtime_version)
    ) | indent | indentRight;
  }
};
