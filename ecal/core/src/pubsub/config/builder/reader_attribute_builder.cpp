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

#pragma once

#include "reader_attribute_builder.h"

namespace eCAL
{
  namespace eCALReader
  {
    SAttributes BuildReaderAttributes(const Subscriber::Configuration& config_)
    {
      SAttributes attributes;

      attributes.enable_tcp = config_.layer.tcp.enable;
      attributes.enable_udp = config_.layer.udp.enable;
      attributes.enable_shm = config_.layer.shm.enable;
      attributes.drop_out_of_order_messages = config_.drop_out_of_order_messages;
      
      return attributes;
    }
  }
}