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

#include "EcalParser/EcalParser.h"

#include <sstream>
#include <algorithm>
#include <list>

#include <ecal_utils/string.h>

#include "functions/env.h"
#include "functions/hostname.h"
#include "functions/os.h"
#include "functions/osselect.h"
#include "functions/time.h"


namespace EcalParser
{
  // Private
  namespace
  {
    static const char TARGET_IDENTIFIER_CHAR = '$';
    static const char FUNCTION_START_CHAR = '{';
    static const char FUNCTION_END_CHAR = '}';
    static const char ESCAPE_CHAR = '`';

    std::list<std::string> SplitIntoWords(const std::string& text);
    std::string EvaluateWordlist(std::list<std::string> word_list, bool eval_target_functions, std::chrono::system_clock::time_point time);
    
    bool IsSpecialWord(const std::string& word);
    bool IsTargetIdentifierWord(const std::string& word);
    bool IsFunctionStartWord(const std::string& word);
    bool IsFunctionEndWord(const std::string& word);
    
    bool IsTargetKeyword(const std::string& word);
    
    std::string EvaluateFunction(const std::string& function_name, const std::string& param, std::chrono::system_clock::time_point time);

    std::list<std::string> SplitIntoWords(const std::string& text)
    {
      std::list<std::string> words;
    
      size_t word_start = 0;
      size_t i = 0;
      while (i < text.size())
      {
        char current_char = text[i];
        if (current_char == ESCAPE_CHAR)
        {
          // The next char is escaped => We skip it!
          i++;
        }
        else if ((current_char == TARGET_IDENTIFIER_CHAR)
          || (current_char == FUNCTION_START_CHAR)
          || (current_char == FUNCTION_END_CHAR))
        {
          if (word_start < i)
          {
            // Copy the previous word
            words.emplace_back(text.substr(word_start, i - word_start));
          }
    
          // copy the current special word
          words.emplace_back(text.substr(i, 1));
    
          // The next word may start at the next position
          word_start = i + 1;
        }
    
        i++;
      }
    
      // copy the last word
      if (word_start < text.size())
      {
        words.emplace_back(text.substr(word_start));
      }
    
      return words;
    }
    
    std::string EvaluateWordlist(std::list<std::string> word_list, bool eval_target_functions, std::chrono::system_clock::time_point time)
    {
      // As long as we have evaluated functions, something might have changed and we have to check again and again if we can evaluate more.
      bool evaluated_function = true;
      while (evaluated_function)
      {
        evaluated_function = false;
    
        // Iterate over all words and check if we find elements 
        for (auto word_it = word_list.begin(); word_it != word_list.end(); ++word_it)
        {
          // Check if we have a function starting at i
          if (IsTargetIdentifierWord(*word_it))
          {
            // Found a target Identifier that may indicate a new function!
            std::list<std::string>::iterator target_identifier_it = word_it;
            std::list<std::string>::iterator target_it            = word_list.end(); // optional
            std::list<std::string>::iterator open_bracket_it      = word_list.end();
            std::list<std::string>::iterator function_it          = word_list.end(); // optional
            std::list<std::string>::iterator closing_bracket_it   = word_list.end();
    
            // Look for (optional) target and open bracket
            if ((std::next(target_identifier_it, 1) != word_list.end())
              && IsFunctionStartWord(*std::next(target_identifier_it, 1)))
            {
              // No target
              open_bracket_it = std::next(target_identifier_it, 1);
            }
            else if ((std::next(target_identifier_it, 1) != word_list.end())
              && (std::next(target_identifier_it, 2) != word_list.end())
              && !IsSpecialWord(*std::next(target_identifier_it, 1))
              && IsFunctionStartWord(*std::next(target_identifier_it, 2)))
            {
              // Target found
              target_it       = std::next(target_identifier_it, 1);
              open_bracket_it = std::next(target_identifier_it, 2);
            }
            else
            {
              // invalid expression
              continue;
            }
    
            // Look for (optional) function and closing bracket
            if ((std::next(open_bracket_it, 1) != word_list.end())
              && IsFunctionEndWord(*std::next(open_bracket_it, 1)))
            {
              // No function
              closing_bracket_it = std::next(open_bracket_it, 1);
            }
            else if ((std::next(open_bracket_it, 1) != word_list.end())
              && (std::next(open_bracket_it, 2) != word_list.end())
              && !IsSpecialWord(*std::next(open_bracket_it, 1))
              && IsFunctionEndWord(*std::next(open_bracket_it, 2)))
            {
              // function found
              function_it        = std::next(open_bracket_it, 1);
              closing_bracket_it = std::next(open_bracket_it, 2);
            }
            else
            {
              // invalid expression
              continue;
            }
    
            // Extract information about target and functions
            std::string target_string;
            std::string function_name;
            std::string function_param;
    
            if (target_it != word_list.end())
            {
              target_string = Unescape(*target_it);
              target_string = EcalUtils::String::Trim(target_string);
            }
    
            if (function_it != word_list.end())
            {
              // Split function into name + params
              std::string function_string = Unescape(*function_it);
              function_string = EcalUtils::String::Trim(function_string);
              
              size_t delim = function_string.find_first_of(' ');
              if (delim == std::string::npos)
              {
                function_name = function_string;
              }
              else
              {
                function_name  = function_string.substr(0, delim);
                function_param = EcalUtils::String::Trim(function_string.substr(delim + 1));
              }
            }
    
            std::string result;
            
            if (eval_target_functions || !IsTargetKeyword(target_string))
            {
              // Evaluate the function, if required
              result = EvaluateFunction(function_name, function_param, time);
            }
            else
            {
              // If we don't want to evaluate the function, we re-asseble the string that generated it
              std::list<std::string> function_elements(target_identifier_it, std::next(closing_bracket_it, 1));
              result = EcalUtils::String::Join("", function_elements);
            }
    
            // If there was a "normal" word before or after the function we just
            // evaluated, we have to combine those to one normal word. Otherwise,
            // the evaluation would not be able to recognize valid functions, as it
            // will always assume a special word after each normal word
    
            std::list<std::string>::iterator first_index_to_remove = target_identifier_it;
            std::list<std::string>::iterator last_index_to_remove  = closing_bracket_it;
    
            if ((target_identifier_it != word_list.begin())
              && !IsSpecialWord(*std::prev(target_identifier_it, 1)))
            {
              first_index_to_remove = std::prev(target_identifier_it, 1);
              result = (*std::prev(target_identifier_it, 1) + result);
            }
    
            if (std::next(closing_bracket_it, 1) != word_list.end()
              && (!IsSpecialWord(*std::next(closing_bracket_it, 1))))
            {
              last_index_to_remove = std::next(closing_bracket_it, 1);
              result = (result + *std::next(closing_bracket_it, 1));
            }
    
            // Re-use the first element that we would otherwise remove for storing the result
            *first_index_to_remove = result;
    
            // Remove the remaining elements that belong the the evaluated function
            word_list.erase(std::next(first_index_to_remove, 1), std::next(last_index_to_remove, 1));
    
            // Inform the outer loop that we have evaluated something, so it will make at least one more pass and check if there is more stuff to evaluate
            evaluated_function = true;
    
            // Exit from the main for-loop, as our iterator has become invalid
            break;
          }
        }
      }
    
      // Un-escape all remaining words
      //for (auto word_it = word_list.begin(); word_it != word_list.end(); word_it++)
      //{
      //  *word_it = Unescape(*word_it);
      //}
    
      // Concatenate all remaining words that we were unable to evaluate. A correct
      // expresion will never leave us with more than one word here, but we cannot
      // count on the user to write valid expressions.
      return EcalUtils::String::Join("", word_list);
    }
    
    bool IsSpecialWord(const std::string& word)
    {
      return (word.size() == 1)
        && ((word.front() == TARGET_IDENTIFIER_CHAR) || (word.front() == FUNCTION_START_CHAR) || (word.front() == FUNCTION_END_CHAR));
    }
    
    bool IsTargetIdentifierWord(const std::string& word)
    {
    
      return (word.size() == 1) && (word.front() == TARGET_IDENTIFIER_CHAR);
    }
    
    bool IsFunctionStartWord(const std::string& word)
    {
        return (word.size() == 1) && (word.front() == FUNCTION_START_CHAR);
    }
    
    bool IsFunctionEndWord(const std::string& word)
    {
        return (word.size() == 1) && (word.front() == FUNCTION_END_CHAR);
    }
    
    bool IsTargetKeyword(const std::string& word)
    {
      std::string lower_word = word;
      // cause warning C4244 with VS2017, VS2019
      //std::transform(lower_word.begin(), lower_word.end(), lower_word.begin(), ::tolower);
      std::transform(lower_word.begin(), lower_word.end(), lower_word.begin(),
        [](char c) {return static_cast<char>(::tolower(c)); });
      return (lower_word == "target")
        || (lower_word == "_");
    }

    std::string EvaluateFunction(const std::string& function_name, const std::string& param, std::chrono::system_clock::time_point time)
    {
      auto function_map = GetFunctions();
      auto function_it = function_map.find(function_name);

      if (function_it != function_map.end())
      {
        return function_it->second->Evaluate(param, time);
      }
      else
      {
        return "";
      }
    }
  }


  std::string Evaluate(const std::string& input, bool eval_target_functions, std::chrono::system_clock::time_point time)
  {
    std::stringstream input_stream(input);
    std::stringstream output;

    bool first_line = true;

    std::string line;
    while (std::getline(input_stream, line)) {
      // Add linebreaks again, except before the first line
      if (first_line)
        first_line = false;
      else
        output << "\n";

      // Lexical analysis of the input
      auto word_list = SplitIntoWords(line);

      // Evaluate the word list
      output << EvaluateWordlist(word_list, eval_target_functions, time);
    }

    return output.str();
  }

  // TODO: I am returning a reference here, I must make sure that everything is const-correct!
  const std::map<std::string, EcalParser::Function*>& GetFunctions()
  {
    static std::map<std::string, EcalParser::Function*> functions =
    {
      { "ENV",      new EcalParser::FunctionEnv() },
      { "HOSTNAME", new EcalParser::FunctionHostname()},
      { "OS",       new EcalParser::FunctionOs() },
      { "OSSELECT", new EcalParser::FunctionOsselect() },
      { "TIME",     new EcalParser::FunctionTime()},
    };

    return functions;
  }
  
  std::string Escape(const std::string& input)
  {
    std::string output;
    output.reserve(input.size() * 2);
    for (char current_char : input)
    {
      if ((current_char == ESCAPE_CHAR)
        || (current_char == TARGET_IDENTIFIER_CHAR)
        || (current_char == FUNCTION_START_CHAR)
        || (current_char == FUNCTION_END_CHAR))
      {
        output += ESCAPE_CHAR;
      }
      output += current_char;
    }
    return output;
  }
  
  std::string Unescape(const std::string& input)
  {
    std::string output;
    output.reserve(input.length());
    size_t pos = 0;
    while (pos < input.size())
    {
      if (input.at(pos) == ESCAPE_CHAR)
      {
        // Skip the escape char itself
        pos++;
      }
  
      if (pos < input.size())
      {
        // Copy the current char (as we may have skipped an escape char, we have to check again whether pos points to a valid position)
        output += input.at(pos);
      }
      pos++;
    }
    return output;
  }
}