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
#include <memory>

#include "tui/viewmodel/table.hpp"

#include "model/monitor.hpp"

#include "utils/string.hpp"

class TopicsViewModel : public TableViewModel<Topic>
{
public:
  enum Column
  {
    Id, Host, Process, Name, Encoding, Type, Direction, Layer, Size, DataClock, Frequency
  };

  TopicsViewModel(std::shared_ptr<MonitorModel> model_)
    : TableViewModel<Topic>({"Id", "Host", "Process", "Topic", "Encoding", "Type", "Direction", "Layer", "Size", "DataClock", "Frequency(Hz)"})
  {
    title = "Topics";

    // Sort(Column::Process);
    // GroupBy(Column::Process);

    model_->AddModelUpdateCallback([this, model_] {
      TableViewModel::UpdateData(model_->Topics());
    });
  }

  virtual std::string StringRepresentation(int column, const Topic& value) override
  {
    switch (column)
    {
      case Column::Id:
        return value.id;
      case Column::Host:
        return value.host_name;
      case Column::Process:
        return value.unit_name;
      case Column::Name:
        return value.name;
      case Column::Encoding:
        return value.encoding;
      case Column::Type:
        return value.type;
      case Column::Direction:
        return std::to_string(value.direction);
      case Column::Layer:
        return std::to_string(value.transport_layers);
      case Column::Size:
        return std::to_string(value.size);
      case Column::DataClock:
        return std::to_string(value.data_clock);
      case Column::Frequency:
        return roundToTwoDecimalsAsString(value.data_frequency_mhz / 1000.0f);
      default: return "";
    }
  }

  private:
    static std::string roundToTwoDecimalsAsString(double value) {
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(2) << value;
        return stream.str();
    }

};
