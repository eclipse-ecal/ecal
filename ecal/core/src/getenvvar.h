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

/**
 * @brief  Read the content of an environment variable
**/

#pragma once

#include <string>
#include <cstdlib>
#include <sstream>

#include <ecal_utils/str_convert.h>

inline std::string getEnvVar(const std::string& key, const std::string& def = "")
{
#if _WIN32
  wchar_t*  val(nullptr);
  size_t len(0);
  errno_t err = _wdupenv_s(&val, &len, EcalUtils::StrConvert::Utf8ToWide(key).c_str());
  if (err)            return def;
  if (val == nullptr) return def;
  std::string ret = EcalUtils::StrConvert::WideToUtf8(val);
  free(val);
  return ret;
#else
  char* val = std::getenv(key.c_str());
  if (val == nullptr) return def;
  return std::string(val);
#endif
}

inline std::vector<std::string> splitPaths(const std::string& paths_value)
{
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream token_stream(paths_value);
#ifdef _WIN32
  const char delimiter{ ';' };
#else
  const char delimiter{ ':' };
#endif
  while (std::getline(token_stream, token, delimiter))
  {
    tokens.push_back(token);
  }
  return tokens;
};
