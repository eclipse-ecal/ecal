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

#include <simpleini_template.h>

/*
* This file provides two methods to access the Simple Inifile Setter/Getters on a template basis.
*/
template<>
bool GetValue<bool>(const CSimpleIni& ini_, const std::string& section_, const std::string& key_, const bool& default_)
{
  return ini_.GetBoolValue(section_.c_str(), key_.c_str(), default_);
}

template<>
SI_Error SetValue<bool>(CSimpleIni& ini_, const std::string& section_, const std::string& key_, const bool& value_, const std::string& comment_, bool force_overwrite_)
{
  return ini_.SetBoolValue(section_.c_str(), key_.c_str(), value_, comment_.c_str(), force_overwrite_);
}

template<>
std::string GetValue<std::string>(const CSimpleIni& ini_, const std::string& section_, const std::string& key_, const std::string& default_)
{
  return std::string(ini_.GetValue(section_.c_str(), key_.c_str(), default_.c_str()));
}

template<>
SI_Error SetValue<std::string>(CSimpleIni& ini_, const std::string& section_, const std::string& key_, const std::string& value_, const std::string& comment_, bool force_overwrite_)
{
  return ini_.SetValue(section_.c_str(), key_.c_str(), value_.c_str(), comment_.c_str(), force_overwrite_);
}

template<>
double GetValue<double>(const CSimpleIni& ini_, const std::string& section_, const std::string& key_, const double& default_)
{
  return ini_.GetDoubleValue(section_.c_str(), key_.c_str(), default_);
}

template<>
SI_Error SetValue<double>(CSimpleIni& ini_, const std::string& section_, const std::string& key_, const double& value_, const std::string& comment_, bool force_overwrite_)
{
  return ini_.SetDoubleValue(section_.c_str(), key_.c_str(), value_, comment_.c_str(), force_overwrite_);
}

template<>
float GetValue<float>(const CSimpleIni& ini_, const std::string& section_, const std::string& key_, const float& default_)
{
  return static_cast<float>(ini_.GetDoubleValue(section_.c_str(), key_.c_str(), default_));
}

template<>
SI_Error SetValue<float>(CSimpleIni& ini_, const std::string& section_, const std::string& key_, const float& value_, const std::string& comment_, bool force_overwrite_)
{
  return ini_.SetDoubleValue(section_.c_str(), key_.c_str(), value_, comment_.c_str(), force_overwrite_);
}

template<>
int32_t GetValue<int32_t>(const CSimpleIni& ini_, const std::string& section_, const std::string& key_, const int32_t& default_)
{
  return ini_.GetLongValue(section_.c_str(), key_.c_str(), default_);
}

template<>
SI_Error SetValue<int32_t>(CSimpleIni& ini_, const std::string& section_, const std::string& key_, const int32_t& value_, const std::string& comment_, bool force_overwrite_)
{
  return ini_.SetLongValue(section_.c_str(), key_.c_str(), value_, comment_.c_str(), force_overwrite_);
}