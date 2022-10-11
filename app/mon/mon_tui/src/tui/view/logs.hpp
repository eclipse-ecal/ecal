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

#include "model/log.hpp"

#include "tui/view/table.hpp"
#include "tui/view/log_details.hpp"

#include "tui/viewmodel/logs.hpp"

class LogsView : public TableView<LogEntry>
{
  std::shared_ptr<LogsViewModel> vm;

  ftxui::Component details;

  class ItemRenderer : public TableView<LogEntry>::ItemRenderer
  {
  public:
    ItemRenderer(ftxui::TypedTableModel<LogEntry> model,
        std::shared_ptr<StyleSheet> style)
      : TableView<LogEntry>::ItemRenderer(model, style)
    {}

    virtual ftxui::Element RenderCell(const LogEntry &data, int column)
    {
      using namespace ftxui;
      Decorator decorator = nothing;
      if(column == LogsViewModel::Column::Level)
      {
        decorator = style->log_level_style(data.log_level);
      }
      return TableView<LogEntry>::ItemRenderer::RenderCell(data, column)
          | decorator;
    }
  };

public:
  LogsView(std::shared_ptr<LogsViewModel> vm_)
    : TableView(vm_), vm{vm_} { }

  virtual std::shared_ptr<TableView<LogEntry>::ItemRenderer> CreateRenderer() override
  {
    return std::make_shared<ItemRenderer>(view_model, style);
  }

  void Init() override
  {
    details = view_factory->Create<LogDetailsView>(vm);

    TableView::Init();
  }

  ftxui::Component DetailsView() override
  {
    return details;
  }
};
