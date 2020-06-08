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
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace eCAL
{
  namespace rec
  {
    namespace addon
    {
      class Variant
      {
      public:
        enum class ValueType
        {
          Undefined,
          Boolean,
          String,
          Float,
          Integer
        };

        Variant() : value_type_(ValueType::Undefined)
        {
        }

        Variant(bool boolean_value) : boolean_value_(boolean_value), value_type_(ValueType::Boolean)
        {
        }

        Variant(const std::string& string_value) : string_value_(string_value), value_type_(ValueType::String)
        {
        }

        Variant(float float_value) : float_value_(float_value), value_type_(ValueType::Float)
        {
        }

        Variant(std::int64_t integer_value) : integer_value_(integer_value), value_type_(ValueType::Integer)
        {
        }

        static Variant ParseFromString(const std::string& str, ValueType value_type)
        {
          switch (value_type)
          {
          case ValueType::Boolean:
          {
            std::string lowercase_str = str;
            if (lowercase_str == "true" || lowercase_str == "1" || lowercase_str == "on") return Variant(true);
            else if (lowercase_str == "false" || lowercase_str == "0" || lowercase_str == "off") return Variant(false);
            else throw std::invalid_argument("\"" + str + "\" is not a boolean value.");
          }
          case ValueType::String:
            return Variant(str);
          case ValueType::Float:
            return Variant(std::stof(str));
          case ValueType::Integer:
            return Variant(static_cast<std::int64_t>(std::stoll(str)));
          default:
            return Variant();
          }
        }

        ValueType GetValueType() const
        {
          return value_type_;
        }

        bool GetBooleanValue() const
        {
          if (value_type_ != ValueType::Boolean) throw std::invalid_argument("Variant doesn't contain a boolean value.");
          return boolean_value_;
        }

        std::string GetStringValue() const
        {
          if (value_type_ != ValueType::String) throw std::invalid_argument("Variant doesn't contain a string value.");
          return string_value_;
        }

        float GetFloatValue() const
        {
          if (value_type_ != ValueType::Float) throw std::invalid_argument("Variant doesn't contain a float value.");
          return float_value_;
        }

        std::int64_t GetIntegerValue() const
        {
          if (value_type_ != ValueType::Integer) throw std::invalid_argument("Variant doesn't contain a integer value.");
          return integer_value_;
        }

      private:
        bool boolean_value_ {false};
        std::string string_value_ {};
        float float_value_ {0.0f};
        std::int64_t integer_value_ {0};
        ValueType value_type_ {ValueType::Undefined};
      };

      struct Request
      {
        std::string function_name;
        std::unordered_map <std::string, Variant> parameters;
      };

      struct Response
      {
        enum class Status
        {
          Ok,
          Failed,
          Syntax_Error
        };

        using Result = std::unordered_map<std::string, Variant>;
        Status status;
        std::string status_message;
        std::vector<Result> results;
      };

      struct FunctionDescriptor
      {
        std::string name;
        std::unordered_map <std::string, Variant::ValueType> mandatory_parameters;
        std::unordered_map <std::string, Variant::ValueType> mandatory_results;

        bool operator==(const FunctionDescriptor& function_descriptor) const
        {
          return (name == function_descriptor.name);
        }
      };
    }
  }
}

namespace std
{
  template <>
  struct hash<eCAL::rec::addon::FunctionDescriptor>
  {
    std::size_t operator()(const eCAL::rec::addon::FunctionDescriptor& k) const
    {
      return std::hash<std::string>()(k.name);
    }
  };
}

