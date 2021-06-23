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
   * The FuzzyValueSwitchArgDouble aims to combine the advantages of the
   * TCLAP::SwitchArg and the TCLAP::ValueArg. It is autodetected whether the
   * parameter becomes a SwitchArg (no following value) or a ValueArg (The user
   * specified the value on command line):
   *
   *     - VALUE MODE: The user manually provides a value
   *         - When the user provides a value that can be converted to a double
   *           value, this value that can be retrieved by ::Value().
   *
   *         Example: my_prog --setting1 4.2 --setting2 5
   *
   *     - SWITCH MODE: The user does not provide a value.
   *         - When encountering the parameter without any double-parsable
   *           value, it is assumed that the user wants to turn the switch on,
   *           just like a normal SwitchArg. The value retrieved by ::Value()
   *           will be the default value.
   *         - The short version of the argument can be combined with other
   *           switches. In that case, manually providing a value is not
   *           supported.
   *
   *         Example: my_prog --setting1 -bc
   *
   */
  class FuzzyValueSwitchArgDouble : public TCLAP::ValueArg<double>
  {
  public:
    FuzzyValueSwitchArgDouble(const std::string &flag, const std::string &name, const std::string &desc, bool req, double value, const std::string &typeDesc, TCLAP::Visitor *v = NULL);
    FuzzyValueSwitchArgDouble(const std::string &flag, const std::string &name, const std::string &desc, bool req, double value, const std::string &typeDesc, TCLAP::CmdLineInterface &parser, TCLAP::Visitor *v = NULL);

    virtual bool processArg(int *i, std::vector<std::string> &args) override;

  private:
    bool combinedSwitchesMatch(std::string& combinedSwitches);
    bool lastCombined(std::string& combinedSwitches);
  };
}
