#include "ecal_config_helper.h"

#include <ecal/ecal.h>
#include <iostream>

#include <set>
#include <chrono>

void wait_for_subscriber(const std::string& topic_name, int min_subscribers = 1, int timeout_ms = 5000)
{
  int waited_ms = 0;
  const int interval_ms = 100;

  std::cout << "[Publisher] Waiting for at least " << min_subscribers
            << " subscriber(s) on topic: " << topic_name << std::endl;

  while (waited_ms < timeout_ms)
  {
    std::set<eCAL::STopicId> sub_ids;
    if (eCAL::Registration::GetSubscriberIDs(sub_ids))
    {
      int count = 0;
      for (const auto& id : sub_ids)
      {
        if (id.topic_name == topic_name)
        {
          ++count;
        }
      }

      if (count >= min_subscribers)
      {
        std::cout << "[Publisher] Found " << count << " subscriber(s). Proceeding to send messages." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        return;
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
    waited_ms += interval_ms;
  }

  std::cout << "[Publisher] Timeout reached. Only " << waited_ms / interval_ms
            << " intervals passed. Proceeding anyway." << std::endl;

}

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
