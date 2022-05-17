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

/**
 * @brief eCALMon Console Application
**/
#include <memory>
#include <string>
#include <cstdlib>

#include <ftxui/component/screen_interactive.hpp>

#include "args.hpp"

#include "tui/style_sheet.hpp"
#include "tui/keybinds_manager.hpp"
#include "tui/command_manager.hpp"
#include "tui/notification_manager.hpp"

#include "tui/view/shell.hpp"
#include "tui/view/factory.hpp"

#include "tui/viewmodel/shell.hpp"
#include "tui/viewmodel/factory.hpp"

#include "model/monitor.hpp"
#include "model/log.hpp"

namespace TUI
{
  StyleSheetType GetStyleType(const Args &args)
  {
    if(args.color == Args::Color::YES)
    {
      return StyleSheetType::DEFAULT;
    }

    if(args.color == Args::Color::NO)
    {
      return StyleSheetType::MONOCHROME;
    }

    auto env_no_color = std::getenv("NO_COLOR");
    auto env_term = std::getenv("TERM");
    std::string env_term_str = env_term ? env_term : "";

    if(env_no_color || env_term_str == "dumb")
    {
      return StyleSheetType::MONOCHROME;
    }
    return StyleSheetType::DEFAULT;
  }

  void Start(Args &args)
  {
    auto commands = CreateCommandMap();
    auto command_descriptions = CreateCommandDescriptionMap();
    auto command_manager = std::make_shared<CommandManager>(commands, command_descriptions);

    auto keybinds = CreateKeybindsMap();
    auto keybinds_manager = std::make_shared<KeybindsManager>(keybinds);

    auto event_loop = std::make_shared<EventLoop>();

    auto model = std::make_shared<MonitorModel>();
    auto logs_model = std::make_shared<LogModel>();

    auto notification_manager = std::make_shared<NotificationManager>();

    auto style_type = GetStyleType(args);
    auto style = StyleSheet::Create(style_type);
    auto view_factory = ViewFactory::Create(style, keybinds_manager);

    auto view_model_factory = ViewModelFactory::Create(event_loop, notification_manager);
    auto shell_vm = view_model_factory->Create<ShellViewModel>(model, logs_model, command_manager);
    auto shell = view_factory->Create<ShellView>(shell_vm);

    auto screen = ftxui::ScreenInteractive::Fullscreen();

    event_loop->SubscribeToStopApplication([&screen](){
      screen.ExitLoopClosure()();
    });

    //FTXUI doesn't support partial screen refresh, so every time data updates,
    //refresh whole screen
    event_loop->SubscribeToDataUpdated([&screen](){
      screen.PostEvent(ftxui::Event::Custom);
    });

    screen.Loop(shell);
  }
}
