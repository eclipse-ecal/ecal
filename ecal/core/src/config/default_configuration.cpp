#include "default_configuration.h"

#include "ecal/ecal_config.h"

#include <string>
#include <fstream>

namespace 
{
  std::string quoteString(const char* str_) {
    return std::string("\"") + std::string(str_) + std::string("\"");
  }

  std::string quoteString(const std::string& str_) {
    return std::string("\"") + str_ + std::string("\"");
  }

  std::string logToArray(const eCAL_Logging_Filter& filter_)
  {
    std::string result = "[";    
    if ((filter_ & log_level_info) != 0)    result += "\"info\", ";
    if ((filter_ & log_level_warning) != 0) result += "\"warning\", ";
    if ((filter_ & log_level_error) != 0)   result += "\"error\", ";
    if ((filter_ & log_level_fatal) != 0)   result += "\"fatal\", ";
    if ((filter_ & log_level_debug1) != 0)  result += "\"debug1\", ";
    if ((filter_ & log_level_debug2) != 0)  result += "\"debug2\", ";
    if ((filter_ & log_level_debug3) != 0)  result += "\"debug3\", ";
    if ((filter_ & log_level_debug4) != 0)  result += "\"debug4\", ";
    
    if (result.size() == 1 && (filter_ & log_level_all) != 0)
    {
      result += "\"all\", ";
    }

    if (result.size() > 1)
    {
      // remove the last ", "
      result.pop_back();
      result.pop_back();
    }
    
    result += "]";
    return result;
  }

  std::string quoteString(const eCAL::Publisher::Configuration::LayerPriorityVector& vector_)
  {
    std::string result = "[";
    for (const auto& elem : vector_ )
    {
      switch (elem)
      {
        case eCAL::TLayer::tlayer_shm:
          result += "\"shm\", ";
          break;
        case eCAL::TLayer::tlayer_udp_mc:
          result += "\"udp\", ";
          break;
        case eCAL::TLayer::tlayer_tcp:
          result += "\"tcp\", ";
          break;
        default:
          break;
      }
    }

    if (!vector_.empty())
    {
      // remove the last ", "
      result.pop_back();
      result.pop_back();
    }

    result += "]";
    return result;
  }

  std::string quoteString(const eCAL::Types::UdpConfigVersion config_version_) {
    switch (config_version_)
    {
      case eCAL::Types::UdpConfigVersion::V1:
        return "\"v1\"";
        break;
      case eCAL::Types::UdpConfigVersion::V2:
        return "\"v2\"";
        break;
      
      default:
        return "";
        break;
    }
  }

  std::string quoteString(const eCAL::Types::UDPMode mode_)
  {
    switch (mode_)
    {
      case eCAL::Types::UDPMode::LOCAL:
        return "\"local\"";
        break;
      case eCAL::Types::UDPMode::NETWORK:
        return "\"network\"";
        break;
      
      default:
        return "";
        break;
    }
  }

  std::string quoteString(const eCAL::Types::IpAddressV4& ip_)
  {
    return std::string("\"") + ip_.Get() + std::string("\"");
  }
}

namespace eCAL
{
  namespace Config
  {
    std::stringstream getConfigAsYamlSS(const eCAL::Configuration& config_)
    {
      std::stringstream ss;
      ss << std::boolalpha;
      ss << R"(#  _____     _ _                                ____    _    _                             )"                        << "\n";
      ss << R"(# | ____|___| (_)_ __  ___  ___            ___ / ___|  / \  | |                            )"                        << "\n";
      ss << R"(# |  _| / __| | | '_ \/ __|/ _ \  _____   / _ \ |     / _ \ | |                            )"                        << "\n";
      ss << R"(# | |__| (__| | | |_) \__ \  __/ |_____| |  __/ |___ / ___ \| |___                         )"                        << "\n";
      ss << R"(# |_____\___|_|_| .__/|___/\___|          \___|\____/_/   \_\_____|                        )"                        << "\n";
      ss << R"(#               |_|                                                                            )"                    << "\n";
      ss << R"(#        _       _           _                    __ _                       _   _             )"                    << "\n";
      ss << R"(#   __ _| | ___ | |__   __ _| |   ___ ___  _ __  / _(_) __ _ _   _ _ __ __ _| |_(_) ___  _ __  )"                    << "\n";
      ss << R"(#  / _` | |/ _ \| '_ \ / _` | |  / __/ _ \| '_ \| |_| |/ _` | | | | '__/ _` | __| |/ _ \| '_ \ )"                    << "\n";
      ss << R"(# | (_| | | (_) | |_) | (_| | | | (_| (_) | | | |  _| | (_| | |_| | | | (_| | |_| | (_) | | | |)"                    << "\n";
      ss << R"(#  \__, |_|\___/|_.__/ \__,_|_|  \___\___/|_| |_|_| |_|\__, |\__,_|_|  \__,_|\__|_|\___/|_| |_|)"                    << "\n";
      ss << R"(#  |___/                                               |___/                                   )"                    << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"(# Registration layer configuration)"                                                                                 << "\n";
      ss << R"(registration:)"                                                                                                      << "\n";
      ss << R"(  # Topic registration refresh cylce (has to be smaller then registration timeout! Default: 1000))"                  << "\n";
      ss << R"(  registration_refresh: )"                            << config_.registration.registration_refresh                   << "\n";
      ss << R"(  # Timeout for topic registration in ms (internal, Default: 60000))"                                                << "\n";
      ss << R"(  registration_timeout: )"                            << config_.registration.registration_timeout                   << "\n";
      ss << R"(  # Enable to receive registration information on the same local machine)"                                           << "\n";
      ss << R"(  loopback: )"                                        << config_.registration.loopback                               << "\n";
      ss << R"(  # Host group name that enables interprocess mechanisms across (virtual))"                                          << "\n";
      ss << R"(  # host borders (e.g, Docker); by default equivalent to local host name)"                                           << "\n";
      ss << R"(  host_group_name: )"                                 << quoteString(config_.registration.host_group_name)           << "\n";
      ss << R"(  # true  = all eCAL components communicate over network boundaries)"                                                << "\n";
      ss << R"(  # false = local host only communication (Default: false))"                                                         << "\n";
      ss << R"(  network_enabled: )"                                 << config_.registration.network_enabled                        << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"(  layer:)"                                                                                                           << "\n";
      ss << R"(    shm:)"                                                                                                           << "\n";
      ss << R"(      enable: )"                                      << config_.registration.layer.shm.enable                       << "\n";
      ss << R"(      # Domain name for shared memory based registration)"                                                           << "\n";
      ss << R"(      domain:  )"                                     << quoteString(config_.registration.layer.shm.domain)          << "\n";
      ss << R"(      # Queue size of registration events)"                                                                          << "\n";
      ss << R"(      queue_size: )"                                  << config_.registration.layer.shm.queue_size                   << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"(    udp:)"                                                                                                           << "\n";
      ss << R"(      enable: )"                                      << config_.registration.layer.udp.enable                       << "\n";
      ss << R"(      port: )"                                        << config_.registration.layer.udp.port                         << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"(# Monitoring configuration)"                                                                                         << "\n";
      ss << R"(monitoring:)"                                                                                                        << "\n";
      ss << R"(  # Timeout for topic monitoring in ms (Default: 5000), increase in 1000er steps)"                                   << "\n";
      ss << R"(  timeout: )"                                         << config_.monitoring.timeout                                  << "\n";
      ss << R"(  # Topics blacklist as regular expression (will not be monitored))"                                                 << "\n";
      ss << R"(  filter_excl: )"                                     << quoteString(config_.monitoring.filter_excl)                 << "\n";
      ss << R"(  # Topics whitelist as regular expression (will be monitored only) (Default: ""))"                                  << "\n";
      ss << R"(  filter_incl: )"                                     << quoteString(config_.monitoring.filter_incl)                 << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"(# Transport layer configuration)"                                                                                    << "\n";
      ss << R"(transport_layer:)"                                                                                                   << "\n";
      ss << R"(  udp:)"                                                                                                             << "\n";
      ss << R"(    # UDP configuration version (Since eCAL 5.12.))"                                                                 << "\n";
      ss << R"(    # v1: default behavior)"                                                                                         << "\n";
      ss << R"(    # v2: new behavior, comes with a bit more intuitive handling regarding masking of the groups)"                   << "\n";
      ss << R"(    config_version: )"                                << quoteString(config_.transport_layer.udp.config_version)     << "\n";
      ss << R"(    # Valid modes: local, network (Default: local))"                                                                 << "\n";
      ss << R"(    mode: )"                                          << quoteString(config_.transport_layer.udp.mode)               << "\n";
      ss << R"(    # Multicast port number)"                                                                                        << "\n";
      ss << R"(    port: )"                                          << config_.transport_layer.udp.port                            << "\n";
      ss << R"(    # v1: Mask maximum number of dynamic multicast group (range 0.0.0.1-0.0.0.255))"                                 << "\n";
      ss << R"(    # v2: Masks are now considered like routes masking (range 255.0.0.0-255.255.255.255))"                           << "\n";
      ss << R"(    mask: )"                                          << quoteString(config_.transport_layer.udp.mask)               << "\n";
      ss << R"(    # Send buffer in bytes)"                                                                                         << "\n";
      ss << R"(    send_buffer: )"                                   << config_.transport_layer.udp.send_buffer                     << "\n";
      ss << R"(    # Receive buffer in bytes)"                                                                                      << "\n";
      ss << R"(    receive_buffer: )"                                << config_.transport_layer.udp.receive_buffer                  << "\n";
      ss << R"(    # Linux specific setting to join all network interfaces independend of their link state.)"                       << "\n";
      ss << R"(    # Enabling ensures that eCAL processes receive data when they are started before the)"                           << "\n";
      ss << R"(    # network devices are up and running.)"                                                                          << "\n";
      ss << R"(    join_all_interfaces: )"                           << config_.transport_layer.udp.join_all_interfaces             << "\n";
      ss << R"(    # Windows specific setting to enable receiving UDP traffic with the Npcap based receiver)"                       << "\n";
      ss << R"(    npcap_enabled: )"                                 << config_.transport_layer.udp.npcap_enabled                   << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"(    # In local mode multicast group and ttl are set by default and are not adjustable)"                              << "\n";
      ss << R"(    local:)"                                                                                                         << "\n";
      ss << R"(      # Multicast group base. All registration and logging is sent on this address)"                                 << "\n";
      ss << R"(      # group: "127.0.0.1")"                                                                                         << "\n";
      ss << R"(      # TTL (hop limit) is used to determine the amount of routers being traversed towards the destination)"         << "\n";
      ss << R"(      # ttl: 0)"                                                                                                     << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"(    network:)"                                                                                                       << "\n";
      ss << R"(      # Multicast group base. All registration and logging is sent on this address)"                                 << "\n";
      ss << R"(      group: )"                                       << quoteString(config_.transport_layer.udp.network.group)      << "\n";
      ss << R"(      # TTL (hop limit) is used to determine the amount of routers being traversed towards the destination)"         << "\n";
      ss << R"(      ttl: )"                                         << config_.transport_layer.udp.network.ttl                     << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"(  tcp: )"                                                                                                            << "\n";
      ss << R"(    # Reader amount of threads that shall execute workload)"                                                         << "\n";
      ss << R"(    number_executor_reader: )"                        << config_.transport_layer.tcp.number_executor_reader          << "\n";
      ss << R"(    # Writer amount of threads that shall execute workload)"                                                         << "\n";
      ss << R"(    number_executor_writer: )"                        << config_.transport_layer.tcp.number_executor_writer          << "\n";
      ss << R"(    # Reconnection attemps the session will try to reconnect in case of an issue)"                                   << "\n";
      ss << R"(    max_reconnections: )"                             << config_.transport_layer.tcp.max_reconnections               << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"(  shm:)"                                                                                                             << "\n";
      ss << R"(    # Default memory file size for new publisher)"                                                                   << "\n";
      ss << R"(    memfile_min_size_bytes: )"                        << config_.transport_layer.shm.memfile_min_size_bytes          << "\n";
      ss << R"(    # Dynamic file size reserve before recreating memory file if topic size changes)"                                << "\n";
      ss << R"(    memfile_reserve_percent: )"                       << config_.transport_layer.shm.memfile_reserve_percent         << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"(# Publisher specific base settings)"                                                                                 << "\n";
      ss << R"(publisher:)"                                                                                                         << "\n";
      ss << R"(  layer:)"                                                                                                           << "\n";
      ss << R"(    # Base configuration for shared memory publisher)"                                                               << "\n";
      ss << R"(    shm:)"                                                                                                           << "\n";
      ss << R"(      # Enable layer)"                                                                                               << "\n";
      ss << R"(      enable: )"                                      << config_.publisher.layer.shm.enable                          << "\n";
      ss << R"(      # Enable zero copy shared memory transport mode)"                                                              << "\n";
      ss << R"(      zero_copy_mode: )"                              << config_.publisher.layer.shm.zero_copy_mode                  << "\n";
      ss << R"(      # Force connected subscribers to send acknowledge event after processing the message.)"                        << "\n";
      ss << R"(      # The publisher send call is blocked on this event with this timeout (0 == no handshake).)"                    << "\n";
      ss << R"(      acknowledge_timeout_ms: )"                      << config_.publisher.layer.shm.acknowledge_timeout_ms          << "\n";
      ss << R"(      # Maximum number of used buffers (needs to be greater than 1, default = 1))"                                   << "\n";
      ss << R"(      memfile_buffer_count: )"                        << config_.publisher.layer.shm.memfile_buffer_count            << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"(    # Base configuration for UDP publisher)"                                                                         << "\n";
      ss << R"(    udp:)"                                                                                                           << "\n";
      ss << R"(      # Enable layer)"                                                                                               << "\n";
      ss << R"(      enable: )"                                      << config_.publisher.layer.udp.enable                          << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"(    # Base configuration for TCP publisher)"                                                                         << "\n";
      ss << R"(    tcp:)"                                                                                                           << "\n";
      ss << R"(      # Enable layer)"                                                                                               << "\n";
      ss << R"(      enable: )"                                      << config_.publisher.layer.shm.enable                          << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"(  # Share topic type via registration)"                                                                              << "\n";
      ss << R"(  share_topic_type: )"                                << config_.publisher.share_topic_type                          << "\n";
      ss << R"(  # Share topic description via registration)"                                                                       << "\n";
      ss << R"(  share_topic_description: )"                         << config_.publisher.share_topic_description                   << "\n";
      ss << R"(  # Priority list for layer usage in local mode (Default: SHM > UDP > TCP))"                                         << "\n";
      ss << R"(  priority_local: )"                                  << quoteString(config_.publisher.layer_priority_local)         << "\n";
      ss << R"(  # Priority list for layer usage in cloud mode (Default: UDP > TCP))"                                               << "\n";
      ss << R"(  priority_network: )"                                << quoteString(config_.publisher.layer_priority_remote)        << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"(# Subscriber specific base configuration)"                                                                           << "\n";
      ss << R"(subscriber:)"                                                                                                        << "\n";
      ss << R"(  layer:)"                                                                                                           << "\n";
      ss << R"(    # Base configuration for shared memory subscriber)"                                                              << "\n";
      ss << R"(    shm:)"                                                                                                           << "\n";
      ss << R"(      # Enable layer)"                                                                                               << "\n";
      ss << R"(      enable: )"                                        << config_.subscriber.layer.shm.enable                       << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"(    # Base configuration for UDP subscriber)"                                                                        << "\n";
      ss << R"(    udp:)"                                                                                                           << "\n";
      ss << R"(      # Enabler layer)"                                                                                              << "\n";
      ss << R"(      enable: )"                                        << config_.subscriber.layer.udp.enable                       << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"(    # Base configuration for TCP subscriber)"                                                                        << "\n";
      ss << R"(    tcp:)"                                                                                                           << "\n";
      ss << R"(      # Enable layer)"                                                                                               << "\n";
      ss << R"(      enable: )"                                        << config_.subscriber.layer.tcp.enable                       << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"(  # Enable dropping of payload messages that arrive out of order)"                                                   << "\n";
      ss << R"(  drop_out_of_order_messages: )"                        << config_.subscriber.drop_out_of_order_messages             << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"(# Time configuration)"                                                                                               << "\n";
      ss << R"(time:)"                                                                                                              << "\n";
      ss << R"(  # Time synchronisation interface name (dynamic library))"                                                          << "\n";
      ss << R"(  # The name will be extended with platform suffix (32|64), debug suffix (d) and platform extension (.dll|.so))"     << "\n";
      ss << R"(  # Available modules are:)"                                                                                         << "\n";
      ss << R"(  #   - ecaltime-localtime    local system time without synchronization)"                                            << "\n";
      ss << R"(  #   - ecaltime-linuxptp     For PTP / gPTP synchronization over ethernet on Linux)"                                << "\n";
      ss << R"(  #                           (device configuration in ecaltime.ini))"                                               << "\n";
      ss << R"(  rt: )"                                               << quoteString(config_.timesync.timesync_module_rt)           << "\n";
      ss << R"(  # Specify the module name for replaying)"                                                                          << "\n";
      ss << R"(  replay: )"                                           << quoteString(config_.timesync.timesync_module_replay)       << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"(# Service configuration)"                                                                                            << "\n";
      ss << R"(service:)"                                                                                                           << "\n";
      ss << R"(  # Support service protocol v0, eCAL 5.11 and older)"                                                               << "\n";
      ss << R"(  protocol_v0: )"                                      << config_.service.protocol_v0                                << "\n";
      ss << R"(  # Support service protocol v1, eCAL 5.12 and newer)"                                                               << "\n";
      ss << R"(  protocol_v1: )"                                      << config_.service.protocol_v1                                << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"(# eCAL Application Configuration)"                                                                                   << "\n";
      ss << R"(application:)"                                                                                                       << "\n";
      ss << R"(  # Configuration for eCAL Sys)"                                                                                     << "\n";
      ss << R"(  sys:)"                                                                                                             << "\n";
      ss << R"(    # Apps blacklist to be excluded when importing tasks from cloud)"                                                << "\n";
      ss << R"(    filter_excl: )"                                    << quoteString(config_.application.sys.filter_excl)           << "\n";
      ss << R"(  # Process specific configuration)"                                                                                 << "\n";
      ss << R"(  terminal:)"                                                                                                        << "\n";
      ss << R"(    # Linux only command for starting applications with an external terminal emulator. )"                            << "\n";
      ss << R"(    # e.g. /usr/bin/x-terminal-emulator -e)"                                                                         << "\n";
      ss << R"(    #      /usr/bin/gnome-terminal -x)"                                                                              << "\n";
      ss << R"(    #      /usr/bin/xterm -e)"                                                                                       << "\n";
      ss << R"(    # If empty, the command will be ignored.)"                                                                       << "\n";
      ss << R"(    emulator: )"                                       << quoteString(config_.application.startup.terminal_emulator) << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"(# Logging configuration)"                                                                                            << "\n";
      ss << R"(logging:)"                                                                                                           << "\n";
      ss << R"(  sinks:)"                                                                                                           << "\n";
      ss << R"(      # Console logging configuration)"                                                                              << "\n";
      ss << R"(    console:)"                                                                                                       << "\n";
      ss << R"(      # Enable console logging)"                                                                                     << "\n";
      ss << R"(      enable: )"                                       << config_.logging.sinks.console.enable                       << "\n";
      ss << R"(      # Log level for console output)"                                                                               << "\n";
      ss << R"(      level: )"                                        << logToArray(config_.logging.sinks.console.filter_log_con)   << "\n";
      ss << R"(    # File logging configuration)"                                                                                   << "\n";
      ss << R"(    file:)"                                                                                                          << "\n";
      ss << R"(      # Enable file logging)"                                                                                        << "\n";
      ss << R"(      enable: )"                                       << config_.logging.sinks.file.enable                          << "\n";
      ss << R"(      # Log level for file output)"                                                                                  << "\n";
      ss << R"(      level: )"                                        << logToArray(config_.logging.sinks.file.filter_log_file)     << "\n";
      ss << R"(      # Log file path)"                                                                                              << "\n";
      ss << R"(      path: )"                                         << quoteString(config_.logging.sinks.file.path)               << "\n";
      ss << R"(    # UDP logging configuration)"                                                                                    << "\n";
      ss << R"(    udp:)"                                                                                                           << "\n";
      ss << R"(      # Enable UDP logging)"                                                                                         << "\n";
      ss << R"(      enable: )"                                       << config_.logging.sinks.udp.enable                           << "\n";
      ss << R"(      # Log level for UDP output)"                                                                                   << "\n";
      ss << R"(      level:  )"                                       << logToArray(config_.logging.sinks.udp.filter_log_udp)       << "\n";
      ss << R"(      # UDP)"                                                                                                        << "\n";
      ss << R"(      port: )"                                         << config_.logging.sinks.udp.port                             << "\n";
      ss << R"()"                                                                                                                   << "\n";
    
      return ss;
    }

    bool dumpConfigToFile(const eCAL::Configuration& config_, const std::string& file_path_)
    {
      std::ofstream file(file_path_);
      if (!file.is_open())
      {
        return false;
      }

      file << getConfigAsYamlSS(config_).str();
      file.close();
      return true;
    }
  }
}