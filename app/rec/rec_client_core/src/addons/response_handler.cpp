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

#include "response_handler.h"

namespace eCAL
{
  namespace rec
  {
    namespace addon
    {

      ResponseHandler::ResponseHandler()
      {
      }

      bool ResponseHandler::DecodeResponse(const std::vector<std::string>& response_lines, Response& response, const std::string& function_name)
      {
        if (response_lines.empty())
        {
          SetErrorString("Response is empty.");
          return false;
        }

        auto function_iter = function_descriptors_.find(function_name);

        if (function_iter == function_descriptors_.end())
        {
          SetErrorString("Unrecognized function \"" + function_name + "\"");
          return false;
        }

        for (std::vector<std::string>::size_type i = 0; i < (response_lines.size() - 1); ++i)
        {
          const auto& result_line = response_lines.at(i);

          if (!result_line.empty())
          {
            std::vector<std::string> tokens;
            if (!TokenizeLine(result_line, tokens))
            {
              SetErrorString("Invalid result format of response.");
              return false;
            }

            std::unordered_map<std::string, Variant> result;
            if (!ParseArgumentTokens(tokens, function_iter->second.mandatory_results, result) ||
              !CheckArguments(result, function_iter->second.mandatory_results))
            {
              SetErrorString("Invalid result of function \"" + function_iter->second.name + "\".");
              return false;
            }

            response.results.push_back(result);
          }
        }
        {
          const auto& status_line = response_lines.back();

          std::vector<std::string> tokens;
          if (!TokenizeLine(status_line, tokens) ||
            !ParseStatusTokens(tokens, response.status, response.status_message))
          {
            SetErrorString("Invalid status format of response.");
            return false;
          }
        }

        return true;
      }

      void ResponseHandler::EncodeRequest(std::string& request_line, const Request& request) const
      {
        request_line = request.function_name + " ";

        SerializeArguments(request.parameters, request_line);

      }

      bool ResponseHandler::Query(const Request& request, Response& response)
      {
        std::string request_line;
        EncodeRequest(request_line, request);

        if (!WriteLine(request_line))
        {
          SetErrorString("Error while writing request line.");
          return false;
        }

        std::vector<std::string> response_lines;
        if (!ReadLines(response_lines))
        {
          SetErrorString("Error while reading response line.");
          return false;
        }

        if (!DecodeResponse(response_lines, response, request.function_name))
        {
          SetErrorString("Error while decoding response lines. " + GetErrorString());
          return false;
        }

        return true;
      }

      void ResponseHandler::SetWriteLineCallback(const std::function<bool(const std::string&)>& callback)
      {
        WriteLine = callback;
      }

      void ResponseHandler::SetReadLineCallback(const std::function<bool(std::string&)>& callback)
      {
        ReadLine = callback;
      }

      std::string ResponseHandler::GetErrorString() const
      {
        return error_string_;
      }

      void ResponseHandler::SetErrorString(const std::string& error_string)
      {
        error_string_ = error_string;
      }

      void ResponseHandler::SetFunctionDescriptors(const std::unordered_set<FunctionDescriptor>& function_descriptors)
      {
        for (const auto& function_descriptor : function_descriptors)
        {
          function_descriptors_.insert(std::make_pair(function_descriptor.name, function_descriptor));
        }
      }

      void ResponseHandler::ResetFunctionDescriptors()
      {
        function_descriptors_.clear();
      }

      bool ResponseHandler::ParseArgumentTokens(const std::vector<std::string>& tokens, const std::unordered_map<std::string, Variant::ValueType>& mandatory_args, std::unordered_map<std::string, Variant>& arguments)
      {
        if (tokens.size() % 2 != 0)
          return false;

        for (std::vector<std::string>::size_type i = 0; i < tokens.size() / 2; ++i)
        {
          const auto& name = tokens.at(2 * i);
          const auto& value = tokens.at(2 * i + 1);

          auto mandatory_iter = mandatory_args.find(name);
          if (mandatory_args.find(name) != mandatory_args.end())
          {
            if (!arguments.insert(std::make_pair(name, Variant::ParseFromString(value, mandatory_iter->second))).second)
              return false;
          }
          else
          {
            return false;
          }
        }

        if (mandatory_args.size() != arguments.size())
          return false;

        return true;
      }

      bool ResponseHandler::ParseStatusTokens(const std::vector<std::string>& tokens, Response::Status& status, std::string& status_message)
      {
        if (tokens.size() != 2)
          return false;

        static std::unordered_map<std::string, Response::Status> status_map{ { "Ok", Response::Status::Ok },{ "Failed", Response::Status::Failed },{ "Syntax_Error", Response::Status::Syntax_Error } };
        auto status_iter = status_map.find(tokens.front());
        if (status_iter != status_map.end())
          status = status_iter->second;
        else
          return false;

        status_message = tokens.back();

        return true;
      }

      bool ResponseHandler::CheckArguments(const std::unordered_map<std::string, Variant>& arguments, const std::unordered_map<std::string, Variant::ValueType>& mandatory_args)
      {
        for (const auto& mandatory_arg_pair : mandatory_args)
        {
          auto arguments_iter = arguments.find(mandatory_arg_pair.first);
          if (arguments_iter == arguments.end() || arguments_iter->second.GetValueType() != mandatory_arg_pair.second)
            return false;
        }

        return true;
      }

      void ResponseHandler::SerializeArguments(const std::unordered_map<std::string, Variant>& arguments, std::string& line)
      {
        for (const auto& argument_pair : arguments)
        {
          const auto& argument_name = argument_pair.first;
          const auto& argument_value = argument_pair.second;
          std::string encoded_argument_value;

          switch (argument_value.GetValueType())
          {
          case Variant::ValueType::Boolean:
            encoded_argument_value = argument_value.GetBooleanValue() ? "true" : "false";
            break;
          case Variant::ValueType::Float:
            encoded_argument_value = std::to_string(argument_value.GetFloatValue());
            break;
          case Variant::ValueType::Integer:
            encoded_argument_value = std::to_string(argument_value.GetIntegerValue());
            break;
          case Variant::ValueType::String:
            encoded_argument_value = GetEncodedString(argument_value.GetStringValue());
            break;
          default:
            continue;
          }

          line += argument_name + " " + encoded_argument_value + " ";
        }
      }

      bool ResponseHandler::TokenizeLine(const std::string& line, std::vector<std::string>& tokens)
      {
        auto state = 0;
        auto substate = 0;

        std::string token;

        for (const auto ch : line)
        {
          if (state == 0)
          {
            if (isspace(ch)) {}
            else if (ch == '\"')
            {
              state = 1; substate = 0;
            }
            else
            {
              state = 2;
              token.push_back(ch);
            }
          }
          else if (state == 1)
          {
            if (substate == 0)
            {
              if (ch == '\\')
                substate = 1;
              else if (ch == '\"')
              {
                tokens.push_back(token);
                token.clear();
                state = 0;
              }
              else
                token.push_back(ch);
            }
            else if (substate == 1)
            {
              if (ch == '\\' || ch == '\"')
              {
                token.push_back(ch);
                substate = 0;
              }
              else
              {
                return false;
              }
            }
          }
          else if (state == 2)
          {
            if (isspace(ch))
            {
              tokens.push_back(token);
              token.clear();
              state = 0;
            }
            else
              token.push_back(ch);
          }
        }

        if (state == 2) tokens.push_back(token);
        else if (state == 1)
        {
          return false;
        }

        return true;
      }

      std::string ResponseHandler::GetEncodedString(const std::string& str)
      {
        std::string result;
        for (const auto& ch : str)
        {
          if (ch == '\"') result += "\\\"";
          else if (ch == '\\') result += "\\\\";
          else result.push_back(ch);
        }

        return "\"" + result + "\"";
      }

      std::string ResponseHandler::GetParameterString(const FunctionDescriptor& function)
      {
        std::string return_value;

        static const std::unordered_map<int, std::string> type_map = {
          { static_cast<int>(Variant::ValueType::Boolean), "boolean" },
          { static_cast<int>(Variant::ValueType::Float), "float" },
          { static_cast<int>(Variant::ValueType::Integer), "integer" },
          { static_cast<int>(Variant::ValueType::String), "string" },
          { static_cast<int>(Variant::ValueType::Undefined), "undefined" }
        };

        //return_value = function.name;
        for (const auto& mandatory_parameter : function.mandatory_parameters)
        {
          return_value += " " + mandatory_parameter.first + " <" + type_map.at(static_cast<int>(mandatory_parameter.second)) + ">";
        }

        return return_value;
      }

      bool ResponseHandler::ReadLines(std::vector<std::string>& response_lines)
      {
        while (true)
        {
          std::string response_line;
          if (!ReadLine(response_line))
            return false;

          if (response_line.empty())
            continue;

          response_lines.push_back(response_line);
          if (response_line.at(0) != ' ')
            return true;
        }
      }
    }
  }
}