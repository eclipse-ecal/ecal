#include "logging_attribute_builder.h"
#include "ecal/process.h"
#include "ecal/util.h"

namespace eCAL
{
  namespace Logging
  {
    SProviderAttributes BuildLoggingProviderAttributes(const Logging::Configuration& log_config_, const Registration::Configuration& reg_config_, const TransportLayer::Configuration& tl_config_)
    {
      SProviderAttributes attributes;

      attributes.host_name               = Process::GetHostName();
      attributes.process_id              = Process::GetProcessID();
      attributes.process_name            = Process::GetProcessName();
      attributes.unit_name               = Process::GetUnitName();

      attributes.udp_sink.enabled        = log_config_.provider.udp.enable;
      attributes.udp_sink.filter_log     = log_config_.provider.udp.filter_log;

      attributes.file_sink.enabled       = log_config_.provider.file.enable;
      attributes.file_sink.filter_log    = log_config_.provider.file.filter_log;
      
      attributes.file_config.path        = Util::GeteCALLogDir();

      attributes.console_sink.enabled    = log_config_.provider.console.enable;
      attributes.console_sink.filter_log = log_config_.provider.console.filter_log;

      // UDP related configuration part
      attributes.udp_config.broadcast    = !reg_config_.network_enabled;
      attributes.udp_config.loopback     = reg_config_.loopback;
      
      attributes.udp_config.sndbuf       = tl_config_.udp.send_buffer;
      attributes.udp_config.port         = log_config_.provider.udp_config.port;
      
      switch (tl_config_.udp.mode)
      {
        case Types::UDPMode::NETWORK:
          attributes.udp_config.address = tl_config_.udp.network.group;
          attributes.udp_config.ttl     = tl_config_.udp.network.ttl;
          break;
        case Types::UDPMode::LOCAL:
          attributes.udp_config.address = tl_config_.udp.local.group;
          attributes.udp_config.ttl     = tl_config_.udp.local.ttl;
          break;
        default:
          break;
      }

      return attributes;
    }

    SReceiverAttributes BuildLoggingReceiverAttributes(const Logging::Configuration& log_config_, const Registration::Configuration& reg_config_, const TransportLayer::Configuration& tl_config_)
    {
      SReceiverAttributes attributes;

      attributes.network_enabled          = reg_config_.network_enabled;
      attributes.host_name                = Process::GetHostName();

      attributes.receive_enabled          = log_config_.receiver.enable;

      attributes.udp_receiver.broadcast   = !reg_config_.network_enabled;
      attributes.udp_receiver.loopback    = true;
      
      attributes.udp_receiver.rcvbuf      = tl_config_.udp.receive_buffer;
      attributes.udp_receiver.port        = log_config_.receiver.udp_config.port;

      switch (tl_config_.udp.mode)
      {
        case Types::UDPMode::NETWORK:
          attributes.udp_receiver.address = tl_config_.udp.network.group;
          break;
        case Types::UDPMode::LOCAL:
          attributes.udp_receiver.address = tl_config_.udp.local.group;
          break;
        default:
          break;
      }

      return attributes;
    }
  }
}