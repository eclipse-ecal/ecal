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

#include "request_handler.h"

using namespace eCAL::rec::addon;

bool RequestHandler::Query(const std::string& request_line, std::vector<std::string>& response_lines)
{
  Request request;
  if (!DecodeRequest(request_line, request))
  {
    SetErrorString("Request decoding failed. " + GetErrorString());
    return false;
  }

  Response response;
  response = function_callbacks_.at(request.function_name).second(request);

  EncodeResponse(response_lines, response);

  return true;
}

void RequestHandler::AddFunctionCallback(const FunctionDescriptor& function_descriptor, const Callback& callback)
{
  function_callbacks_.insert(std::make_pair(function_descriptor.name, std::make_pair(function_descriptor, callback)));
}

void RequestHandler::RemoveFunctionCallbacks()
{
  function_callbacks_.clear();
}

std::string RequestHandler::GetErrorString() const 
{
  return error_string_;
}

bool RequestHandler::ParseArgumentTokens(const std::vector<std::string>& tokens, const std::unordered_map<std::string, Variant::ValueType>& mandatory_args, std::unordered_map<std::string, Variant>& arguments)
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
      try
      {
        if (!arguments.insert(std::make_pair(name, Variant::ParseFromString(value, mandatory_iter->second))).second)
          return false;
      }
      catch (const std::invalid_argument&)
      {
        return false;
      }
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

bool RequestHandler::CheckArguments(const std::unordered_map<std::string, Variant>& arguments, const std::unordered_map<std::string, Variant::ValueType>& mandatory_args)
{
  for (const auto& mandatory_arg_pair : mandatory_args)
  {
    auto arguments_iter = arguments.find(mandatory_arg_pair.first);
    if (arguments_iter == arguments.end() || arguments_iter->second.GetValueType() != mandatory_arg_pair.second)
      return false;
  }

  return true;
}

void RequestHandler::SerializeArguments(const std::unordered_map<std::string, Variant>& arguments, std::string& line)
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

bool RequestHandler::TokenizeLine(const std::string& line, std::vector<std::string>& tokens)
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

void RequestHandler::SerializeStatus(Response::Status status, const std::string& status_message, std::string& line)
{
  static std::unordered_map<int, std::string> status_map{ { static_cast<int>(Response::Status::Ok),  "Ok" },{ static_cast<int>(Response::Status::Failed), "Failed" },{ static_cast<int>(Response::Status::Syntax_Error), "Syntax_Error" } };

  line = status_map.at(static_cast<int>(status)) + " " + GetEncodedString(status_message);
}

std::string RequestHandler::GetEncodedString(const std::string& str)
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

std::string RequestHandler::GetParameterString(const FunctionDescriptor& function)
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

void RequestHandler::SetErrorString(const std::string& error_string)
{
  error_string_ = error_string;
}

bool RequestHandler::DecodeRequest(const std::string& request_line, Request& request)
{
  std::vector<std::string> tokens;
  if (!TokenizeLine(request_line, tokens))
  {
    SetErrorString("Invalid request line format.");
    return false;
  }

  if (tokens.empty())
  {
    SetErrorString("Request is empty.");
    return false;
  }

  request.function_name = tokens.front();
  auto function_iter = function_callbacks_.find(tokens.front());
  if (function_iter == function_callbacks_.end())
  {
    SetErrorString("Unrecognized function " + tokens.front() + ".");
    return false;
  }

  tokens.erase(tokens.begin());

  //std::unordered_map<std::string, Variant> parameters;

  if (!ParseArgumentTokens(tokens, function_iter->second.first.mandatory_parameters, request.parameters)
    || !CheckArguments(request.parameters, function_iter->second.first.mandatory_parameters))
  {
    SetErrorString("Invalid parameter of function " + function_iter->second.first.name + ". Usage: " + function_iter->second.first.name + " " + GetParameterString(function_iter->second.first));
    return false;
  }

  return true;
}

void RequestHandler::EncodeResponse(std::vector<std::string>& response_lines, const Response& response)
{

  for (const auto& result : response.results)
  {
    std::string result_line;
    SerializeArguments(result, result_line);

    response_lines.emplace_back(" " + result_line);
  }

  std::string status_line;
  SerializeStatus(response.status, response.status_message, status_line);
  response_lines.push_back(status_line);
}