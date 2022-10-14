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
#include <unordered_map>
#include <memory>

#include <ftxui/component/event.hpp>

#include "tui/command_manager.hpp"

using KeybindsMap = std::unordered_map<std::string, Command>;

class KeybindsManager
{
  std::shared_ptr<KeybindsMap> keybinds;

public:
  KeybindsManager(std::shared_ptr<KeybindsMap> keybinds_)
    : keybinds{keybinds_} {}

  Command GetCommand(const ftxui::Event &event)
  {
    auto find = keybinds->find(event.character());
    if(find != keybinds->end()) return find->second;

    return Command::UNSUPPORTED;
  }
};

std::shared_ptr<KeybindsMap> CreateKeybindsMap()
{
  using namespace ftxui;
  return std::make_shared<KeybindsMap>(KeybindsMap {
    {Event::Character("1").character(), Command::NAVIGATE_TAB1},
    {Event::Character("2").character(), Command::NAVIGATE_TAB2},
    {Event::Character("3").character(), Command::NAVIGATE_TAB3},
    {Event::Character("4").character(), Command::NAVIGATE_TAB4},
    {Event::Character("5").character(), Command::NAVIGATE_TAB5},
    {Event::Character("6").character(), Command::NAVIGATE_TAB6},
    {Event::Character("7").character(), Command::NAVIGATE_TAB7},
    {Event::Character("n").character(), Command::FIND_NEXT},
    {Event::Character("N").character(), Command::FIND_PREVIOUS},
    {Event::Return.character(), Command::ACTION},
    {Event::Escape.character(), Command::CANCEL},
    {Event::Character(":").character(), Command::START_COMMAND},
    {Event::Character("/").character(), Command::START_FIND},
    {Event::Character("\\").character(), Command::START_FILTER},

    //These are mapped implicitly by components at the moment, they are in this list just for book keepig.
    {Event::Character("h").character(), Command::MOVE_LEFT},
    {Event::Character("j").character(), Command::MOVE_DOWN},
    {Event::Character("k").character(), Command::MOVE_UP},
    {Event::Character("l").character(), Command::MOVE_RIGHT},
    {Event::ArrowLeft.character(), Command::MOVE_LEFT},
    {Event::ArrowDown.character(), Command::MOVE_DOWN},
    {Event::ArrowUp.character(), Command::MOVE_UP},
    {Event::ArrowRight.character(), Command::MOVE_RIGHT},
    {Event::Tab.character(), Command::FOCUS_NEXT},
    {Event::TabReverse.character(), Command::FOCUS_PREVIOUS},
  });
}

