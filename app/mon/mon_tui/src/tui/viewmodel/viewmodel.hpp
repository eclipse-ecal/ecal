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
#include <memory>

#include "tui/event_loop.hpp"
#include "tui/notification_manager.hpp"
#include "tui/command_manager.hpp"

#include "tui/viewmodel/factory.hpp"

class ViewModel
{
public:
  std::shared_ptr<EventLoop> event_loop;
  std::shared_ptr<NotificationManager> notification_manager;
  std::shared_ptr<ViewModelFactory> view_model_factory;
  std::string title = "";

  void NotifyUser(StatusLevel level, const std::string &message)
  {
    notification_manager->Notify(level, message);
  }

  void NotifyDataUpdated()
  {
    if(event_loop) event_loop->PostDataUpdated();
  }

  void StopApplication()
  {
    if(event_loop) event_loop->PostStopApplication();
  }

  template<typename T, typename... ARGS>
  std::shared_ptr<T> CreateViewModel(ARGS&&... args)
  {
    if(view_model_factory) return view_model_factory->Create<T>(args...);
    return nullptr;
  }

  virtual void Init() {}
  virtual void OnCommand(const CommandDetails&  /* command */) {}

  virtual ~ViewModel() = default;
};

