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

#include "tui/viewmodel/table.hpp"

#include "model/monitor.hpp"

class HostsViewModel : public TableViewModel<Host>
{
public:
  enum Column
  {
    Name, PublisherCount, SubscriberCount, DataSent, DataRecieved
  };

  HostsViewModel(std::shared_ptr<MonitorModel> model_)
    : TableViewModel<Host>({"Name", "PublisherCount", "SubscriberCount", "DataSent", "DataRecieved"})
  {
    title = "Hosts";
    model_->AddModelUpdateCallback([this, model_] {
      TableViewModel::UpdateData(model_->Hosts());
    });
  }

  virtual std::string StringRepresentation(int column, const Host& value) override
  {
    switch (column)
    {
      case Column::Name:
        return value.name;
      case Column::PublisherCount:
        return std::to_string(value.publisher_count);
      case Column::SubscriberCount:
        return std::to_string(value.subscriber_count);
      case Column::DataSent:
        return std::to_string(value.data_sent_bytes);
      case Column::DataRecieved:
        return std::to_string(value.data_received_bytes);
      default: return "";
    }
  }
};
