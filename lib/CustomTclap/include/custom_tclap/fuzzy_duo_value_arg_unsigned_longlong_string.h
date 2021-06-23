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
   * @brief An argument taking an optional unsigned long long and a mandatory string
   *
   * This argument consumes 1 to 2 followig parameters:
   *   1. An optional number that must be parsable to an unsigned long long
   *   2. A required string
   * 
   * Whether the first (optional) parameter is present is determined by trying
   * to parse it to an unsigned long long. If that works, the parameter after
   * that is used as string value. If it doesn't work, the direct tailing
   * parameter is used as string and the unsigned long long value is set to the
   * default value provided in the constructor.
   * 
   */
  class FuzzyDuoValueArgUnsignedLongLongString: public TCLAP::Arg
  {
  public:
    FuzzyDuoValueArgUnsignedLongLongString(const std::string& flag
                                         , const std::string& name
                                         , const std::string& desc
                                         , bool               req
                                         , unsigned long long ull_default_value
                                         , const std::string& string_default_value
                                         , const std::string& ull_type_desc
                                         , const std::string& string_type_desc
                                         , TCLAP::Visitor*    v = NULL);

    FuzzyDuoValueArgUnsignedLongLongString(const std::string&        flag
                                         , const std::string&        name
                                         , const std::string&        desc
                                         , bool                      req
                                         , unsigned long long        ull_default_value
                                         , const std::string&        string_default_value
                                         , const std::string&        ull_type_desc
                                         , const std::string&        string_type_desc
                                         , TCLAP::CmdLineInterface&  parser
                                         , TCLAP::Visitor*           v = NULL);

    virtual bool processArg(int *i, std::vector<std::string> &args) override;

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
     * @brief Checks whether the optional unsigned long long value was set by the user
     * 
     * @return Whether the (optional) unsigned long long value was set by the user
     */
    bool isUnsignedLongLongValueSet() const;

    /**
     * @brief Getter for the (optional) unsigned long long value
     * 
     * @return The user-set value or the default value, if the user didn't set it
     */
    unsigned long long getUnsignedLongLongValue() const;

    /**
     * @brief Getter for the (mandatory) string value
     * 
     * @return The string value set by the user
     */
    std::string getStringValue() const;

  private:
    // Is Set
    bool               _is_ulonglong_value_set;

    // Values
    unsigned long long _ulonglong_value;
    std::string        _string_value;

    // Default values
    unsigned long long _default_ulonglong_value;
    std::string        _default_string_value;

    // Descriptions
    const std::string  _ull_type_desc;
    const std::string  _string_type_desc;
  };
}
