#include "logging_attribute_builder.h"
#include "ecal/ecal_process.h"

namespace eCAL
{
  namespace Logging
  {
    SAttributes BuildLoggingAttributes(const Logging::Configuration& log_config_, const Registration::Configuration& reg_config_)
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
      attributes.file.path          = log_config_.sinks.file.path;
      attributes.file.filter_log    = log_config_.sinks.file.filter_log_file;

      attributes.console.enabled    = log_config_.sinks.console.enable;
      attributes.console.filter_log = log_config_.sinks.console.filter_log_con;

      return attributes;
    }
  }
}