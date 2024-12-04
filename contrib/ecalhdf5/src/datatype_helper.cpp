/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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

#include "datatype_helper.h"

namespace eCAL
{
  namespace eh5
  {
    DataTypeInformation CreateInfo(const std::string& combined_topic_type_, const std::string& descriptor_)
    {
    eCAL::eh5::DataTypeInformation info;
    auto pos = combined_topic_type_.find(':');
    if (pos == std::string::npos)
    {
      info.name = combined_topic_type_;
      info.encoding = "";
    }
    else
    {
      info.name = combined_topic_type_.substr(pos + 1);
      info.encoding = combined_topic_type_.substr(0, pos);
    }
    info.descriptor = descriptor_;
    return info;
  }

  std::pair<std::string, std::string> FromInfo(const eCAL::eh5::DataTypeInformation& datatype_info_)
  {
    std::string combined_topic_type;
    if (datatype_info_.encoding.empty())
    {
      combined_topic_type = datatype_info_.name;
    }
    else
    {
      combined_topic_type = datatype_info_.encoding + ":" + datatype_info_.name;
    }

    return std::make_pair(combined_topic_type, datatype_info_.descriptor);
  }
    
  }
}