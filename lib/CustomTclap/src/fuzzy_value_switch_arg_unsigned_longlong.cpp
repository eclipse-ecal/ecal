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

#include <custom_tclap/fuzzy_value_switch_arg_unsigned_longlong.h>

#include <algorithm>
#include <cctype>

namespace CustomTclap
{
  FuzzyValueSwitchArgUnsignedLongLong::FuzzyValueSwitchArgUnsignedLongLong
      (
          const std::string&  flag
        , const std::string&  name
        , const std::string&  desc
        , bool                req
        , unsigned long long  default_value
        , const std::string&  type_desc
        , TCLAP::Visitor*     v
      )
    : TCLAP::Arg(flag, name, desc, req, false, v)
    , _is_value_set (false)
    , _value        (default_value)
    , _default_value(default_value)
    , _type_desc    (type_desc)
  {
    _acceptsMultipleValues = false;
  }

  FuzzyValueSwitchArgUnsignedLongLong::FuzzyValueSwitchArgUnsignedLongLong
      (
          const std::string&        flag
        , const std::string&        name
        , const std::string&        desc
        , bool                      req
        , unsigned long long        default_value
        , const std::string&        type_desc
        , TCLAP::CmdLineInterface&  parser
        , TCLAP::Visitor*           v
      )
    : TCLAP::Arg(flag, name, desc, req, false, v)
    , _is_value_set (false)
    , _value        (default_value)
    , _default_value(default_value)
    , _type_desc    (type_desc)
  {
    parser.add(this);
    _acceptsMultipleValues = false;
  }

  bool FuzzyValueSwitchArgUnsignedLongLong::processArg(int *i, std::vector<std::string> &args)
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
        // Fall back to default, if there is no following parameter
        _value = _default_value;
      }
      else
      {
        // Evaluate the next parameter
        std::string unsigned_long_long_string = args[*i + 1];

        // Try to parse to an unsigned long long
        try
        {
          _value = std::stoull(unsigned_long_long_string);
          (*i)++;
          // -> successfull!
        }
        catch (const std::exception&)
        {
          // -> failed to convert to unsigned long long. Falling back to default.
          _value = _default_value;
        }
      }

      _alreadySet = true;
      _checkWithVisitor();
      return true;
    }
    // A match in a combination cannot have a value. So we fall back to _default.
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
      _value      = _default_value;
      _checkWithVisitor();
      return lastCombined(args[*i]);
    }
    else
    {
      return false;
    }
  }

  // Taken from TCLAP::SwitchArg
  bool FuzzyValueSwitchArgUnsignedLongLong::combinedSwitchesMatch(std::string& combinedSwitches)
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
  bool FuzzyValueSwitchArgUnsignedLongLong::lastCombined(std::string& combinedSwitches)
  {
    for (unsigned int i = 1; i < combinedSwitches.length(); i++)
      if (combinedSwitches[i] != Arg::blankChar())
        return false;

    return true;
  }

  std::string FuzzyValueSwitchArgUnsignedLongLong::shortID(const std::string& /*val*/) const
  {
    std::string id = "";

    if ( _flag != "" )
        id = Arg::flagStartString() + _flag;
    else
        id = Arg::nameStartString() + _name;

    id += std::string( 1, Arg::delimiter() ) + "[" + _type_desc  + "]";

    if ( !_required )
        id = "[" + id + "]";

    return id;
  }

  std::string FuzzyValueSwitchArgUnsignedLongLong::longID(const std::string& /*val*/) const
  {
    std::string id = "";

    if ( _flag != "" )
    {
        id += Arg::flagStartString() + _flag;

        id += std::string( 1, Arg::delimiter() ) + "[" + _type_desc + "]";

        id += ",  ";
    }

    id += Arg::nameStartString() + _name;

    if ( _valueRequired )
        id += std::string( 1, Arg::delimiter() ) + "<" + _type_desc + ">";

    return id;
  }

  bool FuzzyValueSwitchArgUnsignedLongLong::isValueSet() const
  {
    return _is_value_set;
  }

  unsigned long long FuzzyValueSwitchArgUnsignedLongLong::getValue() const
  {
    return _value;
  }
}
