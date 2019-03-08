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

#include <ecal/protobuf/ecal_proto_message_filter.h>

#include <algorithm>
#include <iterator>
#include <sstream>
#include <vector>

namespace eCAL
{
  namespace protobuf
  {
    bool StartsWith(const std::string& haystack, const std::string& needle) {
      return needle.length() <= haystack.length()
        && equal(needle.begin(), needle.end(), haystack.begin());
    }

    /*
    * Creates a set that includes all sub - elements:
    * create_set("a.b.c[1].d")
    * returns a set with the following elements:
    * - a
    * - a.b
    * - a.b.c
    * - a.b.c[1]
    * - a.b.c[1].d
    */
    std::set<std::string> CreateSet(const std::string& input_)
    {
      std::set<std::string> elements;
      std::string item;    // individual item
      std::string include; // item to be put in set
      std::stringstream input(input_);
      while (std::getline(input, item, '.'))
      {
        if (!include.empty())
        {
          include += '.';
        }

        // If adding array info, we need to add both the 
        // first part and the whole part to the elements vector
        // include = a.b.  , item = c[1]
        // add a.b.c
        // and add a.b.c[1]
        auto found = item.find('[');
        if (found != std::string::npos)
        {
          auto extra = item.substr(0, found);
          elements.insert(extra);
        }

        include += item;
        elements.insert(include);
      }
      return elements;
    }

    void BaseIncludeFilter::Clear()
    {
      std::lock_guard<std::mutex> lock(elements_mutex);
      elements.clear();
    }

    void BaseIncludeFilter::Insert(const std::string& include_)
    {
      std::lock_guard<std::mutex> lock(elements_mutex);
      // Compute elements to add
      auto new_elements = CreateSet(include_);
      // make a union between the existing set and the new_elements
      std::set_union(elements.begin(), elements.end(),
        new_elements.begin(), new_elements.end(),
        std::inserter(elements, elements.end()));
    }


    void BaseIncludeFilter::Erase(const std::string& include_)
    {
      std::lock_guard<std::mutex> lock(elements_mutex);
      // This function is a little more complicated
      auto all_identifiers = CreateSet(include_);
      // Now move backwards and remove all elements if the following element is not in the namespace
      // example
      // a, a.b, a.b.c, a.d, a.d.c, a.d.e, a.e
      // removing a.b.c would remove a.b. and a.b.c
      // removing a.d.c would remove only a.d.c
      for (auto id = all_identifiers.rbegin(); id != all_identifiers.rend(); ++id)
      {
          // Find the element and the element afterwards
          auto elem = elements.find(*id);
          auto following_item = elem;
          ++following_item;

          // If the following element starts with the same string, we cannot remove our element
          // nor any of the shorter prefixes, thus we break the loop.
          if ((following_item != elements.end()) && StartsWith(*following_item, *elem))
            break;
          elements.erase(elem);
      }
    }

    SimpleIncludeFilter::SimpleIncludeFilter()
    {
    }

    bool SimpleIncludeFilter::Filter(const std::string & message_name_)
    {
      std::lock_guard<std::mutex> lock(elements_mutex);
      return elements.find(message_name_) != elements.end();
    }

    ComplexIncludeFilter::ComplexIncludeFilter()
      : filter_regex("([^\\[]+)(?:\\[(\\d+|\\*)\\])?")
    {
    }

    bool ComplexIncludeFilter::Filter(const std::string& message_name_)
    {
      // Ok, what's the problem here?
      // In case we get a request a[1].b.c[2].d[3]
      // Our Map might contain a[1].b.c[*].d[3]
      // Or even               a[*].b.c[*].d[*], 
      // so any combination of indices and stars lead to a valid request.
      // So we need an algorithm to figure out if the request is actually valid.
      // Proposed algo:
      std::lock_guard<std::mutex> lock(elements_mutex);
      std::string elem_to_find;
      std::smatch match;

      auto message_name = message_name_;

      while (std::regex_search(message_name, match, filter_regex))
      {
        if (match[2].matched) // did the regex match brackets
        {
          bool is_star(match[1].str() == "*");
          std::string number_try = elem_to_find + match[1].str() + "[" + match[2].str() + "]";
          std::string star_try = elem_to_find + match[1].str() + "[*]";

          // The elements set contains a[1]
          if (!is_star && elements.find(number_try) != elements.end())
          {
            elem_to_find += number_try;
          }
          // The elements set contains a[*]
          else if (elements.find(star_try) != elements.end())
          {
            elem_to_find += star_try;
          }
          else
          {
            return false;
          }

        }
        else // The last match might only have one match
        {
          elem_to_find += match[0].str();
          return (elements.find(elem_to_find) != elements.end());
        }
        message_name = match.suffix().str();
      }

      return true;
    }
}
}