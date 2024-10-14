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
  namespace Registration
  {
    namespace UDP
    {
      eCAL::UDP::SSenderAttr ConvertToIOUDPSenderAttributes (const Registration::UDP::SSenderAttributes& sender_attr_)
      {
        eCAL::UDP::SSenderAttr attr;
        attr.broadcast = sender_attr_.broadcast;
        attr.loopback  = sender_attr_.loopback;
        attr.sndbuf    = sender_attr_.sndbuf;
        attr.port      = sender_attr_.port;
        attr.address   = sender_attr_.address;
        attr.ttl       = sender_attr_.ttl;
        return attr;
      }

      eCAL::UDP::SReceiverAttr ConvertToIOUDPReceiverAttributes (const Registration::UDP::SReceiverAttributes& receiver_attr_)
      {
        eCAL::UDP::SReceiverAttr attr;
        attr.broadcast = receiver_attr_.broadcast;
        attr.loopback  = receiver_attr_.loopback;
        attr.rcvbuf    = receiver_attr_.receive_buffer;
        attr.port      = receiver_attr_.port;
        attr.address   = receiver_attr_.address;
        return attr;
      }
    }
  }
}