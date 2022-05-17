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

#include <memory>

#include "tui/notification_manager.hpp"
#include "tui/event_loop.hpp"

class ViewModelFactory : public std::enable_shared_from_this<ViewModelFactory>
{
  std::shared_ptr<EventLoop> event_loop;
  std::shared_ptr<NotificationManager> notification_manager;

protected:
  ViewModelFactory(std::shared_ptr<EventLoop> event_loop_,
                   std::shared_ptr<NotificationManager> notification_manager_)
    : event_loop{event_loop_},
      notification_manager{notification_manager_} {}

public:
  static std::shared_ptr<ViewModelFactory> Create(
    std::shared_ptr<EventLoop> event_loop,
    std::shared_ptr<NotificationManager> notification_manager
  )
  {
    struct EnableMakeShared : public ViewModelFactory {
      EnableMakeShared(std::shared_ptr<EventLoop> event_loop,
        std::shared_ptr<NotificationManager> notification_manager)
        : ViewModelFactory(event_loop, notification_manager) {}
    };

    // and use that type with make_shared
    return std::make_shared<EnableMakeShared>(event_loop, notification_manager);
  }

  template<typename T, typename... ARGS>
  std::shared_ptr<T> Create(ARGS&&... args)
  {
    auto ptr = std::make_shared<T>(args...);
    ptr->event_loop = event_loop;
    ptr->notification_manager = notification_manager;
    ptr->view_model_factory = shared_from_this();
    ptr->Init();

    return ptr;
  }
};
