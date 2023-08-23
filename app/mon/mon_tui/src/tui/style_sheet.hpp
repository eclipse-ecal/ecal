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

#include <functional>
#include <sstream>
#include <ctime>
#include <iomanip>

#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component_options.hpp>

#include "ftxui/screen/color.hpp"
#include "model/data/process.hpp"
#include "model/log.hpp"
#include "tui/notification_manager.hpp"
#include "tui/view/component/data_table.hpp"
#include "tui/viewmodel/table.hpp"
#include "tui/viewmodel/command_line.hpp"

enum class StyleSheetType
{
  DEFAULT, MONOCHROME
};

struct TableStyleSheet
{
  ftxui::Decorator header;
  ftxui::BorderStyle separator_vertical;
  std::function<std::string(ftxui::TableModelBase::SortDirection)> order_by_marker;
  ftxui::Decorator group_row;
};

struct CommandLineStyleSheet
{
  std::function<ftxui::Decorator(StatusLevel)> status;
  std::function<ftxui::Decorator(CommandLineViewModel::Mode)> mode;
};

struct ProtoTreeStyleSheet
{
  ftxui::Decorator msg_header;
  ftxui::Decorator primitive_header;
  ftxui::Decorator primitive;
  ftxui::Decorator array_header;
  ftxui::Decorator array_value;
};

struct StyleSheet
{
  ftxui::Decorator error_style;
  TableStyleSheet table;
  std::function<ftxui::Decorator(LogEntry::Level)> log_level_style;
  ftxui::MenuOption tab;
  std::function<std::string(int /*nsec_timestamp*/)> timestamp;
  ProtoTreeStyleSheet proto_tree;
  CommandLineStyleSheet command_line;
  std::function<ftxui::Decorator(Process::Severity)> process_table_severity;

  static std::shared_ptr<StyleSheet> Default()
  {
    using namespace ftxui;
    StyleSheet sheet;
    sheet.error_style = bgcolor(Color::Palette16::Red) | color(Color::Palette16::GrayDark),

    sheet.table.header = bold | color(Color::Palette16::Yellow);
    sheet.table.separator_vertical = LIGHT;
    sheet.table.order_by_marker = [](auto direction) {
      switch(direction)
      {
        case ftxui::TableModelBase::SortDirection::DESC: return "▼";
        case ftxui::TableModelBase::SortDirection::ASC: return "▲";
        default: return "?";
      }
    };
    sheet.table.group_row = bold | color(Color::Palette16::Blue);
    sheet.log_level_style = [](auto level) -> Decorator {
      switch (level)
      {
        case LogEntry::Level::INFO: return color(Color::Palette16::Blue);
        case LogEntry::Level::WARNING: return color(Color::Palette16::Yellow);
        case LogEntry::Level::ERROR: return color(Color::Palette16::Red);
        case LogEntry::Level::FATAL: return color(Color::Palette16::Red) | bold;
        default: return nothing;
      }
    };
#if FTXUI_VERSION_MAJOR >= 5
    sheet.tab.entries_option.transform =
#else
    sheet.tab.entries.transform =
#endif
      [](EntryState state)
    {
      if(state.active)
      {
        return text(state.label)
          | bold
          | inverted;
      }
      return text(state.label);
    };
#if FTXUI_VERSION_MAJOR >= 5
    sheet.tab.direction = Direction::Right;
#else
    sheet.tab.direction = MenuOption::Right;
#endif
    sheet.tab.elements_prefix = [] {
      return text(" ");
    };
    sheet.tab.elements_infix = [] {
      return text("  ");
    };
    sheet.tab.elements_postfix = [] {
      return text(" ");
    };
    sheet.timestamp = [](int nsec_timestamp) -> std::string {
      //This is probably not universaly portable, might fail if system typedefs time_t differently
      std::time_t time = nsec_timestamp/(1000 * 1000);
      auto tm = std::gmtime(&time);
      std::stringstream ss;
      std::string s;
      ss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
      return ss.str();
    };
    sheet.proto_tree.msg_header = color(Color::Palette16::Yellow);
    sheet.proto_tree.primitive_header = color(Color::Palette16::Blue);
    sheet.proto_tree.primitive = nothing;
    sheet.proto_tree.array_header = color(Color::Palette16::Yellow);
    sheet.proto_tree.array_value = color(Color::Palette16::Green);
    sheet.command_line.status = [&](auto level) -> Decorator {
      switch(level)
      {
        case StatusLevel::ERROR:
          return color(Color::Palette16::Red);
        default: return color(Color::Palette16::GrayDark);
      }
    };
    sheet.command_line.mode = [&](auto mode) -> Decorator {
      switch(mode)
      {
        case CommandLineViewModel::Mode::COMMAND:
          return color(Color::Palette16::Green);
        case CommandLineViewModel::Mode::FILTER:
          return color(Color::Palette16::Blue);
        case CommandLineViewModel::Mode::SEARCH:
          return color(Color::Palette16::Yellow);
        case CommandLineViewModel::Mode::DEFAULT:
        default:
          return nothing;
      }
    };
    sheet.process_table_severity = [](auto severity) -> Decorator {
      switch (severity)
      {
        case Process::Severity::HEALTHY:
          return color(Color::Palette16::Green);
        case Process::Severity::WARNING:
          return color(Color::Palette16::Yellow);
        case Process::Severity::CRITICAL:
          return color(Color::Palette16::Red);
        case Process::Severity::FAILED:
          return color(Color::Palette16::Red) | bold;
        case Process::Severity::UNKNOWN:
        default:
          return nothing;
      }
    };

    return std::make_shared<StyleSheet>(std::move(sheet));
  }

  static std::shared_ptr<StyleSheet> Monochrome()
  {
    using namespace ftxui;
    StyleSheet sheet;
    sheet.error_style = inverted;
    sheet.table.header = inverted;
    sheet.table.separator_vertical = LIGHT;
    sheet.table.order_by_marker = [](auto direction) {
      switch(direction)
      {
        case ftxui::TableModelBase::SortDirection::DESC: return " (desc)";
        case ftxui::TableModelBase::SortDirection::ASC: return " (asc)";
        default: return " (desc)";
      }
    };
    sheet.table.group_row = inverted;
    sheet.log_level_style = [](auto level) -> Decorator {
      switch(level)
      {
        case LogEntry::Level::WARNING:
        case LogEntry::Level::ERROR:
        case LogEntry::Level::FATAL:
          return inverted;
        default: return nothing;
      }
    };
#if FTXUI_VERSION_MAJOR >= 5
    sheet.tab.entries_option.transform = 
#else
    sheet.tab.entries.transform =
#endif
    [](EntryState state) {
      if(state.active)
      {
        return text(state.label) | inverted;
      }
      return text(state.label);
    };
#if FTXUI_VERSION_MAJOR >= 5
    sheet.tab.direction = Direction::Right;
#else
    sheet.tab.direction = MenuOption::Right;
#endif
    sheet.tab.elements_prefix = [] {
      return text(" ");
    };
    sheet.tab.elements_infix = [] {
      return text("  ");
    };
    sheet.tab.elements_postfix = [] {
      return text(" ");
    };
    sheet.timestamp = [](int nsec_timestamp) -> std::string {
      //This is probably not universaly portable, might fail if system typedefs time_t differently
      std::time_t time = nsec_timestamp/(1000 * 1000);
      auto tm = std::gmtime(&time);
      std::stringstream ss;
      std::string s;
      ss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
      return ss.str();
    };
    sheet.proto_tree.msg_header = nothing;
    sheet.proto_tree.primitive_header = nothing;
    sheet.proto_tree.primitive = nothing;
    sheet.proto_tree.array_header = nothing;
    sheet.proto_tree.array_value = nothing;
    sheet.command_line.status = [&](auto level) -> Decorator {
      switch(level)
      {
        case StatusLevel::ERROR:
          return inverted;
        default: return nothing;
      }
    };
    sheet.command_line.mode = [&](auto /* mode */) -> Decorator {
      return nothing;
    };
    sheet.process_table_severity = [](auto severity) -> Decorator {
      switch (severity)
      {
        case Process::Severity::WARNING:
        case Process::Severity::CRITICAL:
        case Process::Severity::FAILED:
          return inverted;
        case Process::Severity::HEALTHY:
        case Process::Severity::UNKNOWN:
        default:
          return nothing;
      }
    };

    return std::make_shared<StyleSheet>(std::move(sheet));
  }

  static std::shared_ptr<StyleSheet> Create(StyleSheetType type)
  {
    switch(type)
    {
      case StyleSheetType::DEFAULT:
        return Default();
      case StyleSheetType::MONOCHROME:
        return Monochrome();
      default:
        return Default();
    }
  }
};
