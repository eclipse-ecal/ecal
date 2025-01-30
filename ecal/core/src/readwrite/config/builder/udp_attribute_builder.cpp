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

#include "udp_attribute_builder.h"
#include "ecal/types/custom_data_types.h"

namespace eCAL
{
  namespace eCALReader
  {
    UDP::SAttributes BuildUDPAttributes(const eCALReader::SAttributes& attr_)
    {
      UDP::SAttributes attributes;

      attributes.broadcast = !attr_.network_enabled;
      attributes.loopback  = true;
      
      attributes.receive_buffer = attr_.udp.receivebuffer;
      attributes.port           = attr_.udp.port;

      switch (attr_.udp.mode)
      {
        case Types::UDPMode::NETWORK:
          attributes.address = attr_.udp.network.group;
          break;
        case Types::UDPMode::LOCAL:
          attributes.address = attr_.udp.local.group;
          break;
        default:
          break;
      }
      
      return attributes;
    }    
  }

  namespace eCALWriter
  {
    UDP::SAttributes BuildUDPAttributes(const uint64_t& topic_id_, const eCALWriter::SAttributes& attr_)
    {
      UDP::SAttributes attributes;

      attributes.broadcast      = !attr_.network_enabled;
      attributes.loopback       = attr_.loopback;

      attributes.topic_id       = topic_id_;
      attributes.topic_name     = attr_.topic_name;
      attributes.host_name      = attr_.host_name;
      
      attributes.send_buffer    = attr_.udp.send_buffer;
      attributes.port           = attr_.udp.port;
      
      switch (attr_.udp.mode)
      {
        case Types::UDPMode::NETWORK:
          attributes.address = attr_.udp.network.group;
          attributes.ttl     = attr_.udp.network.ttl;
          break;
        case Types::UDPMode::LOCAL:
          attributes.address = attr_.udp.local.group;
          attributes.ttl     = attr_.udp.local.ttl;
          break;
        default:
          break;
      }

      return attributes;
    }
  }
}