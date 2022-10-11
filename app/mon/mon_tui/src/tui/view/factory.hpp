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

#include "tui/style_sheet.hpp"
#include "tui/keybinds_manager.hpp"

class ViewFactory : public std::enable_shared_from_this<ViewFactory>
{
  std::shared_ptr<StyleSheet> style;
  std::shared_ptr<KeybindsManager> keybinds_manager;

protected:
  ViewFactory(std::shared_ptr<StyleSheet> style_,
              std::shared_ptr<KeybindsManager> keybinds_manager_)
    : style{style_}, keybinds_manager{keybinds_manager_} { };

public:
  static std::shared_ptr<ViewFactory> Create(std::shared_ptr<StyleSheet> style,
                                             std::shared_ptr<KeybindsManager> key_binds)
  {
    struct EnableMakeShared : public ViewFactory {
      EnableMakeShared(std::shared_ptr<StyleSheet> style,
                       std::shared_ptr<KeybindsManager> key_binds)
        : ViewFactory(style, key_binds) {}
    };

    // and use that type with make_shared
    return std::make_shared<EnableMakeShared>(style, key_binds);
  }

  template<typename T, typename... ARGS>
  std::shared_ptr<T> Create(ARGS&&... args)
  {
    auto ptr = std::make_shared<T>(args...);
    ptr->style = style;
    ptr->keybinds_manager = keybinds_manager;
    ptr->view_factory = shared_from_this();
    ptr->Init();

    return ptr;
  }
};
