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

#pragma once

#include <string>
#include <unordered_map>

namespace eCAL
{
  namespace Util
  {
    class StringReplacer
    {
      public:
        StringReplacer();
        
        std::string Replace(const std::string& str_);

        void SetMap(const std::unordered_map<std::string, std::string>& map_);

      private:
        std::unordered_map<std::string, std::string> m_replacer_map;
    };
  }
}