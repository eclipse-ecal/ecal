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

#include "table_printer.h"

// termcolor pollutes everything by including Windows.h in the header file.
#ifdef _WIN32
  #define NOMINMAX
  #define WIN32_LEAN_AND_MEAN
#endif // _WIN32

#ifdef _MSC_VER
  #pragma warning(push)
  #pragma warning(disable: 4800) // disable termcolor warnings
#endif

  #include <termcolor/termcolor.hpp>

#ifdef _MSC_VER
  #pragma warning(pop)
#endif

#include <assert.h>
#include <algorithm>
#include <iomanip>

namespace eCAL
{
  namespace rec_cli
  {
    namespace table_printer
    {
      void printTable(const std::vector<std::vector<TableEntry>>& table, std::ostream& ostream)
      {
        assert((table.size() > 0));
        std::vector<size_t> max_sizes(table[0].size());
        
        // compute width of all columns
        for (size_t row = 0; row < table.size(); row++)
        {
          for (size_t col = 0; col < table[0].size(); col++)
          {
            max_sizes[col] = std::max(max_sizes[col], table[row][col].content.size());
          }
        }

        // Print rows
        for (size_t row = 0; row < table.size(); row++)
        {
          for (size_t col = 0; col < table[0].size(); col++)
          {
            const TableEntry& entry = table[row][col];

            switch (entry.text_color)
            {
            case Color::GREY:    ostream << termcolor::grey;    break;
            case Color::RED:     ostream << termcolor::red;     break;
            case Color::GREEN:   ostream << termcolor::green;   break;
            case Color::YELLOW:  ostream << termcolor::yellow;  break;
            case Color::BLUE:    ostream << termcolor::blue;    break;
            case Color::MAGENTA: ostream << termcolor::magenta; break;
            case Color::CYAN:    ostream << termcolor::cyan;    break;
            case Color::WHITE:   ostream << termcolor::white;   break;
            default:                                            break;
            }

            switch (entry.background_color)
            {
            case Color::GREY:    ostream << termcolor::on_grey;    break;
            case Color::RED:     ostream << termcolor::on_red;     break;
            case Color::GREEN:   ostream << termcolor::on_green;   break;
            case Color::YELLOW:  ostream << termcolor::on_yellow;  break;
            case Color::BLUE:    ostream << termcolor::on_blue;    break;
            case Color::MAGENTA: ostream << termcolor::on_magenta; break;
            case Color::CYAN:    ostream << termcolor::on_cyan;    break;
            case Color::WHITE:   ostream << termcolor::on_white;   break;
            default:                                               break;
            }

            switch (entry.alignment)
            {
            case Alignment::ALIGN_RIGHT: ostream << std::right; break;
            default:                                            break;
            }

            if ((entry.format_flags & FormatFlag::BOLD) != 0)
              ostream << termcolor::bold;

            if ((entry.format_flags & FormatFlag::DARK) != 0)
              ostream << termcolor::dark;

            if ((entry.format_flags & FormatFlag::ITALIC) != 0)
              ostream << termcolor::italic;

            if ((entry.format_flags & FormatFlag::UNDERLINE) != 0)
              ostream << termcolor::underline;

            if ((entry.format_flags & FormatFlag::BLINK) != 0)
              ostream << termcolor::blink;

            if ((entry.format_flags & FormatFlag::REVERSE) != 0)
              ostream << termcolor::reverse;

            if ((entry.format_flags & FormatFlag::CONCEALED) != 0)
              ostream << termcolor::concealed;

            if ((entry.format_flags & FormatFlag::CROSSED) != 0)
              ostream << termcolor::crossed;

            if ((col != table[0].size() - 1) || (entry.alignment != Alignment::ALIGN_LEFT))
            {
              // Last column does not need to be filles with spaces, if it is left-aligned
              ostream << std::setw(max_sizes[col]);
            }
            ostream << entry.content;

            // Reset to default
            ostream << termcolor::reset;
            ostream << std::left;

            if (col != table[0].size() - 1)
              ostream << " ";

          }

          ostream << std::endl;

          if (row == 0)
          {
            // Print header-underline after first row
            for (size_t i = 0; i < table[0].size() - 1; i++)
            {
              ostream << std::string(max_sizes[i], '-') << " ";
            }

            // Last column gets a special treatment, if it is left-aligned (The dashes don't expand past the header label)
            const TableEntry& last_header_entry = table[0][table[0].size() - 1];
            if (last_header_entry.alignment == Alignment::ALIGN_LEFT)
              ostream << std::string(last_header_entry.content.size(), '-');
            else
              ostream << std::string(max_sizes[table[0].size() - 1], '-');

            ostream << std::endl;
          }

        }

      }
    }
  }
}
