#include "io/udp_configurations.h"

#include <ecal/ecal_config.h>
#include "topic2mcast.h"

const std::string localhost_udp_address{ "127.255.255.255" };


std::string eCAL::UDP::GetRegistrationMulticastAdress()
{
  bool local_only = !Config::IsNetworkEnabled();
  if (local_only)
  {
    return localhost_udp_address;
  }
  else
  {
    return Config::GetUdpMulticastGroup();
  }
}

std::string eCAL::UDP::GetLoggingMulticastAdress()
{
  //TODO: At the moment, both logging and monitoring addresses seem to be the same
  // Should it be kept or changed?
  return GetRegistrationMulticastAdress();
}

std::string eCAL::UDP::GetTopicMulticastAdress(const std::string& topic_name)
{
  return topic2mcast(topic_name, Config::GetUdpMulticastGroup(), Config::GetUdpMulticastMask());
}
