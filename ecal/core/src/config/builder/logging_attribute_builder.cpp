#include "logging_attribute_builder.h"
#include "ecal/ecal_process.h"
#include "ecal/ecal_util.h"

namespace eCAL
{
  namespace Logging
  {
    SProviderAttributes BuildLoggingProviderAttributes(const Logging::Configuration& log_config_, const Registration::Configuration& reg_config_, const TransportLayer::Configuration& tl_config_)
    {
      SProviderAttributes attributes;

      attributes.host_name          = Process::GetHostName();
      attributes.process_id         = Process::GetProcessID();
      attributes.process_name       = Process::GetProcessName();
      attributes.unit_name          = Process::GetUnitName();
      attributes.level              = log_level_info;

      attributes.udp_sink.enabled        = log_config_.sinks.udp.enable;
      attributes.udp_sink.filter_log     = log_config_.sinks.udp.filter_log;

      attributes.file_sink.enabled       = log_config_.sinks.file.enable;
      attributes.file_sink.filter_log    = log_config_.sinks.file.filter_log;
      attributes.file_config.path        = log_config_.sinks.file_config.path;
      if (attributes.file_config.path.empty())
      {
        // check ECAL_DATA
        // Creates path if not exists
        attributes.file_config.path = Util::GeteCALLogPath();        
      }

      attributes.console_sink.enabled    = log_config_.sinks.console.enable;
      attributes.console_sink.filter_log = log_config_.sinks.console.filter_log;

      // UDP related configuration part
      attributes.udp_config.broadcast = !reg_config_.network_enabled;
      attributes.udp_config.loopback  = reg_config_.loopback;
      
      attributes.udp_config.sndbuf    = tl_config_.udp.send_buffer;
      attributes.udp_config.port      = log_config_.sinks.udp_config.provider.port;
      
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

      attributes.network_enabled    = reg_config_.network_enabled;
      attributes.host_name          = Process::GetHostName();

      attributes.receive_enabled    = log_config_.sinks.udp_config.receiver.enable;

      attributes.udp_receiver.broadcast = !reg_config_.network_enabled;
      attributes.udp_receiver.loopback  = true;
      
      attributes.udp_receiver.rcvbuf    = tl_config_.udp.receive_buffer;
      attributes.udp_receiver.port      = log_config_.sinks.udp_config.receiver.port;

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