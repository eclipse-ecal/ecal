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

#include "io/udp_configurations.h"

#include <ecal/ecal_config.h>

#include "ecal_def.h"
#include "topic2mcast.h"

namespace eCAL
{
  namespace UDP
  {
    std::string LocalHost()
    {
      // the specific address 127.255.255.255 is the broadcast address within the loopback range (127.0.0.0 to 127.255.255.255)
      return "127.255.255.255";
    }

    std::string GetRegistrationMulticastAddress()
    {
      const bool local_only = !Config::IsNetworkEnabled();
      if (local_only)
      {
        return LocalHost();
      }
      else
      {
        // both in v1 and v2, the mulicast group is returned as the adress for the registration layer
        return Config::GetUdpMulticastGroup();
      }
    }

    int GetRegistrationPort()
    {
      return Config::GetUdpMulticastPort() + NET_UDP_MULTICAST_PORT_REG_OFF;
    }
      
    std::string GetLoggingMulticastAddress()
    {
      // both logging and monitoring use the same addresses but different ports
      return GetRegistrationMulticastAddress();
    }

    int GetLoggingPort()
    {
      return Config::GetUdpMulticastPort() + NET_UDP_MULTICAST_PORT_LOG_OFF;
    }

    std::string GetPayloadMulticastAddress(const std::string& topic_name)
    {
      // v1
      if (Config::GetUdpMulticastConfigVersion() == Config::UdpConfigVersion::V1)
      {
        return UDP::V1::topic2mcast(topic_name, Config::GetUdpMulticastGroup(), Config::GetUdpMulticastMask());
      }

      // v2
      return  UDP::V2::topic2mcast(topic_name, Config::GetUdpMulticastGroup(), Config::GetUdpMulticastMask());
    }

    int GetPayloadPort()
    {
      return Config::GetUdpMulticastPort() + NET_UDP_MULTICAST_PORT_SAMPLE_OFF;
    }

    int GetMulticastTtl()
    {
      const bool local_only = !Config::IsNetworkEnabled();
      if (local_only)
      {
        return 1;
      }
      else
      {
        return Config::GetUdpMulticastTtl();
      }
    }
  }
}
