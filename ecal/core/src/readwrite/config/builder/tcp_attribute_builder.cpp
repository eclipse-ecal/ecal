/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include "tcp_attribute_builder.h"

namespace eCAL
{
  namespace eCALReader
  {
    TCPLayer::SAttributes BuildTCPLayerAttributes(const eCALReader::SAttributes& attr_)
    {
      TCPLayer::SAttributes attributes;

      attributes.max_reconnection_attempts = attr_.tcp.max_reconnection_attempts;
      attributes.thread_pool_size          = attr_.tcp.thread_pool_size;
      
      return attributes;
    }
  }

  namespace eCALWriter
  {
    TCP::SAttributes BuildTCPAttributes(const uint64_t& topic_id_, const eCALWriter::SAttributes& attr_)
    {
      TCP::SAttributes attributes;

      attributes.topic_name = attr_.topic_name;
      attributes.topic_id   = topic_id_;
      attributes.thread_pool_size = attr_.tcp.thread_pool_size;
      
      return attributes;
    }
  }
}