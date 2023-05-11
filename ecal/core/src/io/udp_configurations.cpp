#include "io/udp_configurations.h"

#include <ecal/ecal_config.h>
#include "topic2mcast.h"

const std::string localhost_udp_address{ "127.255.255.255" };


std::string eCAL::UDP::GetRegistrationMulticastAddress()
{
  const bool local_only = !Config::IsNetworkEnabled();
  if (local_only)
  {
    return localhost_udp_address;
  }
  else
  {
    // both in v1 and v2, the mulicast group is returned as the adress for the registration layer
    return Config::GetUdpMulticastGroup();
  }
}

std::string eCAL::UDP::GetLoggingMulticastAddress()
{
  //TODO: At the moment, both logging and monitoring addresses seem to be the same
  // Should it be kept or changed?
  return GetRegistrationMulticastAddress();
}

std::string eCAL::UDP::GetTopicMulticastAddress(const std::string& topic_name)
{
  if (Config::GetUdpMulticastConfigVersion() == Config::UdpConfigVersion::V1)
    return UDP::V1::topic2mcast(topic_name, Config::GetUdpMulticastGroup(), Config::GetUdpMulticastMask());
  // v2
  return  UDP::V2::topic2mcast(topic_name, Config::GetUdpMulticastGroup(), Config::GetUdpMulticastMask());
}
