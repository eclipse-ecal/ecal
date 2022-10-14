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
#include <memory>

#include "tui/view/message_visualization/raw.hpp"
#include "tui/view/message_visualization/string.hpp"
#include "tui/view/message_visualization/proto.hpp"

#include "utils/string.hpp"

std::shared_ptr<MessageVisualizationView> CreateVisualizationView(std::shared_ptr<ViewFactory> factory,
    std::shared_ptr<ViewModelFactory> vm_factory, const std::string &topic, const std::string &type_name)
{
  if(type_name == "base:std::string")
  {
    auto vm = vm_factory->Create<StringMessageVisualizationViewModel>(topic);
    return factory->Create<StringMessageVisualizationView>(vm);
  }
  else if(StartsWith(type_name, "proto:"))
  {
    auto vm = vm_factory->Create<ProtoMessageVisualizationViewModel>(topic);
    return factory->Create<ProtoMessageVisualizationView>(vm);
  }

  auto vm = vm_factory->Create<RawMessageVisualizationViewModel>(topic);
  return factory->Create<RawMessageVisualizationView>(vm);
}
