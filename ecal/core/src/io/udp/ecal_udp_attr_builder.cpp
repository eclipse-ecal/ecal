
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

#include "ecal_udp_attr_builder.h"

namespace eCAL
{
  namespace UDP
  {
    SSenderAttr CreateUDPSenderAttr(const Registration::Configuration& registration_config_, const TransportLayer::UDP::Configuration& udp_config_)
    {
      SSenderAttr attr;
      attr.broadcast = !registration_config_.network_enabled;
      attr.port      = registration_config_.layer.udp.port;

      attr.loopback = true;
      attr.sndbuf   = udp_config_.send_buffer;

      if (udp_config_.mode == Types::UDPMode::NETWORK)
      {
        attr.address = udp_config_.network.group;
        attr.ttl     = udp_config_.network.ttl;
      } else
      {
        attr.address = udp_config_.local.group;
        attr.ttl     = udp_config_.local.ttl;
      }

      return attr;
    }

    SReceiverAttr CreateUDPReceiverAttr(const Registration::Configuration& registration_config_, const TransportLayer::UDP::Configuration& udp_config_)
    {
      SReceiverAttr attr;
      attr.port = registration_config_.layer.udp.port;
      attr.broadcast = !registration_config_.network_enabled;

      attr.rcvbuf  = udp_config_.receive_buffer;
      attr.loopback = true;

      if (udp_config_.mode == Types::UDPMode::NETWORK)
      {
        attr.address    = udp_config_.network.group;
      } else
      {
        attr.address = udp_config_.local.group;
      }
      return attr;
    }
  }
}