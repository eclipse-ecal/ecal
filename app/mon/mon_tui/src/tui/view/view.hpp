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

#include <ftxui/component/component.hpp>

#include "tui/keybinds_manager.hpp"
#include "tui/style_sheet.hpp"

#include "tui/view/factory.hpp"
#include "tui/view/component/focus_manager.hpp"

#include "tui/viewmodel/viewmodel.hpp"

class View : public ftxui::ComponentBase
{
protected:
  std::shared_ptr<FocusManager> focus_manager;
public:
  std::shared_ptr<StyleSheet> style;
  std::shared_ptr<KeybindsManager> keybinds_manager;
  std::shared_ptr<ViewFactory> view_factory;

  template<typename T, typename... ARGS>
  std::shared_ptr<T> CreateSubView(ARGS&&... args)
  {
    if(view_factory) return view_factory->Create<T>(args...);
    return nullptr;
  }

  Command KeyCommand(const ftxui::Event &event)
  {
    return keybinds_manager->GetCommand(event);
  }

  virtual void Init() {};
  virtual class ViewModel& ViewModel() = 0;

  virtual ~View() = default;
};
