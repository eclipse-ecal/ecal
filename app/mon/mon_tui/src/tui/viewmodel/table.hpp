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

#include "tui/command_manager.hpp"

#include "tui/view/component/data_table.hpp"
#include "tui/viewmodel/viewmodel.hpp"

template<typename T>
class TableViewModel : public ftxui::TypedTableModelBase<T>, public ViewModel
{
  void NotifyUserUnknowColumn(const std::string &column_name)
  {
    NotifyUser(StatusLevel::ERROR, "Unknown column: " + column_name);
  }

public:
  std::string find = "";
  bool details_visible = false;

  bool auto_scroll = false;

  TableViewModel(const std::vector<std::string> &columns)
  {
    this->SetColumns(columns);
  }

  virtual void UpdateData(std::vector<T> &&data_in) override
  {
    if(data_in.empty()) HideDetails();
    ftxui::TypedTableModelBase<T>::UpdateData(std::move(data_in));
    ViewModel::NotifyDataUpdated();
  }

  void ShowDetails()
  {
    if(this->RowCount() == 0)
    {
      HideDetails();
      return;
    }
    details_visible = true;
  }

  void HideDetails()
  {
    details_visible = false;
  }

  void FindNext()
  {
    this->SelecteNext(find);
  }

  void FindPrev()
  {
    this->SelectPrev(find);
  }

  void FilterDataBy(const std::string &column_name, const std::string &filter_in)
  {
    auto column = this->FindColumnByName(column_name);
    if(column > -1)
    {
      if(filter_in == "") this->Filter(filter_in);
      else
      {
        this->Filter(filter_in, column);
      }
    }
    else
    {
      NotifyUserUnknowColumn(column_name);
    }
  }

  // Enable the Original GroupBy(int) function before hiding it with the GroupBy(string) function
  using ftxui::TypedTableModelBase<T>::GroupBy;

  void GroupBy(const std::string &column_name)
  {
    auto column = this->FindColumnByName(column_name);
    if(column > -1)
    {
      GroupBy(column);
    }
    else
    {
      NotifyUserUnknowColumn(column_name);
    }
  }

  void OrderByCommand(const CommandDetails &command)
  {
    auto &args = command.args;
    if(args.size() > 0)
    {
      auto &column_name = args[0];
      auto column = this->FindColumnByName(column_name);
      if(column == -1)
      {
        NotifyUserUnknowColumn(column_name);
        return;
      }

      switch(command.command)
      {
      case Command::ORDER_BY:
        ftxui::TableModelBase::Sort(column);
        break;
      case Command::ORDER_BY_ASC:
        this->Sort(column, ftxui::TableModelBase::SortDirection::ASC);
        break;
      case Command::ORDER_BY_DESC:
        this->Sort(column, ftxui::TableModelBase::SortDirection::DESC);
        break;
      default:
        break;
      }
    }
  }

  virtual void OnCommand(const CommandDetails &command) override
  {
    auto &args = command.args;
    switch(command.command)
    {
      case Command::FIND:
        if(args.size() > 0)
        {
          find = command.args[0];
          FindNext();
        } else find = "";
        break;
      case Command::FIND_NEXT:
        FindNext();
        break;
      case Command::FIND_PREVIOUS:
        FindPrev();
        break;
      case Command::ORDER_BY:
      case Command::ORDER_BY_ASC:
      case Command::ORDER_BY_DESC:
        OrderByCommand(command);
        break;
      case Command::FILTER:
        if(args.size() > 0) this->Filter(args[0]);
        else this->ClearFilter();
        if(this->RowCount() == 0) HideDetails();
        break;
      case Command::FILTER_BY:
        if(args.size() > 1)
        {
          FilterDataBy(args[0], args[1]);
          if(this->RowCount() == 0) HideDetails();
        }
        break;
      case Command::GROUP_BY:
        if(args.size() > 0)
        {
          GroupBy(args[0]);
        }
        else this->ClearGroupBy();
      case Command::DETAILS:
        ShowDetails();
        break;
      case Command::AUTO_SCROLL_DISABLE:
        auto_scroll = false;
        break;
      case Command::AUTO_SCROLL_ENABLE:
        auto_scroll = true;
        break;
      default: break;
    }
  }
};
