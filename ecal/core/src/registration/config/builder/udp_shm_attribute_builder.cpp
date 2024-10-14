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

#include "udp_shm_attribute_builder.h"

namespace eCAL
{
  namespace Registration
  {
    UDP::SSenderAttributes BuildUDPSenderAttributes(const SAttributes& provider_attr_)
    {
      UDP::SSenderAttributes sender_attr;
      sender_attr.broadcast      = !provider_attr_.network_enabled;
      sender_attr.loopback       = provider_attr_.loopback;
      
      sender_attr.sndbuf         = provider_attr_.udp.sendbuffer;
      sender_attr.port           = provider_attr_.udp.port;
      
      switch (provider_attr_.udp.mode)
      {
        case Types::UDPMode::NETWORK:
          sender_attr.address = provider_attr_.udp.network.group;
          sender_attr.ttl     = provider_attr_.udp.network.ttl;
          break;
        case Types::UDPMode::LOCAL:
          sender_attr.address = provider_attr_.udp.local.group;
          sender_attr.ttl     = provider_attr_.udp.local.ttl;
          break;
        default:
          break;
      }

      return sender_attr;
    }

    UDP::SReceiverAttributes BuildUDPReceiverAttributes(const SAttributes& provider_attr_)
    {
      UDP::SReceiverAttributes receiver_attr;      
      receiver_attr.broadcast = !provider_attr_.network_enabled;
      receiver_attr.loopback  = true;
      
      receiver_attr.receive_buffer    = provider_attr_.udp.receivebuffer;
      receiver_attr.port      = provider_attr_.udp.port;

      switch (provider_attr_.udp.mode)
      {
        case Types::UDPMode::NETWORK:
          receiver_attr.address = provider_attr_.udp.network.group;
          break;
        case Types::UDPMode::LOCAL:
          receiver_attr.address = provider_attr_.udp.local.group;
          break;
        default:
          break;
      }

      return receiver_attr;
    }

    SHM::SAttributes BuildSHMAttributes(const SAttributes& provider_attr_)
    {
      SHM::SAttributes sender_attr;      
      sender_attr.domain     = provider_attr_.shm.domain;
      sender_attr.queue_size = provider_attr_.shm.queue_size;
      return sender_attr;
    }
  }
}