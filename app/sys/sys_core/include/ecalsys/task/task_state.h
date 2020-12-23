/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
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

#include <ecal/ecal.h>
#include <string>
#include <algorithm>
#include <cmath>
#include <sstream>

class TaskState
{
public:
  eCAL_Process_eSeverity        severity;             /**< The severity (Unknown, Healthy, Warning, Critical, Failed) */
  eCAL_Process_eSeverity_Level  severity_level;       /**< The severity Level (Level 1 - 5) */
  std::string                   info;                 /**< A human readable status message */

  /**
   * @brief Creates a new State (Unknown Level1) without info message.
   */
  TaskState()
    : severity(eCAL_Process_eSeverity::proc_sev_unknown)
    , severity_level(eCAL_Process_eSeverity_Level::proc_sev_level1)
    , info("")
  {}

  /**
   * @brief Creates a new State with the given severity and without info message.
   * @param severity        The severity of the new state
   * @param severity_level  The severity level of the new state
   */
  TaskState(eCAL_Process_eSeverity severity, eCAL_Process_eSeverity_Level severity_level)
    : severity(severity)
    , severity_level(severity_level)
    , info("")
  {}

  /**
   * @brief Creates a new State by parsing the given combined severity and without info message. @see{FromInt()}
   * @param combined_severity The combines severity / severity level as integer
   */
  TaskState(int combined_severity)
    : info("")
  {
    FromInt(combined_severity);
  }

  /**
   * @brief Parses the given strings and sets the severity and severity level accordingly
   *
   * The Strings are case-insensitive and can be:
   *  Severity:       Unknown, Healthy, Warning, Critical, Failed
   *  Severity Level: Level1, Level2, Level3, Level4, Level5
   *                       ^ (there may be a space after each "Level")
   *
   * The default is Unknown Level1, if the string could not be parsed.
   *
   * @param severity_string The Severity as string
   * @param severity_level_string The Severity Level as string
   *
   * @return wether both strings could be parsed correctly
   */
  bool FromString(const std::string& severity_string, const std::string& severity_level_string)
  {
    std::string severity_string_lower      (severity_string);
    std::string severity_level_string_lower(severity_level_string);

    // cause warning C4244 with VS2017
    //std::transform(severity_string_lower.begin(),       severity_string_lower.end(),       severity_string_lower.begin(),       ::tolower);
    //std::transform(severity_level_string_lower.begin(), severity_level_string_lower.end(), severity_level_string_lower.begin(), ::tolower);
    std::transform(severity_string_lower.begin(), severity_string_lower.end(), severity_string_lower.begin(),
      [](char c) {return static_cast<char>(::tolower(c)); });
    std::transform(severity_level_string_lower.begin(), severity_level_string_lower.end(), severity_level_string_lower.begin(),
      [](char c) {return static_cast<char>(::tolower(c)); });

    bool failure = false;

    if (severity_string_lower == "unknown")
    {
      severity = eCAL_Process_eSeverity::proc_sev_unknown;
    }
    else if (severity_string_lower == "healthy")
    {
      severity = eCAL_Process_eSeverity::proc_sev_healthy;
    }
    else if (severity_string_lower == "warning")
    {
      severity = eCAL_Process_eSeverity::proc_sev_warning;
    }
    else if (severity_string_lower == "critical")
    {
      severity = eCAL_Process_eSeverity::proc_sev_critical;
    }
    else if (severity_string_lower == "failed")
    {
      severity = eCAL_Process_eSeverity::proc_sev_failed;
    }
    else
    {
      severity = eCAL_Process_eSeverity::proc_sev_unknown;
      failure = true;
    }

    if (severity_level_string_lower == "level1" || severity_level_string_lower == "level 1")
    {
      severity_level = eCAL_Process_eSeverity_Level::proc_sev_level1;
    }
    else if (severity_level_string_lower == "level2" || severity_level_string_lower == "level 2")
    {
      severity_level = eCAL_Process_eSeverity_Level::proc_sev_level2;
    }
    else if (severity_level_string_lower == "level3" || severity_level_string_lower == "level 3")
    {
      severity_level = eCAL_Process_eSeverity_Level::proc_sev_level3;
    }
    else if (severity_level_string_lower == "level4" || severity_level_string_lower == "level 4")
    {
      severity_level = eCAL_Process_eSeverity_Level::proc_sev_level4;
    }
    else if (severity_level_string_lower == "level5" || severity_level_string_lower == "level 5")
    {
      severity_level = eCAL_Process_eSeverity_Level::proc_sev_level5;
    }
    else
    {
      severity_level = eCAL_Process_eSeverity_Level::proc_sev_level1;
      failure = true;
    }

    return !failure;
  }

  /**
   * @brief Converts the Severity and Severity Level to a string
   *
   * @param severity_string The object to save the string representation of the severity to
   * @param severity_level_string The object to save the string representation of the severity level to
   */
  void ToString(std::string& severity_string, std::string& severity_level_string) const
  {
    switch (severity_level)
    {
    case eCAL_Process_eSeverity_Level::proc_sev_level1:
      severity_level_string = "Level1";
      break;
    case eCAL_Process_eSeverity_Level::proc_sev_level2:
      severity_level_string = "Level2";
      break;
    case eCAL_Process_eSeverity_Level::proc_sev_level3:
      severity_level_string = "Level3";
      break;
    case eCAL_Process_eSeverity_Level::proc_sev_level4:
      severity_level_string = "Level4";
      break;
    case eCAL_Process_eSeverity_Level::proc_sev_level5:
      severity_level_string = "Level5";
      break;
    default:
      severity_level_string = "";
    }

    switch (severity)
    {
    case eCAL_Process_eSeverity::proc_sev_unknown:
      severity_string = "Unknown";
      break;
    case eCAL_Process_eSeverity::proc_sev_healthy:
      severity_string = "Healthy";
      break;
    case eCAL_Process_eSeverity::proc_sev_warning:
      severity_string = "Warning";
      break;
    case eCAL_Process_eSeverity::proc_sev_critical:
      severity_string = "Critical";
      break;
    case eCAL_Process_eSeverity::proc_sev_failed:
      severity_string = "Failed";
      break;
    default:
      severity_string = "";
    }
  }

  /**
   * @brief Sets the severity and serverity_level to the value parsed from the combined severity integer.
   *
   * The combined severity can range from 0 to 24. Thus, there are 25 values
   * representing all 25 combinations of severities and severity levels:
   *
   *  0: Unknown Level0
   *  1: Unknown Level1
   *    ...
   * 23: Failed  Level4
   * 24: Failed  Level5
   *
   * @param combined_severity   The combined severity and severity level as interger
   */
  void FromInt(int combined_severity)
  {
    // We want to directly cast the severities. That will lead to funny bugs if someone eventually decides to change the ranges.
    static_assert((int)eCAL_Process_eSeverity::proc_sev_unknown == 0,      "Somebody shifted the severity values! We expect severities to have a rage 0-4 and severity levels to have a range of 1-5!");
    static_assert((int)eCAL_Process_eSeverity::proc_sev_failed  == 4,      "Somebody shifted the severity values! We expect severities to have a rage 0-4 and severity levels to have a range of 1-5!");
    static_assert((int)eCAL_Process_eSeverity_Level::proc_sev_level1 == 1, "Somebody shifted the severity values! We expect severities to have a rage 0-4 and severity levels to have a range of 1-5!");
    static_assert((int)eCAL_Process_eSeverity_Level::proc_sev_level5 == 5, "Somebody shifted the severity values! We expect severities to have a rage 0-4 and severity levels to have a range of 1-5!");

    // limit the interger to the desired range
    combined_severity = std::max(std::min(combined_severity, 24), 0);

    // calculate the interger severity / -level
    severity       = (eCAL_Process_eSeverity)      (combined_severity / 5);
    severity_level = (eCAL_Process_eSeverity_Level)(combined_severity % 5 + 1);
  }

  /**
   * @brief Converts the severity and serverity_level to one number ranging from 0 to 24.
   *
   * The combined severity can range from 0 to 24. Thus, there are 25 values
   * representing all 25 combinations of severities and severity levels:
   *
   *  0: Unknown Level0
   *  1: Unknown Level1
   *    ...
   * 23: Failed  Level4
   * 24: Failed  Level5
   *
   * @return the combined severity and severity level as int
   */
  int ToInt() const
  {
    // We want to directly cast the severities. That will lead to funny bugs if someone eventually decides to change the ranges.
    static_assert((int)eCAL_Process_eSeverity::proc_sev_unknown == 0,      "Somebody shifted the severity values! We expect severities to have a rage 0-4 and severity levels to have a range of 1-5!");
    static_assert((int)eCAL_Process_eSeverity::proc_sev_failed  == 4,      "Somebody shifted the severity values! We expect severities to have a rage 0-4 and severity levels to have a range of 1-5!");
    static_assert((int)eCAL_Process_eSeverity_Level::proc_sev_level1 == 1, "Somebody shifted the severity values! We expect severities to have a rage 0-4 and severity levels to have a range of 1-5!");
    static_assert((int)eCAL_Process_eSeverity_Level::proc_sev_level5 == 5, "Somebody shifted the severity values! We expect severities to have a rage 0-4 and severity levels to have a range of 1-5!");

    return ((int)severity * 5) + ((int)severity_level - 1);
  }

  /**
   * @brief Creates a String representation consisting of the Severity, the severity level and the info.
   *
   * Example:
   *    Healthy Level1 (I feel good)
   *
   * @return A string representation of the state
   */
  std::string ToString()
  {
    std::string severity_string;
    std::string severity_level_string;
    ToString(severity_string, severity_level_string);
    std::stringstream ss;
    ss << severity_string << " " << severity_level_string << " (" << info << ")";
    return ss.str();
  }

  /**
   * @brief Compares the severity and severity level
   */
  bool operator==(const TaskState& other) const
  {
    return ToInt() == other.ToInt();
  }

  /**
   * @brief Compares the severity and severity level
   */
  bool operator!=(const TaskState& other) const
  {
    return ToInt() != other.ToInt();
  }

  /**
   * @brief BETTER THAN operator (good < bad)
   *
   * unknown < healty < warning < critical < failed
   *
   * level1 < level2 < level3 < level4 < level5
   */
  bool operator<(const TaskState& other) const
  {
    return ToInt() < other.ToInt();
  }

  /**
   * @brief WORSE THAN operator (bad > good)
   *
   * failed > critical > warning > healthy > unknown
   *
   * level5 > level4 > level3 > level2 > level1
   */
  bool operator>(const TaskState& other) const
  {
    return ToInt() > other.ToInt();
  }

  /**
   * @brief BETTER OR EQUAL THAN oprerator (good =< bad)
   *
   * unknown < healty < warning < critical < failed
   *
   * level1 < level2 < level3 < level4 < level5
   */
  bool operator<=(const TaskState& other) const
  {
    return ToInt() <= other.ToInt();;
  }

  /**
   * @brief WORSE OR EQUAL THAN oprerator (bad >= good)
   *
   * failed > critical > warning > healthy > unknown
   *
   * level5 > level4 > level3 > level2 > level1
   */
  bool operator>=(const TaskState& other) const
  {
    return ToInt() >= other.ToInt();;
  }
};
