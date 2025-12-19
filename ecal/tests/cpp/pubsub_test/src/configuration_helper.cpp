#include "configuration_helper.h"

void eCAL::PrintTo(const Configuration& config, std::ostream* os) {
    *os << "Config Layers: ";
    if (config.publisher.layer.shm.enable)
        *os << "SHM ";
    if (config.publisher.layer.tcp.enable)
        *os << "TCP ";
    if (config.publisher.layer.udp.enable)
        *os << "UDP ";
}

eCAL::Configuration GetTestingConfig()
{
  eCAL::Configuration config;
  config.registration.registration_refresh = 100;
  config.registration.registration_timeout = 200;

  config.publisher.layer.shm.enable = false;
  config.publisher.layer.udp.enable = false;
  config.publisher.layer.tcp.enable = false;

  config.subscriber.layer.shm.enable = false;
  config.subscriber.layer.udp.enable = false;
  config.subscriber.layer.tcp.enable = false;

  return config;
}

eCAL::Configuration EnableUDP(const eCAL::Configuration& config_)
{
  eCAL::Configuration config(config_);
  config.publisher.layer.udp.enable = true;
  config.subscriber.layer.udp.enable = true;
  return config;
}

eCAL::Configuration EnableTCP(const eCAL::Configuration& config_)
{
  eCAL::Configuration config(config_);
  config.publisher.layer.tcp.enable = true;
  config.subscriber.layer.tcp.enable = true;
  return config;
}

eCAL::Configuration EnableSHM(const eCAL::Configuration& config_)
{
  eCAL::Configuration config(config_);
  config.publisher.layer.shm.enable = true;
  config.subscriber.layer.shm.enable = true;
  return config;
}

void PrintTo(const TestParams& config, std::ostream* os) 
{
  eCAL::PrintTo(config.config, os);
}
