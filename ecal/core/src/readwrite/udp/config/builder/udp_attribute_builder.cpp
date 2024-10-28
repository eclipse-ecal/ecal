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

#include "udp_attribute_builder.h"

namespace eCAL
{
  namespace eCALReader
  {
    namespace UDP
    {
      eCAL::UDP::SReceiverAttr ConvertToIOUDPReceiverAttributes(const eCAL::eCALReader::UDP::SAttributes& attr_)
      {
        eCAL::UDP::SReceiverAttr receiver_attr;      
        receiver_attr.broadcast = attr_.broadcast;
        receiver_attr.loopback  = attr_.loopback;
        
        receiver_attr.rcvbuf    = attr_.receive_buffer;
        receiver_attr.port      = attr_.port;
        receiver_attr.address   = attr_.address;

        return receiver_attr;
      }
    }
  }

  namespace eCALWriter
  {
    namespace UDP
    {
      eCAL::UDP::SSenderAttr ConvertToIOUDPSenderAttributes(const SAttributes& attr_)
      {
        eCAL::UDP::SSenderAttr sender_attr;
        sender_attr.broadcast = attr_.broadcast;
        sender_attr.loopback  = attr_.loopback;
        
        sender_attr.sndbuf    = attr_.send_buffer;
        sender_attr.port      = attr_.port;
        sender_attr.address   = attr_.address;
        sender_attr.ttl       = attr_.ttl;

        return sender_attr;
      }
    }
  }
}