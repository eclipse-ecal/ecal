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
#include <sstream>
#include <iostream>

#include <ftxui/component/component_base.hpp>
#include <ftxui/dom/elements.hpp>

#include "tui/view/view.hpp"
#include "tui/view/component/scroller.hpp"
#include "tui/view/component/non_wrapping_parameter.hpp"
#include "tui/view/component/decorator.hpp"

#include "tui/viewmodel/system_information.hpp"

class SystemInformationView : public View
{
  std::shared_ptr<SystemInformationViewModel> view_model;

  ftxui::Component view;
public:
  SystemInformationView(std::shared_ptr<SystemInformationViewModel> vm)
    : view_model{vm} { }

  void Init() override
  {
    using namespace ftxui;
    auto element = nonWrappingParagraph(view_model->data);
    view = Scroller(Renderer([element]{
      return element | indent;
    }));

    Add(view);
  }

  class ViewModel& ViewModel() override
  {
    return *view_model;
  }

  ftxui::Element Render() override
  {
    return view->Render();
  }
};
