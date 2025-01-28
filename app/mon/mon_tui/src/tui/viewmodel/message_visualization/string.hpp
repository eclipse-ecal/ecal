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
#include <functional>
#include <mutex>

#include <ecal/ecal.h>
#include <ecal/msg/string/subscriber.h>

#include "tui/viewmodel/message_visualization/message_visualization.hpp"

class StringMessageVisualizationViewModel : public MessageVisualizationViewModel
{
  eCAL::string::CSubscriber<std::string> subscriber;

  mutable std::mutex message_mtx;
  std::string latest_message;

  void OnMessage(const std::string& msg, long long time)
  {
    {
      std::lock_guard<std::mutex> lock{message_mtx};
      latest_message = std::move(msg);

      message_timestamp = time;
    }

    NotifyDataUpdated();
  }

public:
  StringMessageVisualizationViewModel(const std::string &topic)
    : subscriber{topic}
  {
    using namespace std::placeholders;
    subscriber.SetReceiveCallback(std::bind(&StringMessageVisualizationViewModel::OnMessage, this, _2, _3));
  }

  std::string message() const
  {
    std::lock_guard<std::mutex> lock{message_mtx};
    return latest_message;
  }
};
