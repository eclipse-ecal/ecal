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

/*
This class is just for convenience 
A type which supports eCAL messages needs to implement these class functions.
We'd rather need a contract, but that's C++20
*/
namespace eCAL
{
  namespace message
  {
    template <typename T>
    class MessageProvider
    {
    public:
      // unfortunately, we need an actual object for this :/
      static std::string GetTypeName(const T& /*message*/);

      // unfortunately, we need an actual object for this :/
      static std::string GetDescription(const T& /*message*/);

      static bool Serialize(const T& message, std::string& buffer);

      static bool Deserialize(const std::string& buffer, T& message);
    };
  }
}