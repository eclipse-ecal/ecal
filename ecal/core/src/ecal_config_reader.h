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
 * @brief  Global eCAL configuration interface
**/

#pragma once

#include <ecal/ecal_os.h>

#include <string>
#include <vector>

namespace eCAL
{
  class CConfigImpl;
  class CConfig
  {
  public:
    CConfig();
    virtual ~CConfig();

    void OverwriteKeys(const std::vector<std::string>& key_vec_);
    void AddFile(std::string& ini_file_);

    bool Validate();

    // common getter
    bool        get(const std::string& section_, const std::string& key_, bool        default_);
    int         get(const std::string& section_, const std::string& key_, int         default_);
    double      get(const std::string& section_, const std::string& key_, double      default_);
    std::string get(const std::string& section_, const std::string& key_, const char* default_);

  private:
    CConfigImpl* m_impl;
  };

  ECAL_API bool        CfgGetBool  (const std::string& section_, const std::string& key_, bool        default_ = false);
  ECAL_API int         CfgGetInt   (const std::string& section_, const std::string& key_, int         default_ = 0);
  ECAL_API double      CfgGetDouble(const std::string& section_, const std::string& key_, double      default_ = 0.0);
  ECAL_API std::string CfgGetString(const std::string& section_, const std::string& key_, const char* default_ = "");
}
