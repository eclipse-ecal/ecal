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

#include <ecal/protobuf/ecal_proto_visitor.h>

#include <map>
#include <string>

namespace eCAL
{
  namespace protobuf
  {
    class MaximumArrayDimensionsVisitor : public MessageVisitorDoubleIntegral 
    {
    public:
      size_t MaxSize(const std::string& element);

    protected:
      void ArrayStart(const MessageInfo& info_, const google::protobuf::FieldDescriptor::Type& type_, size_t size_) override;

    private:
      std::map<std::string, size_t> max_sizes;
    };
  }
}