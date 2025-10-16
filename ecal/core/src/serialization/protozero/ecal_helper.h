/* ========================= eCAL LICENSE =================================
 *
 * Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
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

inline void AssignString(::protozero::pbf_reader& reader, std::string& assignee)
{
  protozero::data_view string_view = reader.get_view();
  assignee.assign(string_view.data(), string_view.size());
}

inline void AssignBytes(::protozero::pbf_reader& reader, std::string& assignee)
{
  AssignString(reader, assignee);
}

template <class Assignee, class Conversion>
void AssignMessage(protozero::pbf_reader& parent_reader, Assignee& assignee, Conversion&& convert) {
    protozero::pbf_reader message_reader = parent_reader.get_message();
    convert(message_reader, assignee);
}

template <class Assignee, class Conversion>
void AddRepeatedMessage(protozero::pbf_reader& parent_reader, Assignee& assignee, Conversion&& convert) {
    protozero::pbf_reader message_reader = parent_reader.get_message();
    auto& new_assignee_element = assignee.push_back();
    convert(message_reader, new_assignee_element);
}