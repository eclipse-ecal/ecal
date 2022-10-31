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
#include <string>
#include <vector>

#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/dom/table.hpp"
#include "tui/view/component/scroller.hpp"

#include "tui/view/view.hpp"
#include "tui/view/component/decorator.hpp"

#include "tui/viewmodel/services.hpp"

class ServiceDetailsView : public View
{
  std::shared_ptr<ServicesViewModel> view_model;

  ftxui::Component methods = ftxui::Scroller(ftxui::Renderer([this]{
    using namespace ftxui;
    auto details = view_model->SelectedItem();
    if(details == nullptr) return emptyElement();

    std::vector<std::vector<std::string>> methods_vec;
    methods_vec.emplace_back(std::initializer_list<std::string>
      {"Name", "Request", "Response", "CallCount"}
    );
    for(auto &m: details->methods)
    {
      auto &method = methods_vec.emplace_back();
      method.push_back(m.name);
      method.push_back(m.request_type);
      method.push_back(m.response_type);
      method.push_back(std::to_string(m.call_count));
    }

    auto tbl = Table(methods_vec);
    tbl.SelectAll().SeparatorVertical(ftxui::EMPTY);
    tbl.SelectRow(0).Decorate(style->table.header);
    return tbl.Render();
  }));

  ftxui::Element Info()
  {
    using namespace ftxui;
    auto details = view_model->SelectedItem();
    if(details == nullptr) return emptyElement();

    return vbox (
      separatorEmpty(),
      text("Process path:" + details->process_name)
    );
  }

public:
  ServiceDetailsView(std::shared_ptr<ServicesViewModel> vm)
    : View(), view_model{vm} {}

  void Init() override
  {
    Add(methods);
  }

  class ViewModel &ViewModel() override
  {
    return *view_model;
  }

  ftxui::Element Render() override
  {
    using namespace ftxui;

    return vbox(
      Info(),
      separatorEmpty(),
      text("Methods:"),
      separatorEmpty(),
      methods->Render()
    ) | indent | indentRight;
  }
};
