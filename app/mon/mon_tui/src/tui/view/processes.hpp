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

#include "model/data/process.hpp"
#include "model/data/service.hpp"

#include "tui/view/table.hpp"
#include "tui/view/process_details.hpp"

#include "tui/viewmodel/processes.hpp"

class ProcessesView : public TableView<Process>
{
  std::shared_ptr<ProcessesViewModel> vm;

  ftxui::Component details;

  class ItemRenderer : public TableView<Process>::ItemRenderer
  {
  public:
    ItemRenderer(ftxui::TypedTableModel<Process> model,
        std::shared_ptr<StyleSheet> style)
      : TableView<Process>::ItemRenderer(model, style)
    {}

    virtual ftxui::Element RenderCell(const Process &data, int column)
    {
      using namespace ftxui;
      Decorator decorator = nothing;
      if(column == ProcessesViewModel::Column::State)
      {
        decorator = style->process_table_severity(data.severity);
      }
      return TableView<Process>::ItemRenderer::RenderCell(data, column)
          | decorator;
    }
  };
public:
  ProcessesView(std::shared_ptr<ProcessesViewModel> vm_)
    : TableView<Process>(vm_), vm{vm_} { }

  virtual std::shared_ptr<TableView<Process>::ItemRenderer> CreateRenderer() override
  {
    return std::make_shared<ItemRenderer>(view_model, style);
  }

  void Init() override
  {
    details = view_factory->Create<ProcessDetailsView>(vm);

    TableView::Init();
  }

  ftxui::Component DetailsView() override
  {
    return details;
  }
};
