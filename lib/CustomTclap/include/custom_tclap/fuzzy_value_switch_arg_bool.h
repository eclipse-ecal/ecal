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

#include <tclap/ValueArg.h>
#include <string>

namespace CustomTclap
{
  /**
   * @brief A Combination of a SwitchArg and a ValueArg<bool> with auto-detection
   *
   * The FuzzyValueSwitchArgBool aims to combine the advantages of the
   * TCLAP::SwitchArg and the TCLAP::ValueArg. It is autodetected whether the
   * parameter becomes a SwitchArg (no following value) or a ValueArg (The user
   * specified the value on command line):
   *
   *     - VALUE MODE: The user can manually turn the switch on or off:
   *         - {true  | on  | 1 | yes} result in true
   *         - {false | off | 0 | no } result in false
   *         - Parsing is case-insensitive (i.e. true, TRUE, True, TrUe, ON will
   *           all result in true)
   *
   *         Example: my_prog --setting1 true -a false -b ON -c on
   *
   *     - SWITCH MODE: The user does not have to provide true|false
   *         - When encountering the parameter without any boolean-parsable
   *           value, it is assumed that the user wants to turn the switch on,
   *           just like a normal SwitchArg (Why else would he provide the
   *           parameter)
   *         - The short version of the argument can be combined with other
   *           switches. In that case, manuelly providing true/false is not
   *           supported.
   *
   *         Example: my_prog --setting1 -bc
   *
   */
  class FuzzyValueSwitchArgBool : public TCLAP::ValueArg<bool>
  {
  public:
    FuzzyValueSwitchArgBool(const std::string &flag, const std::string &name, const std::string &desc, bool req, bool value, const std::string &typeDesc, TCLAP::Visitor *v = NULL);
    FuzzyValueSwitchArgBool(const std::string &flag, const std::string &name, const std::string &desc, bool req, bool value, const std::string &typeDesc, TCLAP::CmdLineInterface &parser, TCLAP::Visitor *v = NULL);

    virtual bool processArg(int *i, std::vector<std::string> &args) override;

  private:
    bool combinedSwitchesMatch(std::string& combinedSwitches);
    bool lastCombined(std::string& combinedSwitches);
  };
}
