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

#include <algorithm>
#include <sstream>
#include <cctype> // std::isspace

namespace EcalUtils
{
  namespace String
  {
    /// Replace all occurrences of from (which must not be the empty string)
    /// in str with to, starting at position start.
    template <class S>
    S Replace(const S& str, const S& from, const S& to, typename S::size_type start = 0)
    {
      S result;
      typename S::size_type pos = 0;
      result.append(str, 0, start);
      do
      {
        pos = str.find(from, start);
        if (pos != S::npos)
        {
          result.append(str, start, pos - start);
          result.append(to);
          start = pos + from.length();
        }
        else result.append(str, start, str.size() - start);
      } while (pos != S::npos);

      return result;
    }

    inline bool Icharcompare(char a, char b)
    {
      return(toupper(a) == toupper(b));
    }

    inline bool Icompare(const std::string& s1, const std::string& s2)
    {
      return((s1.size() == s2.size()) &&
        equal(s1.begin(), s1.end(), s2.begin(), Icharcompare));
    }

    template<typename Container>
    auto FindCaseInsensitive(const Container& container, const std::string& value)
    {
      typename Container::iterator it = container.begin();
      while (it != container.end())
      {
        if (Icompare(*it, value))
        {
          break;
        }
        ++it;
      }
      return it;
    }

    template<typename Container>
    void Split(const std::string& str, const std::string& delim, Container& parts)
    {
      size_t end = 0;
      while (end < str.size())
      {
        size_t start = end;
        while (start < str.size() && (delim.find(str[start]) != std::string::npos))
        {
          start++;
        }
        end = start;
        while (end < str.size() && (delim.find(str[end]) == std::string::npos))
        {
          end++;
        }
        if (end - start != 0)
        {
          parts.insert(parts.end(), std::string(str, start, end - start));
        }
      }
    }

    template<typename Container>
    void SplitQuotedString(const std::string& str, Container& parts, char escape_char = '\\', bool remove_quotes = true, bool keep_empty_strings = false)
    {
      enum State
      {
        NONE,
        STRING,
        QUOTED_STRING
      };

      State current_state = State::NONE;
      size_t pos = 0;

      std::string current_string;
      current_string.reserve(str.size()); // Reserve enough memory so we never need to resize the string

      while (pos < str.size())
      {
        if (current_state == State::NONE)
        {
          if (!std::isspace(static_cast<unsigned char>(str[pos])))
          {
            if (str[pos] == '\"')
            {
              // start quoted string
              current_state = State::QUOTED_STRING;
              if (!remove_quotes)
              {
                // Only copy the quote if the user wants it to remain part of the output
                current_string += str[pos];
              }
            }
            else if ((pos < (str.size() - 1) && (str[pos] == escape_char))
              && ((str[pos + 1] == '\"') || (str[pos + 1] == escape_char)))
            {
              current_state = State::STRING;
              pos++; // skip escape character and copy the next char
              current_string += str[pos];
            }
            else
            {
              current_state = State::STRING;
              // copy the current char
              current_string += str[pos];
            }
          }
        }
        else if (current_state == State::STRING)
        {
          if (std::isspace(static_cast<unsigned char>(str[pos])))
          {
            current_state = State::NONE;
            parts.insert(parts.end(), current_string);
            current_string.clear();
          }
          else if (str[pos] == '\"')
          {
            // start quoted string
            current_state = State::QUOTED_STRING;
            if (!remove_quotes)
            {
              // Only copy the quote if the user wants it to remain part of the output
              current_string += str[pos];
            }
          }
          else if ((pos < (str.size() - 1) && (str[pos] == escape_char))
            && ((str[pos + 1] == '\"') || (str[pos + 1] == escape_char)))
          {
            pos++; // skip escape character and copy the next char
            current_string += str[pos];
          }
          else
          {
            // copy the current char
            current_string += str[pos];
          }
        }
        else if (current_state == State::QUOTED_STRING)
        {
          if (str[pos] == '\"')
          {
            // end the quoted string
            current_state = State::STRING;
            if (!remove_quotes)
            {
              // Only copy the quote if the user wants it to remain part of the output
              current_string += str[pos];
            }
          }
          else if ((pos < (str.size() - 1) && (str[pos] == escape_char))
            && ((str[pos + 1] == '\"') || (str[pos + 1] == escape_char)))
          {
            pos++; // skip escape character and copy the next char
            current_string += str[pos];
          }
          else
          {
            // copy the current char
            current_string += str[pos];
          }
        }

        pos++;
      }

      if (!current_string.empty() || keep_empty_strings)
      {
        // Insert the last element
        parts.insert(parts.end(), current_string);
      }
    }

    inline std::string Trim(const std::string &s)
    {
      std::string sCopy(s);
      sCopy.erase(sCopy.begin(), std::find_if_not(sCopy.begin(), sCopy.end(), [](char c) { return std::isspace(static_cast<unsigned char>(c)); }));
      sCopy.erase(std::find_if_not(sCopy.rbegin(), sCopy.rend(), [](char c) { return std::isspace(static_cast<unsigned char>(c)); }).base(), sCopy.end());
      return sCopy;
    }

    template<typename Container>
    inline std::string Join(const std::string &delim, const Container& parts)
    {
      std::stringstream ss;
      for (auto i = parts.begin(); i != parts.end(); i++)
      {
        if (i != parts.begin())
        {
          ss << delim;
        }
        ss << *i;
      }
      return ss.str();
    }

    inline bool CenterString(std::string& str, char padding_char, size_t max_size)
    {
      if (str.length() >= max_size) return false;

      size_t empty_space = max_size - str.length();
      size_t left_padding = empty_space / 2;

      str.insert(0, left_padding, padding_char);
      str.insert(str.length(), empty_space - left_padding, padding_char);

      return true;
    }

    inline std::string htmlEscape(const std::string& str)
    {
      std::string buffer;
      buffer.reserve(str.size());
      for (size_t pos = 0; pos != str.size(); ++pos) {
        switch (str[pos]) {
        case '&':  buffer.append("&amp;");       break;
        case '\"': buffer.append("&quot;");      break;
        case '\'': buffer.append("&apos;");      break;
        case '<':  buffer.append("&lt;");        break;
        case '>':  buffer.append("&gt;");        break;
        default:   buffer.append(&str[pos], 1); break;
        }
      }
      return buffer;
    }
  }
}
