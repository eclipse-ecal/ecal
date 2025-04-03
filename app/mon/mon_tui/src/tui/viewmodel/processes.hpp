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

#include "tui/viewmodel/table.hpp"

#include "model/monitor.hpp"

class ProcessesViewModel : public TableViewModel<Process>
{
public:
  enum Column
  {
    Host, PID, Name, Info, State
  };

  ProcessesViewModel(std::shared_ptr<MonitorModel> model_)
    : TableViewModel<Process>({"Host", "PID", "Name", "Info", "State"})
  {
    title = "Processes";
    model_->AddModelUpdateCallback([this, model_] {
      TableViewModel::UpdateData(model_->Processes());
    });
  }

  virtual std::string StringRepresentation(int column, const Process& value) override
  {
    switch (column)
    {
      case Column::Host:
        return value.host_name;
      case Column::PID:
        return std::to_string(value.process_id);
      case Column::Name:
        return value.name;
      case Column::Info:
        return value.state_info;
      case Column::State:
        return std::to_string(value.severity);
      default: return "";
    }
  }
};
