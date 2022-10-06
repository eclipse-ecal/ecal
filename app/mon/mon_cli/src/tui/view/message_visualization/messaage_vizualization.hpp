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

#include "tui/view/view.hpp"
#include "tui/view/component/scroller.hpp"

#include "tui/viewmodel/message_visualization/message_visualization.hpp"

class MessageVisualizationView : public View
{
  std::shared_ptr<MessageVisualizationViewModel> view_model;

protected:
  virtual ftxui::Component DataView() = 0;

public:
  MessageVisualizationView(std::shared_ptr<MessageVisualizationViewModel> vm)
    : view_model{vm} { }

  class ViewModel& ViewModel() override
  {
    return *view_model;
  }

  ftxui::Element Render() override
  {
    using namespace ftxui;

    if(view_model->message_timestamp == 0)
      return text("--- No messages recieved, yet ---");

    return vbox(
      text("Timestamp: " + std::to_string(view_model->message_timestamp)),
      separatorEmpty(),
      DataView()->Render()
    );
  }
};
