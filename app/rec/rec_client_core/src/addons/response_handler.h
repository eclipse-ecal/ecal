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

#include <vector>
#include <string>
#include <functional>
#include <unordered_set>
#include <unordered_map>

#include "common_types.h"

namespace eCAL
{
  namespace rec
  {
    namespace addon
    {
      class ResponseHandler
      {
      public:
        ResponseHandler();

        bool DecodeResponse(const std::vector<std::string>& response_lines, Response& response, const std::string& function_name);
        void EncodeRequest(std::string& request_line, const Request& request) const;

        bool Query(const Request& request, Response& response);

        void SetWriteLineCallback(const std::function<bool(const std::string&)>& callback);
        void SetReadLineCallback(const std::function<bool(std::string&)>& callback);

        std::string GetErrorString() const;

        void SetFunctionDescriptors(const std::unordered_set<FunctionDescriptor>& function_descriptor);
        void ResetFunctionDescriptors();

      private:
        void SetErrorString(const std::string& error_string);

        bool ReadLines(std::vector<std::string>& response_lines);

        static bool ParseArgumentTokens(const std::vector<std::string>& tokens, const std::unordered_map<std::string, Variant::ValueType>& mandatory_args, std::unordered_map<std::string, Variant>& arguments);
        static bool ParseStatusTokens(const std::vector<std::string>& tokens, Response::Status& status, std::string& status_message);
        static bool CheckArguments(const std::unordered_map<std::string, Variant>& arguments, const std::unordered_map<std::string, Variant::ValueType>& mandatory_args);
        static void SerializeArguments(const std::unordered_map<std::string, Variant>& arguments, std::string& line);
        static bool TokenizeLine(const std::string& line, std::vector<std::string>& tokens);

        static std::string GetEncodedString(const std::string& str);
        static std::string GetParameterString(const FunctionDescriptor& function);


      private:
        std::function<bool(const std::string&)> WriteLine = [](const auto&) {return false; };
        std::function<bool(std::string&)> ReadLine = [](auto&) {return false; };

        std::string error_string_;

        std::unordered_map<std::string, FunctionDescriptor> function_descriptors_;
      };
    }
  }
}