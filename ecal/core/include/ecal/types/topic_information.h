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
 * @file   topic_information.h
 * @brief  This file contains type definitions for information associated with a given topic
**/

#pragma once
#include <string>

namespace eCAL
{
    /**
     * @brief Optional compile time information associated with a given topic
     *        (necessary for reflection / runtime type checking)
    **/
    struct STopicInformation
    {
      std::string encoding;
      std::string type;
      std::string descriptor;

      bool operator==(const STopicInformation& other) const
      {
        return encoding == other.encoding && type == other.type && descriptor == other.descriptor;
      }

      bool operator!=(const STopicInformation& other) const
      {
        return !(*this == other);
      }
    };
}
