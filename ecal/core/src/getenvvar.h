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

inline std::string getEnvVar(const std::string& key, const std::string& def = "")
{
#if _WIN32
  char*  val(nullptr);
  size_t len(0);
  errno_t err = _dupenv_s(&val, &len, key.c_str());
  if (err)            return def;
  if (val == nullptr) return def;
  std::string ret(val);
  free(val);
  return ret;
#else
  char* val = std::getenv(key.c_str());
  if (val == nullptr) return def;
  return std::string(val);
#endif
}
