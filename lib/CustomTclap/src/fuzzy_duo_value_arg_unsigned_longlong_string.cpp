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

#include <custom_tclap/fuzzy_duo_value_arg_unsigned_longlong_string.h>

#include <algorithm>
#include <cctype>

namespace CustomTclap
{
  FuzzyDuoValueArgUnsignedLongLongString::FuzzyDuoValueArgUnsignedLongLongString
        (
            const std::string& flag
          , const std::string& name
          , const std::string& desc
          , bool               req
          , unsigned long long ull_default_value
          , const std::string& string_default_value
          , const std::string& ull_type_desc
          , const std::string& string_type_desc
          , TCLAP::Visitor*    v
        )

    : TCLAP::Arg(flag, name, desc, req, true, v)
    , _is_ulonglong_value_set (false)
    , _ulonglong_value        (ull_default_value)
    , _default_ulonglong_value(ull_default_value)
    , _default_string_value   (string_default_value)
    , _ull_type_desc          (ull_type_desc)
    , _string_type_desc       (string_type_desc)
  {
    _acceptsMultipleValues = true;
  }

  FuzzyDuoValueArgUnsignedLongLongString::FuzzyDuoValueArgUnsignedLongLongString
        (
            const std::string&        flag
          , const std::string&        name
          , const std::string&        desc
          , bool                      req
          , unsigned long long        ull_default_value
          , const std::string&        string_default_value
          , const std::string&        ull_type_desc
          , const std::string&        string_type_desc
          , TCLAP::CmdLineInterface&  parser
          , TCLAP::Visitor*           v
        )

    : TCLAP::Arg(flag, name, desc, req, true, v)
    , _is_ulonglong_value_set (false)
    , _ulonglong_value        (ull_default_value)
    , _default_ulonglong_value(ull_default_value)
    , _default_string_value   (string_default_value)
    , _ull_type_desc          (ull_type_desc)
    , _string_type_desc       (string_type_desc)
  {
    parser.add(this);
    _acceptsMultipleValues = true;
  }

  bool FuzzyDuoValueArgUnsignedLongLongString::processArg(int *i, std::vector<std::string> &args)
  {
    if (_ignoreable && Arg::ignoreRest())
    {
      return false;
    }

    if (_hasBlanks(args[*i]))
      return false;

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
        // No tailing parameter => No match
        throw(TCLAP::ArgParseException("Missing a value for this argument!", toString()));
      }
      else if ((*i + 2) == (int)args.size())
      {
        // Only 1 tailing parameter => this is the string parameter
        (*i)++;
        _string_value = args[*i];

        _is_ulonglong_value_set = false;

      }
      else
      {
        // We have 2 or more tailing parameters => decide whether we have a unsigned long-long parsable value
        try
        {
          _ulonglong_value = std::stoull(args[*i + 1]);
          _string_value    = args[*i + 2];

          _is_ulonglong_value_set = true;

          (*i) += 2;

          // -> successfull!
        }
        catch (const std::exception&)
        {
          // -> failed to convert to unsigned long long. Falling back to default and taking the next arg as string value
          _ulonglong_value = _default_ulonglong_value;
          _string_value    = args[*i + 1];

          _is_ulonglong_value_set = false;

          (*i) += 1;
        }
      }

      _alreadySet = true;
      _checkWithVisitor();
      return true;
    }	
    else
    {
      return false;
    }
  }

  /**
  * Specialization of shortID.
  * \param val - value to be used.
  */
  std::string FuzzyDuoValueArgUnsignedLongLongString::shortID(const std::string& /*val*/) const
  {
    std::string id = "";

    if ( _flag != "" )
      id = Arg::flagStartString() + _flag;
    else
      id = Arg::nameStartString() + _name;

    id += std::string(1, Arg::delimiter()) + "[" + _ull_type_desc  + "]" + std::string(1, Arg::delimiter()) + "<" + _string_type_desc  + ">";

    if ( !_required )
      id = "[" + id + "]";

    return id;
  }

  /**
  * Specialization of longID.
  * \param val - value to be used.
  */
  std::string FuzzyDuoValueArgUnsignedLongLongString::longID(const std::string& /*val*/) const
  {
    std::string id = "";

    if ( _flag != "" )
    {
      id += Arg::flagStartString() + _flag;

      if ( _valueRequired )
        id += std::string(1, Arg::delimiter()) + "[" + _ull_type_desc  + "]" + std::string(1, Arg::delimiter()) + "<" + _string_type_desc  + ">";

      id += ",  ";
    }

    id += Arg::nameStartString() + _name;

    if ( _valueRequired )
      id += std::string(1, Arg::delimiter()) + "[" + _ull_type_desc  + "]" + std::string(1, Arg::delimiter()) + "<" + _string_type_desc  + ">";

    return id;

  }

  bool FuzzyDuoValueArgUnsignedLongLongString::isUnsignedLongLongValueSet() const
  {
    return _is_ulonglong_value_set;
  }

  unsigned long long FuzzyDuoValueArgUnsignedLongLongString::getUnsignedLongLongValue() const
  {
    return _ulonglong_value;
  }

  std::string FuzzyDuoValueArgUnsignedLongLongString::getStringValue() const
  {
    return _string_value;
  }
}