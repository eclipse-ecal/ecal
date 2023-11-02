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
#include "topic2mcast.h"

namespace eCAL
{
  namespace UDP
  {
    std::string LocalHost()
    {
      return "127.0.0.1";
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

    std::string GetLoggingMulticastAddress()
    {
      //TODO: At the moment, both logging and monitoring addresses seem to be the same
      // Should it be kept or changed?
      return GetRegistrationMulticastAddress();
    }

    std::string GetTopicMulticastAddress(const std::string& topic_name)
    {
      const bool local_only = !Config::IsNetworkEnabled();
      if (local_only)
      {
        return LocalHost();
      }
      else
      {
        if (topic_name.empty())
        {
          return Config::GetUdpMulticastGroup();
        }
        else
        {
          // v1
          if (Config::GetUdpMulticastConfigVersion() == Config::UdpConfigVersion::V1)
          {
            return UDP::V1::topic2mcast(topic_name, Config::GetUdpMulticastGroup(), Config::GetUdpMulticastMask());
          }
          // v2
          return  UDP::V2::topic2mcast(topic_name, Config::GetUdpMulticastGroup(), Config::GetUdpMulticastMask());
        }
      }
    }
  }
}
