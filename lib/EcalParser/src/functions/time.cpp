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

#include "time.h"

#include <set>
#include <iomanip>
#include <sstream>
#include <utility>
#include <vector>

#include <ecal_utils/string.h>

namespace EcalParser
{
  std::string FunctionTime::Evaluate(const std::string& parameters, std::chrono::system_clock::time_point time) const
  {
    std::string format_string;
    if (parameters == "")
    {
      format_string = "%F_%H-%M-%S.%.m";
    }
    else
    {
      format_string = parameters;
    }

    time_t time_t_time = std::chrono::system_clock::to_time_t(time);
    
    static const std::set<char> allowed_strftime_chars
    {
      'a', 'A', 'b', 'B', 'c', 'C', 'd', 'D', 'e', 'F', 'g', 'G', 'h', 'H', 'I', 'j', 'm', 'M', 'n', 'p', 'r', 'R', 'S', 't', 'T', 'u', 'U', 'V', 'w', 'W', 'x', 'X', 'y', 'Y', 'z', 'Z', '%'
    };
    
    std::string output;
    
    size_t pos = 0;
    while (pos < format_string.size())
    {
      if (format_string.at(pos) == '%')
      {
        // 1-char custom formats
        if ((pos + 1) < format_string.size())
        {
          // Seconds since epoch
          if (format_string.at(pos + 1) == 's')
          {
            output += std::to_string(std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count());
            pos += 2;
            continue;
          }
        }
    
        // 2-char custom formats
        if ((pos + 2) < format_string.size())
        {
          // milli/micro/nanoseconds
          if (format_string.at(pos + 1) == '.')
          {
            bool parsed_successfully = false;
            std::stringstream ss;
            std::chrono::nanoseconds ns_to_full_second = std::chrono::duration_cast<std::chrono::nanoseconds>(time.time_since_epoch() - std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()));
    
            // Milliseconds to full second (3 digits, zero padded)
            if (format_string.at(pos + 2) == 'm')
            {
              ss << std::internal << std::setfill('0') << std::setw(3) << std::chrono::duration_cast<std::chrono::milliseconds>(ns_to_full_second).count();
              parsed_successfully = true;
            }
            // Microseconds to full second (6 digits, zero padded)
            else if (format_string.at(pos + 2) == 'u')
            {
              ss << std::internal << std::setfill('0') << std::setw(6) << std::chrono::duration_cast<std::chrono::microseconds>(ns_to_full_second).count();
              parsed_successfully = true;
            }
            // Nanoseconds to full second (9 digits, zero padded)
            else if (format_string.at(pos + 2) == 'n')
            {
              ss << std::internal << std::setfill('0') << std::setw(9) << std::chrono::duration_cast<std::chrono::nanoseconds>(ns_to_full_second).count();
              parsed_successfully = true;
            }
    
            if (parsed_successfully)
            {
              output += ss.str();
              pos += 3;
              continue;
            }
          }
        }
    
        // strftime evaluation
        if (((pos + 1) < format_string.size())
          && (allowed_strftime_chars.find(format_string.at(pos + 1)) != allowed_strftime_chars.end()))
        {
          char strftime_output[64];
          size_t strftime_output_size = strftime(strftime_output, 64, format_string.substr(pos, 2).c_str(), localtime(&time_t_time));
          output += std::string(strftime_output, strftime_output_size);
          pos += 2;
    
          continue;
        }
      }
    
      output += format_string.at(pos);
      pos++;
    }
    return output;
  }

  std::string FunctionTime::ParameterUsage   () const { return "<FormatString>"; }

  std::string FunctionTime::ParameterExample() const { return "%F, %R"; }
  std::string FunctionTime::Description     () const { return "Time and Date"; }

  std::string FunctionTime::HtmlDocumentation        () const
  {
    auto now = std::chrono::system_clock::now();

    static const std::vector<std::pair<std::string, std::string>> format_options = 
    {
      { "%a", "Abbreviated weekday name" },
      { "%A", "Full weekday name" },
      { "%b", "Abbreviated month name" },
      { "%B", "Full month name" },
      { "%c", "Date and time representation" },
      { "%C", "Year divided by 100 and truncated to integer (00-99)" },
      { "%d", "Day of the month, zero-padded (01-31)" },
      { "%D", "Short MM/DD/YY date, equivalent to %m/%d/%y" },
      { "%e", "Day of the month, space-padded ( 1-31)" },
      { "%F", "Short YYYY-MM-DD date, equivalent to %Y-%m-%d" },
      { "%g", "Week-based year, last two digits (00-99)" },
      { "%G", "Week-based year" },
      { "%h", "Abbreviated month name (same as %b)" },
      { "%H", "Hour in 24h format (00-23)" },
      { "%I", "Hour in 12h format (01-12)" },
      { "%j", "Day of the year (001-366)" },
      { "%m", "Month as a decimal number (01-12)" },
      { "%M", "Minute (00-59)" },
      { "%n", "New-line character ('\\n')" },
      { "%p", "AM or PM designation" },
      { "%r", "12-hour clock time" },
      { "%R", "24-hour HH:MM time, equivalent to %H:%M" },
      { "%s", "Seconds since epoch (1970-01-01 00:00:00 UTC)" },
      { "%S", "Second (00-61)" },
      { "%t", "Horizontal-tab character ('\t')" },
      { "%T", "ISO 8601 time format (HH:MM:SS), equivalent to %H:%M:%S" },
      { "%u", "ISO 8601 weekday as number with Monday as 1 (1-7)" },
      { "%U", "Week number with the first Sunday as the first day of week one (00-53)" },
      { "%V", "ISO 8601 week number (01-53)" },
      { "%w", "Weekday as a decimal number with Sunday as 0 (0-6)" },
      { "%W", "Week number with the first Monday as the first day of week one (00-53)" },
      { "%x", "Date representation" },
      { "%X", "Time representation" },
      { "%y", "Year, last two digits (00-99)" },
      { "%Y", "Year" },
      { "%z", "ISO 8601 offset from UTC in timezone (1 minute=1, 1 hour=100). If timezone cannot be determined, no characters" },
      { "%Z", "Timezone name or abbreviation. If timezone cannot be determined, no characters" },
      { "%%", "A % sign" },
      { "%.m", "Millisconds since full second, zero-padded (000-999)" },
      { "%.u", "Microseconds since full second, zero-padded (000000-999999)" },
      { "%.n", "Nanoseconds since full second, zero-padded (000000000-999999999)" },
    };

    std::stringstream ss;
    ss << R"(
<p>
Parses the format string with extended strftime replacement. If no format string is given, it will default to "<tt>%F_%H-%M-%S.%.m</tt>"
</p>
<p>
The following specifiers are supported:
  <table style="border-width:1px; border-style:solid; border-color:#000000; margin-top:5px; margin-bottom:5px; margin-left:10px; margin-right:10px;" align="left" cellspacing="0" cellpadding="2">
  <tr>
    <th>Specifier</th>
    <th>Desctription</th>
    <th>Example</th>
  </tr>
)";

    for (const auto& format : format_options)
    {
      ss << "<tr>";
      ss << "<td>" << format.first << "</td>";
      ss << "<td>" << EcalUtils::String::htmlEscape(format.second) << "</td>";
      ss << "<td>" << EcalUtils::String::htmlEscape(Evaluate(format.first, now)) << "</td>";
      ss << "</tr>";
    }

    ss << R"(
</table> 
</p>
)";
    return ss.str();
  }
}