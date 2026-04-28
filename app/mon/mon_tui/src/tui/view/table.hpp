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

#include <string>
#include <vector>

#include <ftxui/dom/table.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>

#include "component/data_table.hpp"

#include "ftxui/dom/node.hpp"
#include "ftxui/screen/color.hpp"
#include "tui/view/view.hpp"
#include "tui/view/component/scroller.hpp"
#include "tui/view/component/focus_manager.hpp"
#include "tui/view/component/data_table.hpp"

#include "tui/viewmodel/table.hpp"

#include "tui/ftxui_version_compatibility.hpp"

template<typename T>
class TableView : public View
{
  ftxui::DataTable table;
  ftxui::Component details;
  ftxui::Component split_view;
  int split_left_width = 80;

protected:
  std::shared_ptr<TableViewModel<T>> view_model;

public:
  class ItemRenderer : public ftxui::TypedTableRendererBase<T>
  {
  protected:
    std::shared_ptr<StyleSheet> style;

  public:
    ItemRenderer(ftxui::TypedTableModel<T> model,
        std::shared_ptr<StyleSheet> style_)
      : ftxui::TypedTableRendererBase<T>(model), style{style_}
    {}

    virtual ftxui::Element RenderColumn(const std::string &val, int index) override
    {
      using namespace ftxui;
      return TypedTableRendererBase<T>::RenderColumn(val, index)
        | style->table.header;
    }

    virtual ftxui::Element RenderGroupRow(const std::string &val) override
    {
      using namespace ftxui;
      return TypedTableRendererBase<T>::RenderGroupRow(val)
        | style->table.group_row;
    }
  };

  TableView(std::shared_ptr<TableViewModel<T>> vm)
    : view_model{vm} { }

  virtual void Init() override
  {
    using namespace ftxui;

    auto renderer = CreateRenderer();
    table = std::make_shared<ftxui::DataTableBase>(view_model, renderer);
    Components focusable;
    focusable.push_back(table);

    details = DetailsView();

    if(details == nullptr)
    {
      details = Renderer([]{
        return emptyElement();
      });
    }
    else
    {
      details = Maybe(details, &view_model->details_visible);
      focusable.push_back(details);
    }

    split_view = ftxui::ResizableSplitLeft(table, details, &split_left_width);

    Add(std::make_shared<FocusManager>(focusable, nullptr));
  }

  bool OnEvent(ftxui::Event event) override
  {
    if (event.is_mouse() && view_model->details_visible && split_view != nullptr && split_view->OnEvent(event))
      return true;

    if(ComponentBase::OnEvent(event)) return true;

    auto command = KeyCommand(event);
    switch(command)
    {
      case Command::FIND_NEXT:
        view_model->FindNext();
        return true;
      case Command::FIND_PREVIOUS:
        view_model->FindPrev();
        return true;
      case Command::ACTION:
        if(!details->Focused()) view_model->ShowDetails();
        return true;
      case Command::CANCEL:
        view_model->HideDetails();
        return true;
      default:
        return false;
    }
  }

  virtual class ViewModel& ViewModel() override
  {
    return *view_model;
  }

  virtual ftxui::Component DetailsView()
  {
    return nullptr;
  }

  ftxui::Element FTXUI_COMPATIBILITY_RENDER() override
  {
    using namespace ftxui;

    if (!view_model->details_visible)
    {
      return hbox(
        separatorEmpty(),
        table->Render() | flex
      );
    }

    return split_view->Render();
  }

  virtual std::shared_ptr<ItemRenderer> CreateRenderer()
  {
    return std::make_shared<ItemRenderer>(view_model, style);
  }
};
