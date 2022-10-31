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

#include <ftxui/dom/elements.hpp>

#include <sstream>
#include <iomanip>

namespace ftxui
{

Element bytes(const std::string &data)
{
  std::stringstream stream;

  for(auto byte: data)
  {
    stream << std::setw(2)
           << std::setfill('0')
           << std::hex
           //Explanation since printing "char" as hex doesn't work as (i) expected.
           //i've had to cast it to int and maskout added bytes, hence anding with 0xFF
           << (0xFF & byte)
           << " ";
  }

  auto byte_arr = stream.str();

  return paragraph(byte_arr);
}

}
