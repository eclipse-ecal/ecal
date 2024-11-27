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

#include "common_generate.h"

#include <cstdlib>
#include <string>
#include <vector>

namespace eCAL
{
  // Generate a random string of given length (may contain null character)
  std::string GenerateString(size_t length)
  {
    const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string result;
    result.reserve(length);

    for (size_t i = 0; i < length; ++i) {
      // use the full charset, including null character
      result.push_back(charset[rand() % (charset.length() + 1)]);
    }

    return result;
  }
  
  std::vector<char> GenerateRandomVector(size_t length)
  {
    std::vector<char> result;
    result.reserve(length);

    for (size_t i = 0; i < length; ++i)
    {
      char random_byte = rand() % 256; // Generates a random byte (0-255)
      result.push_back(random_byte);
    }

    return result;
  }
}
