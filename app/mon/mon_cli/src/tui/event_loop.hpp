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
#include <functional>

class EventLoop
{
public:
  using Callback = std::function<void()>;

private:
  std::vector<Callback> data_updated_callbacks;
  std::vector<Callback> stop_application_callbacks;

public:
  void SubscribeToDataUpdated(Callback callback)
  {
    data_updated_callbacks.push_back(callback);
  }

  void PostDataUpdated()
  {
    for(auto callback: data_updated_callbacks)
    {
      callback();
    }
  }

  void SubscribeToStopApplication(Callback callback)
  {
    stop_application_callbacks.push_back(callback);
  }

  void PostStopApplication()
  {
    for(auto callback: stop_application_callbacks)
    {
      callback();
    }
  }
};
