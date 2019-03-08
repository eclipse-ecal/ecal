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
#include <set>
#include <mutex>
#include <regex>

namespace eCAL
{
  namespace protobuf
  {

    /*
    * Hierarchy:
    *
    * MessageFilter (abstract)
    *  - NoFilter
    *  - BaseIncludeFilter (abstract)
    *     - SimpleIncludeFilter
    *     - ComplexIncludeFilter
    */


    class MessageFilter
    {
    public:

      virtual ~MessageFilter() {}
      virtual bool Filter(const std::string& message_name_) = 0;

    };

    class NoFilter : public MessageFilter
    {
    public:
      bool Filter(const std::string&) override
      {
        return true;
      }
    };

    /* This filter includes the given strings */
    class BaseIncludeFilter : public MessageFilter
    {
    public:
      BaseIncludeFilter() {};
      bool Filter(const std::string& message_name_) override = 0;
      void Clear();
      void Insert(const std::string& include_);
      void Erase(const std::string& include_);

    protected:
      std::set<std::string> elements;
      std::mutex elements_mutex;
    };

    /* This filter includes the given strings */
    class SimpleIncludeFilter : public BaseIncludeFilter
    {
    public:
      SimpleIncludeFilter();
      bool Filter(const std::string& message_name_) override;
    };

    /* This filter includes the given strings */
    class ComplexIncludeFilter : public BaseIncludeFilter
    {
    public:
      ComplexIncludeFilter();
      bool Filter(const std::string& message_name_) override;

    private:
      std::regex filter_regex;
    };

  }
}