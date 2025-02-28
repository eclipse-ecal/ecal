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

#include "udp_shm_attribute_builder.h"
#include "ecal/config/configuration.h"

namespace eCAL
{
  namespace Registration
  {
    UDP::SSenderAttributes BuildUDPSenderAttributes(const SAttributes& provider_attr_)
    {
      UDP::SSenderAttributes sender_attr;

      sender_attr.broadcast = provider_attr_.communication_mode == eCommunicationMode::local ? true : false;
      sender_attr.loopback  = provider_attr_.loopback;
      sender_attr.sndbuf    = provider_attr_.udp.sendbuffer;
      sender_attr.port      = provider_attr_.udp.port;
      sender_attr.address   = provider_attr_.udp.group;
      sender_attr.ttl       = provider_attr_.udp.ttl;

      return sender_attr;
    }

    UDP::SReceiverAttributes BuildUDPReceiverAttributes(const SAttributes& provider_attr_)
    {
      UDP::SReceiverAttributes receiver_attr;

      receiver_attr.broadcast      = provider_attr_.communication_mode == eCommunicationMode::local ? true : false;
      receiver_attr.loopback       = true;
      receiver_attr.receive_buffer = provider_attr_.udp.receivebuffer;
      receiver_attr.port           = provider_attr_.udp.port;
      receiver_attr.address        = provider_attr_.udp.group;
   
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