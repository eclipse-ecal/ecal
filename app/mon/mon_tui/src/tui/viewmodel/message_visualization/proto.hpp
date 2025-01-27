/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
#include <memory>
#include <mutex>

#include <ecal/ecal.h>
#include <ecal/msg/string/subscriber.h>

#include <ecal/msg/protobuf/dynamic_subscriber.h>
#include <ecal/msg/protobuf/ecal_proto_decoder.h>

#include "tui/viewmodel/message_visualization/message_visualization.hpp"

class ProtoMessageVisualizationViewModel : public MessageVisualizationViewModel
{
  eCAL::protobuf::CDynamicSubscriber subscriber;

  mutable std::mutex message_mtx;
  std::shared_ptr<google::protobuf::Message> latest_message = nullptr;

  void OnMessage(const std::shared_ptr<google::protobuf::Message>& message, long long send_time_usecs)
  {
    {
      std::lock_guard<std::mutex> lock(message_mtx);
      latest_message = message;

      message_timestamp = send_time_usecs;
    }

    NotifyDataUpdated();
  }

public:
  //Wrapper around message that keeps it locked outside of context of this class
  //NOTE: Use with caution!
  struct ProtectedMessage
  {
    std::shared_ptr<google::protobuf::Message> message;
    std::unique_lock<std::mutex> lock;
  };

  ProtoMessageVisualizationViewModel(const std::string &topic)
    : subscriber{topic}
  {
    using namespace std::placeholders;
    subscriber.SetReceiveCallback(std::bind(&ProtoMessageVisualizationViewModel::OnMessage, this, _2, _3));
  }

  ProtectedMessage message() const
  {
    std::unique_lock<std::mutex> lock{message_mtx};
    return { latest_message, std::move(lock) };
  }
};
