/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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

#include "data_reader_tcp_attribute_builder.h"

namespace eCAL
{
  namespace eCALReader
  {
    namespace TCP
    {
      TCP::SAttributes BuildTCPReaderAttributes(const TCPLayer::SAttributes& attr_)
      {
        TCP::SAttributes attributes;

        attributes.thread_pool_size          = attr_.thread_pool_size;
        attributes.max_reconnection_attempts = attr_.max_reconnection_attempts;
        
        return attributes;
      }
    }
  }
}