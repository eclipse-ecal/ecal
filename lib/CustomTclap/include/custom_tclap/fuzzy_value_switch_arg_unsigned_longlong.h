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

#include <tclap/Arg.h>
#include <string>

namespace CustomTclap
{
  /**
   * @brief A Combination of a SwitchArg and a ValueArg<bool> with auto-detection
   *
   * The FuzzyValueSwitchArgUnsignedLongLong aims to combine the advantages of the
   * TCLAP::SwitchArg and the TCLAP::ValueArg. It is autodetected whether the
   * parameter becomes a SwitchArg (no following value) or a ValueArg (The user
   * specified the value on command line):
   *
   *     - VALUE MODE: The user manually provides a value
   *         - When the user provides a value that can be converted to an
   *           unsigned long long value, this value that can be retrieved by
   *           ::getValue().
   *
   *         Example: my_prog --setting1 42 --setting2 -1234567899
   *
   *     - SWITCH MODE: The user does not provide a value.
   *         - When encountering the parameter without any unsigned long long 
   *           parsable value, it is assumed that the user wants to turn the
   *           switch on, just like a normal SwitchArg. The value retrieved by
   *           ::getValue() will be the default value.
   *         - The short version of the argument can be combined with other
   *           switches. In that case, manually providing a value is not
   *           supported.
   *
   *         Example: my_prog --setting1 -bc
   *
   */
  class FuzzyValueSwitchArgUnsignedLongLong : public TCLAP::Arg
  {
  public:
    FuzzyValueSwitchArgUnsignedLongLong(const std::string&  flag
                                      , const std::string&  name
                                      , const std::string&  desc
                                      , bool                req
                                      , unsigned long long  value
                                      , const std::string&  type_desc
                                      , TCLAP::Visitor*     v = NULL);

    FuzzyValueSwitchArgUnsignedLongLong(const std::string&        flag
                                      , const std::string&        name
                                      , const std::string&        desc
                                      , bool                      req
                                      , unsigned long long        value
                                      , const std::string&        type_desc
                                      , TCLAP::CmdLineInterface&  parser
                                      , TCLAP::Visitor*           v = NULL);

    virtual bool processArg(int *i, std::vector<std::string> &args) override;

  private:
    bool combinedSwitchesMatch(std::string& combinedSwitches);
    bool lastCombined(std::string& combinedSwitches);

  public:
    /**
    * Specialization of shortID.
    * \param val - value to be used.
    */
    virtual std::string shortID(const std::string& val = "val") const override;

    /**
    * Specialization of longID.
    * \param val - value to be used.
    */
    virtual std::string longID(const std::string& val = "val") const override;


    /**
     * @brief Checks whether the optional value was set by the user
     *
     * @return Whether the (optional) value was set by the user
     */
    bool isValueSet() const;

    /**
     * @brief Getter for the (optional) value
     *
     * @return The user-set value or the default value, if the user didn't set it
     */
    unsigned long long getValue() const;

  private:
    bool                _is_value_set;
    unsigned long long  _value;
    unsigned long long  _default_value;
    const std::string   _type_desc;
  };
}
