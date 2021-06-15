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

#include <custom_tclap/fuzzy_value_switch_arg_bool.h>

#include <algorithm>
#include <cctype>

namespace CustomTclap
{
  FuzzyValueSwitchArgBool::FuzzyValueSwitchArgBool(const std::string &flag, const std::string &name, const std::string &desc, bool req, bool value, const std::string &typeDesc, TCLAP::Visitor *v)
    : TCLAP::ValueArg<bool>(flag, name, desc, req, value, typeDesc, v)
  {}

  FuzzyValueSwitchArgBool::FuzzyValueSwitchArgBool(const std::string &flag, const std::string &name, const std::string &desc, bool req, bool value, const std::string &typeDesc, TCLAP::CmdLineInterface &parser, TCLAP::Visitor *v)
    : TCLAP::ValueArg<bool>(flag, name, desc, req, value, typeDesc, parser, v)
  {}

  bool FuzzyValueSwitchArgBool::processArg(int *i, std::vector<std::string> &args)
  {
    if ((_ignoreable && Arg::ignoreRest()))
    {
      return false;
    }

    // A direct arg match can have a TRUE / FALSE toggle
    if (argMatches(args[*i]))
    {
      if (_alreadySet)
      {
        if (_xorSet)
        {
          throw(TCLAP::CmdLineParseException("Mutually exclusive argument already set!", toString()));
        }
        else
        {
          throw(TCLAP::CmdLineParseException("Argument already set!", toString()));
        }
      }

      if ((*i + 1) >= (int)args.size())
      {
        // Implicit TRUE if there are no following parameters
        _value = true;
      }
      else
      {
        // Evaluate the next parameter
        std::string bool_string = args[*i + 1];
        std::transform(bool_string.begin(), bool_string.end(), bool_string.begin(), [](unsigned char c) { return static_cast<unsigned char>(std::tolower(c)); });

        // If the user manually specified 0|1|false|true|off|on, we use that value.
        if (bool_string == "1" || bool_string == "true" || bool_string == "on" || bool_string == "yes")
        {
          // TRUE
          (*i)++;
          _value = true;
        }
        else if (bool_string == "0" || bool_string == "false" || bool_string == "off" || bool_string == "no")
        {
          // FALSE
          (*i)++;
          _value = false;
        }
        else
        {
          // If the user did not specify 0|1|false|true|off|on|no|yes, we assume TRUE
          _value = true;
        }
      }

      _alreadySet = true;
      _checkWithVisitor();
      return true;
    }
    // A match in a combination cannot have TRUE/FALSE applied. So we will assume it being true.
    else if (combinedSwitchesMatch(args[*i]))
    {
      // check again to ensure we don't misinterpret 
      // this as a MultiSwitchArg 
      if (combinedSwitchesMatch(args[*i]))
        throw(TCLAP::CmdLineParseException("Argument already set!", toString()));

      if (_alreadySet)
      {
        if (_xorSet)
        {
          throw(TCLAP::CmdLineParseException("Mutually exclusive argument already set!", toString()));
        }
        else
        {
          throw(TCLAP::CmdLineParseException("Argument already set!", toString()));
        }
      }

      _alreadySet = true;
      _value = true;
      _checkWithVisitor();
      return lastCombined(args[*i]);
    }
    else
    {
      return false;
    }
  }

  // Taken from TCLAP::SwitchArg
  bool FuzzyValueSwitchArgBool::combinedSwitchesMatch(std::string& combinedSwitches)
  {
    // make sure this is actually a combined switch
    if (combinedSwitches.length() > 0 &&
      combinedSwitches[0] != Arg::flagStartString()[0])
      return false;

    // make sure it isn't a long name 
    if (combinedSwitches.substr(0, Arg::nameStartString().length()) ==
      Arg::nameStartString())
      return false;

    // make sure the delimiter isn't in the string 
    if (combinedSwitches.find_first_of(Arg::delimiter()) != std::string::npos)
      return false;

    // ok, we're not specifying a ValueArg, so we know that we have
    // a combined switch list.  
    for (unsigned int i = 1; i < combinedSwitches.length(); i++)
      if (_flag.length() > 0 &&
        combinedSwitches[i] == _flag[0] &&
        _flag[0] != Arg::flagStartString()[0])
      {
        // update the combined switches so this one is no longer present
        // this is necessary so that no unlabeled args are matched
        // later in the processing.
        //combinedSwitches.erase(i,1);
        combinedSwitches[i] = Arg::blankChar();
        return true;
      }

    // none of the switches passed in the list match. 
    return false;
  }

  // Taken from TCLAP::SwitchArg
  bool FuzzyValueSwitchArgBool::lastCombined(std::string& combinedSwitches)
  {
    for (unsigned int i = 1; i < combinedSwitches.length(); i++)
      if (combinedSwitches[i] != Arg::blankChar())
        return false;

    return true;
  }
}
