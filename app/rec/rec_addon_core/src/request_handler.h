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

#include "common_types.h"

#include <functional>

namespace eCAL
{
  namespace rec
  {
    namespace addon
    {
      class RequestHandler
      {
      public:
        using Callback = std::function<Response(const Request&)>;

        bool Query(const std::string& request_line, std::vector<std::string>& response_lines);

        void AddFunctionCallback(const FunctionDescriptor& function, const Callback& callback);
        void RemoveFunctionCallbacks();

        std::string GetErrorString() const;

        static void EncodeResponse(std::vector<std::string>& response_lines, const Response& response);

      private:
        bool DecodeRequest(const std::string& request_line, Request& request);

        static bool ParseArgumentTokens(const std::vector<std::string>& tokens, const std::unordered_map<std::string, Variant::ValueType>& mandatory_args, std::unordered_map<std::string, Variant>& arguments);
        static bool CheckArguments(const std::unordered_map<std::string, Variant>& arguments, const std::unordered_map<std::string, Variant::ValueType>& mandatory_args);
        static void SerializeArguments(const std::unordered_map<std::string, Variant>& arguments, std::string& line);
        static bool TokenizeLine(const std::string& line, std::vector<std::string>& tokens);
        static void SerializeStatus(Response::Status status, const std::string& status_message, std::string& line);

        static std::string GetEncodedString(const std::string& str);
        static std::string GetParameterString(const FunctionDescriptor& function);

        void SetErrorString(const std::string& error_string);
      private:
        std::string error_string_;

        std::unordered_map<std::string, std::pair<FunctionDescriptor, Callback>> function_callbacks_;
      };
    }
  }
}
