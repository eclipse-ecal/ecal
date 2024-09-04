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

#include "ecal_file_resource.h"

#include <cstdarg>
#include <sstream>
#include <vector>

namespace eCAL
{
  namespace Logging
  {
    bool FileResource::fopen(const std::string& filename, std::ios::openmode mode)
    {
      m_file.open(filename, mode);

      return m_file.is_open();
    }

    void FileResource::fprintf(const char* format, ...) 
    {
      if (!m_file.is_open())
      {
        return;
      } 

      va_list args;
      va_start(args, format);

      // Determine the size of the formatted string
      va_list args_copy;
      va_copy(args_copy, args);
      int size = vsnprintf(nullptr, 0, format, args_copy);
      va_end(args_copy);

      if (size < 0)
      {
        va_end(args);
        return;
      }

      // Create a buffer and format the string into it
      std::vector<char> buffer(size + 1);
      vsnprintf(buffer.data(), buffer.size(), format, args);
      va_end(args);

      // Write the formatted string to the file
      m_file << buffer.data();
    }

    void FileResource::fflush() 
    {
      if (m_file.is_open())
      {
        m_file.flush();
      }
    }

    void FileResource::fclose() 
    {
      m_file.close();
    }

    bool FileResource::isOpen() const
    {
      return m_file.is_open();
    }
  }
}
