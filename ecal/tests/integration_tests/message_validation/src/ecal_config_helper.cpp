#include "ecal_config_helper.h"

#include <ecal/ecal.h>
#include <iostream>

void setup_ecal_configuration(const std::string& mode, bool is_publisher, const std::string& node_name)
{
  eCAL::Configuration config;

  if (mode == "local_shm")
  {
    config.communication_mode = eCAL::eCommunicationMode::local;
    config.registration.local.transport_type = eCAL::Registration::Local::eTransportType::shm;

    if (is_publisher)
    {
      config.publisher.layer_priority_local = { eCAL::TransportLayer::eType::shm };
    }
    else
    {
      config.subscriber.layer.shm.enable = true;
      config.subscriber.layer.udp.enable = false;
      config.subscriber.layer.tcp.enable = false;
    }
  }
  else if (mode == "local_udp")
  {
    config.communication_mode = eCAL::eCommunicationMode::local;
    config.registration.local.transport_type = eCAL::Registration::Local::eTransportType::udp;

    if (is_publisher)
    {
      config.publisher.layer_priority_local = { eCAL::TransportLayer::eType::udp_mc };
    }
    else
    {
      config.subscriber.layer.shm.enable = false;
      config.subscriber.layer.udp.enable = true;
      config.subscriber.layer.tcp.enable = false;
    }
  }
  else if (mode == "local_tcp")
  {
    config.communication_mode = eCAL::eCommunicationMode::local;
    config.registration.local.transport_type = eCAL::Registration::Local::eTransportType::udp;

    if (is_publisher)
    {
      config.publisher.layer_priority_local = { eCAL::TransportLayer::eType::tcp };
    }
    else
    {
      config.subscriber.layer.shm.enable = false;
      config.subscriber.layer.udp.enable = false;
      config.subscriber.layer.tcp.enable = true;
    }
  }
  else if (mode == "network_udp")
  {
    config.communication_mode = eCAL::eCommunicationMode::network;
    config.registration.network.transport_type = eCAL::Registration::Network::eTransportType::udp;

    if (is_publisher)
    {
      config.publisher.layer_priority_remote = { eCAL::TransportLayer::eType::udp_mc };
    }
    else
    {
      config.subscriber.layer.shm.enable = false;
      config.subscriber.layer.udp.enable = true;
      config.subscriber.layer.tcp.enable = false;
    }
  }
  else if (mode == "network_tcp")
  {
    config.communication_mode = eCAL::eCommunicationMode::network;
    config.registration.network.transport_type = eCAL::Registration::Network::eTransportType::udp;

    if (is_publisher)
    {
      config.publisher.layer_priority_remote = { eCAL::TransportLayer::eType::tcp };
    }
    else
    {
      config.subscriber.layer.shm.enable = false;
      config.subscriber.layer.udp.enable = false;
      config.subscriber.layer.tcp.enable = true;
    }
  }
  else
  {
    std::cerr << "[eCAL Config Helper] Unknown mode: " << mode << std::endl;
    exit(1);
  }

  eCAL::Initialize(config, node_name, eCAL::Init::All);
}
