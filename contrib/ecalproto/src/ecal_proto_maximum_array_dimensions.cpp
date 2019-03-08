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

#include <ecal/protobuf/ecal_proto_maximum_array_dimensions.h>

#include <ecal/protobuf/ecal_proto_hlp.h>

namespace eCAL
{
  namespace protobuf
  {
    // returns the max size of the element, or 0 if the element had not been registered.
    size_t MaximumArrayDimensionsVisitor::MaxSize(const std::string & element)
    {
      auto it_size = max_sizes.find(element);
      if (it_size != max_sizes.end())
      {
        return it_size->second;
      }
      else
      {
        return 0;
      }
    }

    void MaximumArrayDimensionsVisitor::ArrayStart(const MessageInfo& info_, const google::protobuf::FieldDescriptor::Type & /*type_*/, size_t size_)
    {
      std::string complete_name = CreateCompleteMessageName(info_.field_name, info_.group_name);
      auto& size = max_sizes[complete_name];
      if (size < size_)
      {
        size = size_;
      }
    }
  }
}