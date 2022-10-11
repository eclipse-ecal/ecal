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

#include <string>
#include <ctime>
#include <iomanip>

#include "tui/command_manager.hpp"

#include "tui/viewmodel/table.hpp"

#include "model/log.hpp"
#include "utils/string.hpp"

class LogsViewModel : public TableViewModel<LogEntry>
{
  std::shared_ptr<LogModel> model;

public:
  enum Column
  {
    Time, Level, Process, Message
  };

  LogsViewModel(std::shared_ptr<LogModel> model_)
    : TableViewModel<LogEntry>({"Time", "Level", "Process", "Message"})
  {
    title = "Logs";
    auto_scroll = true;
    model_->AddModelUpdateCallback([this, model_] {
        TableViewModel::UpdateData(model_->Logs());
    });
  }

  virtual std::string StringRepresentation(int column, const LogEntry& value) override
  {
    switch (column)
    {
      case Column::Time:
        return EcalTimestampToString(value.time);
      case Column::Level:
        return std::to_string(value.log_level);
      case Column::Process:
        return value.process_name;
      case Column::Message:
        return value.message;
      default: return "";
    }
  }

  void OnCommand(const CommandDetails &command) final override
  {
    if(command.command == Command::CLEAR)
    {
      model->Clear();
    }
    else
    {
      TableViewModel::OnCommand(command);
    }
  }
};
