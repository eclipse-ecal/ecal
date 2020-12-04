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

/**
 * eCALSys configuration file
**/

#pragma once

#include <string>

class EcalsysSettings
{
 public:
  EcalsysSettings(void);
  ~EcalsysSettings(void);

  void Create(const std::string& configFile);
  void Destroy();

  // config
  std::string   AppsFilterDenied()  { return (apps_filter_denied_); }

 protected:
  void LoadIniFile(const std::string& configFile);

  bool          is_init_;
  std::string   file_name_;

  std::string   apps_filter_denied_;
};
