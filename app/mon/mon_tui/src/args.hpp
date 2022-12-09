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

#include <unordered_set>

#include <tclap/CmdLine.h>
#include <tclap/Constraint.h>

#include "ecal_mon_tui_defs.h"

struct Args
{
  enum class Color
  {
    YES, NO, AUTO
  };
  Color color = Color::AUTO;
};

class ColorArgConstraint : public TCLAP::Constraint<std::string>
{
  std::unordered_set<std::string> possible_values = {"auto", "yes", "always", "no", "never"};
  std::string description() const override { return ""; }
  std::string shortID() const override { return ""; }
  bool check(const std::string &value) const override
  {
    return possible_values.find(value) != possible_values.end();
  }
};

Args::Color ColorArgsFromString(const std::string &val)
{
  if(val == "yes" || val == "always")
    return Args::Color::YES;
  if(val == "no" || val == "never")
    return Args::Color::NO;
  return Args::Color::AUTO;
}

Args ParseArgs(int argc, char** argv)
{
  TCLAP::CmdLine cmd(ECAL_MON_TUI_NAME, ' ', ECAL_MON_TUI_VERSION);
  ColorArgConstraint color_arg_constraint{};

  auto color = TCLAP::ValueArg<std::string>("", "color",
    "Specifies whether monitor should render colors, possible values are (default is auto): \n"
    "  yes/always - render colors\n"
    "  no/never - don't render colors\n"
    "  auto - automatically decide whether to render colors.",
    false, "auto", &color_arg_constraint, cmd);

  cmd.parse(argc, argv);

  return Args {
    ColorArgsFromString(color.getValue())
  };
}
