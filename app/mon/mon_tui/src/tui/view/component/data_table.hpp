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

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <mutex>
#include <numeric>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "ftxui/component/component_base.hpp"
#include "ftxui/component/event.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/dom/node.hpp"
#include "scroller.hpp"

#include <ftxui/component/component.hpp>

namespace ftxui
{

class TableModelBase
{
  friend class DataTableBase;

  using DataUpdatedCallback = std::function<void()>;
public:
  enum class SortDirection { ASC, DESC };
  const static int NO_COLUMN = -1;

protected:
  int selected = 0;
  std::vector<std::string> columns;
  DataUpdatedCallback on_data_updated;

  SortDirection InvertSortDirection(SortDirection direction)
  {
    switch (direction)
    {
      case SortDirection::ASC:
        return SortDirection::DESC;
      case SortDirection::DESC:
        return SortDirection::ASC;
      default:
        return SortDirection::DESC;
    }
  }

public:
  int sorted_column = NO_COLUMN;
  SortDirection sort_direction = SortDirection::DESC;

  std::string filter = "";
  int filter_column = NO_COLUMN;

  int group_column = NO_COLUMN;

  virtual std::vector<std::string>& Columns()
  {
    return columns;
  }

  virtual void SetColumns(std::vector<std::string> columns_)
  {
    columns = columns_;
  }

  virtual const std::string& ColumnName(int column)
  {
    return columns[column];
  }

  virtual void SetSelectedIndex(int index)
  {
    selected = index;
  }

  virtual int SelectedIndex() const
  {
    return selected;
  }

  virtual void NotifyDataUpdated()
  {
    if(on_data_updated) on_data_updated();
  }

  virtual void SetDataUpdatedCallback(DataUpdatedCallback callback)
  {
    on_data_updated = callback;
  }

  virtual void Sort(int column)
  {
    SortDirection direction = SortDirection::ASC;
    if(column == sorted_column)
    {
      direction = InvertSortDirection(sort_direction);
    }
    Sort(column, direction);
  }

  virtual int FindColumnByName(const std::string &name)
  {
    for(size_t i = 0; i < columns.size(); i++)
    {
      if(columns[i] == name)
      {
        return static_cast<int>(i);
      }
    }

    return NO_COLUMN;
  }

  virtual bool IsFiltered() const
  {
    return filter != "";
  }

  virtual bool IsGrouped() const
  {
    return group_column != NO_COLUMN;
  }

  virtual int GroupByColumn() const
  {
    return group_column;
  }

  virtual int FilterColumn() const
  {
    return filter_column;
  }

  virtual void Sort(int column, SortDirection direction) = 0;
  virtual void Filter(const std::string &value, int column = NO_COLUMN) = 0;
  virtual void ClearFilter() = 0;
  virtual void GroupBy(int column) = 0;
  virtual void ClearGroupBy() = 0;
  virtual std::string StringRepresentation(int column, int row) = 0;
  virtual size_t RowCount() const = 0;
  virtual bool ToggleGroupExpanded() = 0;
  virtual ~TableModelBase() {}
};

template<typename T>
class TypedTableModelBase : public TableModelBase
{
  struct Row
  {
    T data;
    std::vector<std::string> printable_values;

    Row(const T &&data_) : data{std::move(data_)} {}
    Row() = default;
  };

  struct Group
  {
    std::string value;
    bool is_expanded;
    std::vector<std::reference_wrapper<Row>> rows;

    Group() = default;
  };

  void GenerateStringReperesentations()
  {
    for(int row_i = 0; row_i < static_cast<int>(data.size()); row_i++)
    {
      auto &d = data[row_i];
      d.printable_values.resize(columns.size());
      for(int column_i = 0; column_i < static_cast<int>(columns.size()); column_i++)
      {
        d.printable_values[column_i] = StringRepresentation(column_i, row_i);
      }
    }
  }

  void ApplyGroupBy(int column)
  {
    bool new_group = column != group_column;
    group_column = column;

    std::unordered_set<std::string> non_expanded_groups;
    if(!new_group)
    {
      for(auto &group: visible_data)
      {
        if(!group.is_expanded) non_expanded_groups.insert(group.value);
      }
    }

    auto is_expanded = [&non_expanded_groups](const std::string &val) {
      return non_expanded_groups.find(val) == non_expanded_groups.end();
    };

    visible_data.clear();
    if(group_column == NO_COLUMN)
    {
      auto &default_group = visible_data.emplace_back();
      default_group.is_expanded = true;
      std::copy(data.begin(), data.end(), std::back_inserter(default_group.rows));
      return;
    }

    std::unordered_map<std::string, std::reference_wrapper<Group>> groups;
    for(auto &row: data)
    {
      auto &group_val = row.printable_values[group_column];
      auto it = groups.find(group_val);
      if(it == groups.end())
      {
        auto &group = visible_data.emplace_back();
        group.value = group_val;
        group.is_expanded = is_expanded(group_val);
        it = groups.insert({group_val, group}).first;
      }
      it->second.get().rows.push_back(std::ref(row));
    }
  }

  void ApplyFilter()
  {
    std::function<bool(Row&)> filter_func;
    if(filter_column == NO_COLUMN)
    {
      filter_func = [&filter=filter](auto &row) {
        for(auto &val: row.printable_values)
        {
          if(val.find(filter) != std::string::npos)
            return false;
        }
        return true;
      };
    }
    else
    {
      filter_func = [filter_column=filter_column, &filter=filter](auto &row) {
        if(row.printable_values[filter_column].find(filter) != std::string::npos)
          return false;
        else return true;
      };
    }

    //Delete groups that are empty after filtering
    visible_data.erase(std::remove_if(visible_data.begin(), visible_data.end(),[filter_func](auto &group){
      group.rows.erase(std::remove_if(group.rows.begin(), group.rows.end(), filter_func), group.rows.end());
      return group.rows.size() == 0;
    }), visible_data.end());
  }

  void ApplySort()
  {
    if(sorted_column == NO_COLUMN) return;

    if(sorted_column == group_column)
    {
      SortGroups();
    }
    else
    {
      SortRows();
    }
  }

  void SortRows()
  {
    std::function<bool(Row&, Row&)> sort_fun = [sorted_column=sorted_column](auto &l, auto &r) {
      return l.printable_values[sorted_column] < r.printable_values[sorted_column];
    };

    if(sort_direction == TableModelBase::SortDirection::DESC)
    {
      sort_fun = [sort_fun](auto& ls, auto &rs) {
        return !sort_fun(ls, rs);
      };
    }

    for(auto &group: visible_data)
    {
      std::sort(group.rows.begin(), group.rows.end(), sort_fun);
    }
  }

  void SortGroups()
  {
    std::function<bool(Group&, Group&)> sort_fun = [](auto &l, auto &r) {
      return l.value < r.value;
    };

    if(sort_direction == TableModelBase::SortDirection::DESC)
    {
      sort_fun = [sort_fun](auto& ls, auto &rs) {
        return !sort_fun(ls, rs);
      };
    }

    std::sort(visible_data.begin(), visible_data.end(), sort_fun);
  }

protected:
  std::vector<Row> data;

public:
  std::vector<Group> visible_data;

  TypedTableModelBase()
  {
    UpdateData({});
  }

  virtual std::string StringRepresentation(int column, int row) override
  {
    return StringRepresentation(column, data[row].data);
  }

  virtual void UpdateData(std::vector<T> &&data_)
  {
    data.clear();
    for(auto &d: data_)
    {
      data.emplace_back(std::move(d));
    }
    GenerateStringReperesentations();

    ApplyGroupBy(group_column);
    ApplyFilter();
    ApplySort();

    NotifyDataUpdated();
  }

  virtual T* SelectedItem()
  {
    auto selected_index = SelectedIndex();
    auto current_index = group_column == NO_COLUMN ? -1 : 0;
    for(auto &group: visible_data)
    {
      if(current_index == selected_index)
        return &group.rows[0].get().data;
      current_index++;

      for(auto row: group.rows)
      {
        if(current_index == selected_index)
          return &row.get().data;

        current_index++;
      }
    }
    return nullptr;
  }

  virtual bool ToggleGroupExpanded() override
  {
    if(group_column == NO_COLUMN) return false;

    size_t selected_index = SelectedIndex();
    size_t current_index = 0;
    for(auto &group: visible_data)
    {
       if(selected_index == current_index)
       {
         group.is_expanded = !group.is_expanded;
         NotifyDataUpdated();
         return true;
       }
       if(selected_index < current_index) return false;
       current_index += group.rows.size() + 1;
    }

    return false;
  }

  virtual size_t RowCount() const override
  {
    if(group_column == NO_COLUMN)
    {
      return visible_data[0].rows.size();
    }

    return std::accumulate(visible_data.begin(), visible_data.end(), 0,
    [](auto val, auto &group) {
      return group.rows.size() + val + 1;
    });
  }

  virtual void ClearGroupBy() override
  {
    GroupBy(NO_COLUMN);
  }

  virtual void GroupBy(int column) override
  {
    ApplyGroupBy(column);
    ApplyFilter();
    ApplySort();

    NotifyDataUpdated();
  }

  virtual void Sort(int column, SortDirection direction) override
  {
    sorted_column = column;
    sort_direction = direction;

    ApplySort();

    NotifyDataUpdated();
  }

  virtual void Filter(const std::string &filter_, int column = NO_COLUMN) override
  {
    if(filter_ == filter && column == filter_column)
    {
      return;
    }

    filter = filter_;
    filter_column = column;

    ApplyGroupBy(group_column);
    ApplyFilter();
    ApplySort();

    NotifyDataUpdated();
  }

  virtual void ClearFilter() override
  {
    Filter("", NO_COLUMN);
  }

  auto FindPredicate(const std::string &find)
  {
    return [&](auto row) {
      for(auto &s: row.get().printable_values)
      {
        if(s.find(find) != std::string::npos)
        {
          return true;
        }
      }
      return false;
    };
  }

  void SelecteNext(const std::string &find)
  {
    auto find_predicate = FindPredicate(find);
    auto selected_index = SelectedIndex();
    auto current_index = group_column == NO_COLUMN ? -1 : 0;

    auto loop_back_index = -1;

    auto current_found = false;

    for(auto &group: visible_data)
    {
      if(current_index == selected_index)
        current_found = true;

      current_index++;

      for(auto row: group.rows)
      {
        if(current_index == selected_index)
        {
          current_found = true;
          current_index++;
          continue;
        }
        if(!current_found && loop_back_index == -1 && find_predicate(row))
          loop_back_index = current_index;

        if(current_found && find_predicate(row))
        {
          selected = current_index;
          return;
        }

        current_index++;
      }
    }
    if(loop_back_index != -1) selected = loop_back_index;
  }

  void SelectPrev(const std::string &find)
  {
    auto find_predicate = FindPredicate(find);
    auto selected_index = SelectedIndex();
    auto loop_back_index = -1;
    int  current_index = static_cast<int>(RowCount());
    auto current_found = false;
    for(size_t i = visible_data.size() - 1; i >= 0; i--)
    {
      current_index--;
      auto &rows = visible_data[i].rows;
      for(size_t j = rows.size() - 1; j >= 0; j--)
      {
        auto &row = rows[j];
        if(current_index == selected_index)
        {
          current_found = true;
          current_index--;
          continue;
        }
        if(!current_found && loop_back_index == -1 && find_predicate(row))
          loop_back_index = current_index;

        if(current_found && find_predicate(row))
        {
          selected = current_index;
          return;
        }
        current_index--;
      }
    }
    if(loop_back_index != -1) selected = loop_back_index;
  }

  virtual std::string StringRepresentation(int column, const T& value) = 0;
};

template<typename T>
using TypedTableModel = std::shared_ptr<TypedTableModelBase<T>>;

class TableRendererBase
{
public:
  struct RenderedParts
  {
    Element column_part;
    Element data_part;
  };

  virtual Element RenderColumnSeparator()
  {
    return separatorEmpty();
  }

  virtual Element RenderSortMarker(TableModelBase::SortDirection direction)
  {
    switch (direction)
    {
      case TableModelBase::SortDirection::ASC:
        return text(" ▴");
      case TableModelBase::SortDirection::DESC:
        return text(" ▾");
      default:
        return text(" ?");
    }
  }

  virtual Element RenderGroupExpandedMarker(bool is_expanded)
  {
    if(is_expanded) return text("▾ ");

    return text("▸ ");
  }

  virtual RenderedParts Render() = 0;
  virtual Element RenderGroupRow(const std::string &group_value)
  {
    return text(group_value);
  }
  virtual Element RenderColumn(int row) = 0;
  virtual ~TableRendererBase() {}
};

template<typename T>
class TypedTableRendererBase : public TableRendererBase
{
  TypedTableModel<T> model;

  bool IsSortedByColumn(int column)
  {
    return model->sorted_column == column;
  }

  Elements AcumulateColumns(std::vector<int> &column_sizes)
  {
    Elements column_elements;
    const int columns_size = static_cast<int>(model->Columns().size());
    for(int i = 0; i < columns_size; i++)
    {
      auto column = RenderColumn(i);
      if(IsSortedByColumn(i))
      {
        column = hbox(column, RenderSortMarker(model->sort_direction));
      }
      column->ComputeRequirement();
      column_sizes[i] = column->requirement().min_x;
      column_elements.push_back(column);
    }
    return column_elements;
  }

  std::vector<Elements> AcumulateRows(std::vector<int> &column_widths)
  {
    std::vector<Elements> rows;
    for(auto &group: model->visible_data)
    {
      if(model->group_column != TableModelBase::NO_COLUMN)
      {
        rows.emplace_back().push_back(
          hbox(
            RenderGroupExpandedMarker(group.is_expanded),
            RenderGroupRow(group.value)
          )
        );
      }
      if(group.is_expanded)
      {
        for(auto &row: group.rows)
        {
          auto &cells = rows.emplace_back();
          for(size_t column_i = 0; column_i < model->Columns().size(); column_i++)
          {
            auto cell = RenderCell(row.get().data, static_cast<int>(column_i));
            cells.push_back(cell);

            cell->ComputeRequirement();
            auto req = cell->requirement();
            auto width = req.min_x;
            if(column_widths[column_i] < width)
            {
              column_widths[column_i] = width;
            }
          }
        }
      }
    }

    return rows;
  }

  Element RenderColumnPart(Elements &columns, std::vector<int> &column_sizes)
  {
    auto separator = RenderColumnSeparator();
    for(size_t i = 0; i < columns.size(); i++)
    {
      columns[i] = hbox(
        columns[i]
          | size(WIDTH, EQUAL, column_sizes[i]),
        separator
      );
    }

    return hbox(columns);
  }

  Element RenderDataPart(std::vector<Elements> &rows, std::vector<int> &column_sizes)
  {
    auto separator = RenderColumnSeparator();
    Elements row_elements;
    for(size_t row_i = 0; row_i < rows.size(); row_i++)
    {
      Elements row_element;
      auto column_count = rows[row_i].size();
      if(column_count == 1)
      {
        row_elements.push_back(rows[row_i][0]);
      }
      else
      {
        for(size_t column_i = 0; column_i < column_count; column_i++)
        {
          row_element.emplace_back(
            hbox(rows[row_i][column_i]
                  | size(WIDTH, EQUAL, column_sizes[column_i]),
                 separator
            )
          );
        }
        row_elements.push_back(hbox(row_element));
      }
    }

    return vbox(row_elements);
  }

public:
  TypedTableRendererBase(TypedTableModel<T> model_)
    : model{model_}
  {}

  virtual Element RenderColumn(int column) override
  {
    return RenderColumn(model->ColumnName(column), column);
  }

  virtual Element RenderColumn(const std::string &val, int /* index */)
  {
    return text(val);
  }

  virtual Element RenderCell(const T &data, int column)
  {
    auto str_data = model->StringRepresentation(column, data);
    return text(str_data);
  }

  virtual RenderedParts Render() override
  {
    std::vector<int> column_widths(model->Columns().size());
    auto columns = AcumulateColumns(column_widths);
    auto rows = AcumulateRows(column_widths);

    return {
      RenderColumnPart(columns, column_widths),
      RenderDataPart(rows, column_widths)
    };
  }
};

using TableRenderer = std::shared_ptr<TableRendererBase>;
using TableModel = std::shared_ptr<TableModelBase>;
template<typename T>
using TypedTableRenderer = std::shared_ptr<TypedTableRendererBase<T>>;

class DataTableBase : public ComponentBase
{
  std::mutex data_mutex;

  TableModel model;
  TableRenderer item_renderer;

  Element column_part;
  Element data_part;
  Component data_part_scroller;
  Element format_part;

  void DataUpdated()
  {
    std::lock_guard<std::mutex> lock{data_mutex};
    auto parts = item_renderer->Render();
    column_part = parts.column_part;
    data_part = parts.data_part;
    format_part = FormatPart();
  }

  Element FormatPart()
  {
    Elements info;
    if(model->IsFiltered())
    {
      int filter_column = model->FilterColumn();
      if(filter_column != -1)
      {
        info.push_back(text("Filter " + model->ColumnName(filter_column) + ": " + model->filter));
      }
      else
      {
        info.push_back(text("Filter all: " + model->filter));
      }
      info.push_back(text(" | "));
    }
    if(model->IsGrouped())
    {
      auto group_column = model->GroupByColumn();
      info.push_back(separatorEmpty());
      info.push_back(text("Grouped by: " + model->ColumnName(group_column)));
      info.push_back(text(" | "));
    }

    if(!info.empty())
    {
      return vbox(
        separatorEmpty(),
        hbox(info),
        separatorEmpty()
      );
    }

    return emptyElement();
  }

public:
  DataTableBase(TableModel model_, TableRenderer item_renderer_)
    : model{model_},
      item_renderer{item_renderer_}
  {
    data_part_scroller = Scroller(Renderer([this]{
     return data_part;
    }), model->selected);
    DataUpdated();

    Add(data_part_scroller);
    model_->SetDataUpdatedCallback(std::bind(&DataTableBase::DataUpdated, this));
  }

  bool OnEvent(Event event) override
  {
    if(event == Event::Return) return model->ToggleGroupExpanded();
    return ComponentBase::OnEvent(event);
  }

  Element Render() override
  {
    std::lock_guard<std::mutex> lock{data_mutex};
    return vbox(
      column_part,
      data_part_scroller->Render(),
      format_part
    );
  }
};

using DataTable = std::shared_ptr<DataTableBase>;

template<typename T>
DataTable CreateDataTable(TypedTableModel<T> model, TypedTableRenderer<T> renderer = std::make_shared<TypedTableRenderer<T>>())
{
  return std::make_shared<DataTableBase>(model, renderer);
}

}

