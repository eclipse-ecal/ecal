#include "logging_attribute_builder.h"
#include "ecal/ecal_process.h"
#include "ecal/ecal_util.h"

namespace eCAL
{
  namespace Logging
  {
    SAttributes BuildLoggingAttributes(const Logging::Configuration& log_config_, const Registration::Configuration& reg_config_, const TransportLayer::Configuration& tl_config_)
    {
      SAttributes attributes;

      attributes.network_enabled    = reg_config_.network_enabled;
      attributes.host_name          = Process::GetHostName();
      attributes.process_id         = Process::GetProcessID();
      attributes.process_name       = Process::GetProcessName();
      attributes.unit_name          = Process::GetUnitName();
      attributes.level              = log_level_info;

      attributes.udp.enabled        = log_config_.sinks.udp.enable;
      attributes.udp.port           = log_config_.sinks.udp.port;
      attributes.udp.filter_log     = log_config_.sinks.udp.filter_log_udp;

      attributes.file.enabled       = log_config_.sinks.file.enable;
      attributes.file.filter_log    = log_config_.sinks.file.filter_log_file;
      attributes.file.path          = log_config_.sinks.file.path;
      if (attributes.file.path.empty())
      {
        // check ECAL_DATA
        // Creates path if not exists
        attributes.file.path = Util::GeteCALLogPath();        
      }

      attributes.console.enabled    = log_config_.sinks.console.enable;
      attributes.console.filter_log = log_config_.sinks.console.filter_log_con;

      // UDP related configuration part
      attributes.udp_sender.broadcast = !reg_config_.network_enabled;
      attributes.udp_sender.loopback  = reg_config_.loopback;
      
      attributes.udp_sender.sndbuf    = tl_config_.udp.send_buffer;
      attributes.udp_sender.port      = tl_config_.udp.port;
      
      switch (tl_config_.udp.mode)
      {
        case Types::UDPMode::NETWORK:
          attributes.udp_sender.address = tl_config_.udp.network.group;
          attributes.udp_sender.ttl     = tl_config_.udp.network.ttl;
          break;
        case Types::UDPMode::LOCAL:
          attributes.udp_sender.address = tl_config_.udp.local.group;
          attributes.udp_sender.ttl     = tl_config_.udp.local.ttl;
          break;
        default:
          break;
      }

      attributes.udp_receiver.broadcast = !reg_config_.network_enabled;
      attributes.udp_receiver.loopback  = true;
      
      attributes.udp_receiver.rcvbuf    = tl_config_.udp.receive_buffer;
      attributes.udp_receiver.port      = tl_config_.udp.port;

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