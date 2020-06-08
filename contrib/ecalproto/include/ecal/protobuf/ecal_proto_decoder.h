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

/**
 * @brief  eCALMonitor proto message decoding class
**/

#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4244 4267 4512 4996 4146 4800) // disable proto warnings
#endif
#include <google/protobuf/message.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <memory>
#include <string>
#include <functional>
#include <stdint.h>

namespace eCAL
{
namespace protobuf
{
  class MessageFilter;
  class MessageVisitor;
  class CProtoDecoder
  {
  public:
    CProtoDecoder();

    bool ProcProtoMsg(const google::protobuf::Message& msg_, const std::string& name_="", const std::string& prefix_="", bool is_array_=false, size_t index_=0);

    void SetVisitor(std::shared_ptr<MessageVisitor> visitor_) { visitor = std::move(visitor_); };

  private:

    std::shared_ptr<MessageVisitor> visitor;
  };

}
}
