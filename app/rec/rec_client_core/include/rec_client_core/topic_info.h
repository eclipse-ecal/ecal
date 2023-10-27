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

#include <string>
#include <set>
#include <map>
#include <ecal/ecal_types.h>

namespace eCAL
{
  namespace rec
  {
    struct TopicInfo
    {
      TopicInfo(const eCAL::SDataTypeInformation& tinfo)
        : tinfo_(tinfo)
      {}

      TopicInfo(const std::string& type, const std::string& encoding, const std::string& description)
        : tinfo_{ type, encoding, description }
      {}

      TopicInfo() 
      {}

      eCAL::SDataTypeInformation tinfo_;

      int description_quality_ = 0;

      std::map<std::string, std::set<std::string>> publishers_;
    };
  }
}