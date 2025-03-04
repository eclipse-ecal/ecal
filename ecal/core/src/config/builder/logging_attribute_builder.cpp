#include "logging_attribute_builder.h"
#include "ecal/process.h"
#include "ecal/util.h"

namespace eCAL
{
  namespace Logging
  {
    SProviderAttributes BuildLoggingProviderAttributes(const eCAL::Configuration& config_)
    {
      const auto& logging_config         = config_.logging;
      const auto& transport_layer_config = config_.transport_layer;
      const auto& registration_config    = config_.registration;

      SProviderAttributes attributes;

      attributes.host_name               = Process::GetHostName();
      attributes.process_id              = Process::GetProcessID();
      attributes.process_name            = Process::GetProcessName();
      attributes.unit_name               = Process::GetUnitName();

      attributes.udp_sink.enabled        = log_config_.provider.udp.enable;
      attributes.udp_sink.log_level     = log_config_.provider.udp.log_level;

      attributes.file_sink.enabled       = log_config_.provider.file.enable;
      attributes.file_sink.log_level     = log_config_.provider.file.log_level;
      
      attributes.file_config.path        = Util::GeteCALLogDir();

      attributes.console_sink.enabled    = log_config_.provider.console.enable;
      attributes.console_sink.log_level  = log_config_.provider.console.log_level;

      // UDP related configuration part
      attributes.udp_config.broadcast    = config_.communication_mode == eCAL::eCommunicationMode::local;
      attributes.udp_config.loopback     = registration_config.loopback;
      
      attributes.udp_config.sndbuf       = transport_layer_config.udp.send_buffer;
      attributes.udp_config.port         = logging_config.provider.udp_config.port;
      
      switch (config_.communication_mode)
      {
        case eCAL::eCommunicationMode::network:
          attributes.udp_config.address = transport_layer_config.udp.network.group;
          attributes.udp_config.ttl     = transport_layer_config.udp.network.ttl;
          break;
        case eCAL::eCommunicationMode::local:
          attributes.udp_config.address = transport_layer_config.udp.local.group;
          attributes.udp_config.ttl     = transport_layer_config.udp.local.ttl;
          break;
        default:
          break;
      }

      return attributes;
    }

    SReceiverAttributes BuildLoggingReceiverAttributes(const eCAL::Configuration& config_)
    {
      const auto& logging_config         = config_.logging;
      const auto& transport_layer_config = config_.transport_layer;

      SReceiverAttributes attributes;

      attributes.network_enabled          = config_.communication_mode == eCAL::eCommunicationMode::network;
      attributes.host_name                = Process::GetHostName();

      attributes.receive_enabled          = logging_config.receiver.enable;

      attributes.udp_receiver.broadcast   = config_.communication_mode == eCAL::eCommunicationMode::local;
      attributes.udp_receiver.loopback    = true;
      
      attributes.udp_receiver.rcvbuf      = transport_layer_config.udp.receive_buffer;
      attributes.udp_receiver.port        = logging_config.receiver.udp_config.port;

      switch (config_.communication_mode)
      {
        case eCAL::eCommunicationMode::network:
          attributes.udp_receiver.address = transport_layer_config.udp.network.group;
          break;
        case eCAL::eCommunicationMode::local:
          attributes.udp_receiver.address = transport_layer_config.udp.local.group;
          break;
        default:
          break;
      }

      return attributes;
    }
  }
}