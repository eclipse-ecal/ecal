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

#include "ftxui/component/component_base.hpp"
#include "tui/view/table.hpp"
#include "tui/view/service_details.hpp"

#include "tui/viewmodel/services.hpp"

#include "model/data/service.hpp"

class ServicesView : public TableView<Service>
{
  std::shared_ptr<ServicesViewModel> vm;

  ftxui::Component details;
public:
  ServicesView(std::shared_ptr<ServicesViewModel> vm_)
    : TableView<Service>(vm_), vm{vm_} { }

  void Init() override
  {
    details = view_factory->Create<ServiceDetailsView>(vm);

    TableView::Init();
  }

  ftxui::Component DetailsView() override
  {
    return details;
  }
};
