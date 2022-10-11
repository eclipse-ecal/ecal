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
#include <sstream>
#include <iomanip>
#include <functional>
#include <mutex>

#include <ecal/ecal.h>
#include <ecal/msg/string/subscriber.h>

#include "tui/viewmodel/message_visualization/message_visualization.hpp"

class RawMessageVisualizationViewModel : public MessageVisualizationViewModel
{
  eCAL::CSubscriber subscriber;

  mutable std::mutex message_mtx;
  std::string latest_message;

  void OnMessage(const struct eCAL::SReceiveCallbackData* callback_data)
  {
    {
      std::lock_guard<std::mutex> lock{message_mtx};
      latest_message = std::string(static_cast<char *>(callback_data->buf), callback_data->size);
      message_timestamp = callback_data->time;
    }

    NotifyDataUpdated();
  }

public:
  RawMessageVisualizationViewModel(const std::string &topic)
    : subscriber{topic}
  {
    using namespace std::placeholders;
    subscriber.AddReceiveCallback(std::bind(&RawMessageVisualizationViewModel::OnMessage, this, _2));
  }

  std::string message() const
  {
    std::lock_guard<std::mutex> lock{message_mtx};
    return latest_message;
  }
};
