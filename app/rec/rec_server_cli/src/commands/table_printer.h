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

#pragma once

#include <vector>
#include <string>
#include <ostream>


namespace eCAL
{
  namespace rec_cli
  {
    namespace table_printer
    {
      enum class Color
      {
        DEFAULT,
        GREY, // Black on Windows!
        RED,
        GREEN,
        YELLOW,
        BLUE,
        MAGENTA,
        CYAN,
        WHITE,
      };

      enum class Alignment
      {
        ALIGN_LEFT,
        ALIGN_RIGHT,
      };

      namespace FormatFlag
      {
        constexpr int DEFAULT   = 0;
        constexpr int BOLD      = (1 << 0);
        constexpr int DARK      = (1 << 1);
        constexpr int ITALIC    = (1 << 2);
        constexpr int UNDERLINE = (1 << 3); // Supported on Windows
        constexpr int BLINK     = (1 << 4);
        constexpr int REVERSE   = (1 << 5);
        constexpr int CONCEALED = (1 << 6);
        constexpr int CROSSED   = (1 << 7);
      };

      struct TableEntry
      {
        TableEntry(const std::string& text, Alignment alignment, Color text_color, Color background_color, int format_flags)
          : content         (text)
          , text_color      (text_color)
          , background_color(background_color)
          , alignment       (alignment)
          , format_flags    (format_flags)
        {}

        TableEntry(const std::string& text = "")
          : TableEntry(text, Alignment::ALIGN_LEFT, Color::DEFAULT, Color::DEFAULT, FormatFlag::DEFAULT)
        {}

        TableEntry(const std::string& text, Alignment alignment)
          : TableEntry(text, alignment, Color::DEFAULT, Color::DEFAULT, FormatFlag::DEFAULT)
        {}

        TableEntry(const std::string& text, Color text_color, Color background_color)
          : TableEntry(text, Alignment::ALIGN_LEFT, text_color, background_color, FormatFlag::DEFAULT)
        {}

        TableEntry(const std::string& text, int format_flags)
          : TableEntry(text, Alignment::ALIGN_LEFT, Color::DEFAULT, Color::DEFAULT, format_flags)
        {}

        std::string content;
        Color       text_color;
        Color       background_color;
        Alignment   alignment;
        int         format_flags;
      };
      
      void printTable(const std::vector<std::vector<TableEntry>>& table, std::ostream& ostream);
    }
  }
}