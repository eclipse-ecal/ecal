#include "default_configuration.h"

#include "ecal/config.h"

#include <string>

namespace 
{
  std::string quoteString(const std::string& str_) {
    return std::string("\"") + str_ + std::string("\"");
  }

  std::string logToArray(const eCAL::Logging::Filter& filter_)
  {
    std::string result = "[";    
    if ((filter_ & eCAL::Logging::log_level_info) != 0)    result += "\"info\", ";
    if ((filter_ & eCAL::Logging::log_level_warning) != 0) result += "\"warning\", ";
    if ((filter_ & eCAL::Logging::log_level_error) != 0)   result += "\"error\", ";
    if ((filter_ & eCAL::Logging::log_level_fatal) != 0)   result += "\"fatal\", ";
    if ((filter_ & eCAL::Logging::log_level_debug1) != 0)  result += "\"debug1\", ";
    if ((filter_ & eCAL::Logging::log_level_debug2) != 0)  result += "\"debug2\", ";
    if ((filter_ & eCAL::Logging::log_level_debug3) != 0)  result += "\"debug3\", ";
    if ((filter_ & eCAL::Logging::log_level_debug4) != 0)  result += "\"debug4\", ";
    
    if (result.size() == 1 && (filter_ & eCAL::Logging::log_level_all) != 0)
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
      case eCAL::TransportLayer::eType::shm:
          result += "\"shm\", ";
          break;
        case eCAL::TransportLayer::eType::udp_mc:
          result += "\"udp\", ";
          break;
        case eCAL::TransportLayer::eType::tcp:
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

  std::string quoteString(const eCAL::eCommunicationMode mode_)
  {
    switch (mode_)
    {
      case eCAL::eCommunicationMode::local:
        return "\"local\"";
        break;
      case eCAL::eCommunicationMode::network:
        return "\"network\"";
        break;
      
      default:
        return "";
        break;
    }
  }

  std::string quoteString(const eCAL::Registration::Local::eTransportType transport_type_)
  {
    switch (transport_type_)
    {
      case eCAL::Registration::Local::eTransportType::shm:
        return "\"shm\"";
        break;
      case eCAL::Registration::Local::eTransportType::udp:
        return "\"udp\"";
        break;
      
      default:
        return "";
        break;
    }
  }

  std::string quoteString(const eCAL::Registration::Network::eTransportType transport_type_)
  {
    switch (transport_type_)
    {
      case eCAL::Registration::Network::eTransportType::udp:
        return "\"udp\"";
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
      ss << R"(# Operation configuration)"                                                                                          << "\n";
      ss << R"(# eCAL components communication mode ("local" or "network"):)"                                                       << "\n";
      ss << R"(#   "local": local host only communication (default))"                                                               << "\n";
      ss << R"(#   "network": communication across network boundaries)"                                                             << "\n";
      ss << R"(communication_mode: )"                                << quoteString(config_.communication_mode)                     << "\n";
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
      ss << R"(  # SHM transport domain that enables interprocess mechanisms across (virtual))"                                     << "\n";
      ss << R"(  # host borders (e.g, Docker); by default equivalent to local host name)"                                           << "\n";
      ss << R"(  shm_transport_domain: )"                            << quoteString(config_.registration.shm_transport_domain)      << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"(  local:)"                                                                                                           << "\n";
      ss << R"(    # Specify the transport type for local registration)"                                                            << "\n";
      ss << R"(    #   "shm": shared memory based registration)"                                                                    << "\n";
      ss << R"(    #   "udp": udp based registration (default))"                                                                    << "\n";
      ss << R"(    transport_type: )"                                << quoteString(config_.registration.local.transport_type)      << "\n";
      ss << R"(    shm:)"                                                                                                           << "\n";
      ss << R"(      # Domain name for shared memory based registration)"                                                           << "\n";
      ss << R"(      domain:  )"                                     << quoteString(config_.registration.local.shm.domain)          << "\n";
      ss << R"(      # Queue size of registration events)"                                                                          << "\n";
      ss << R"(      queue_size: )"                                  << config_.registration.local.shm.queue_size                   << "\n";
      ss << R"(    udp:)"                                                                                                           << "\n";
      ss << R"(      # Specify port for local registration traffic)"                                                                << "\n";
      ss << R"(      port: )"                                        << config_.registration.local.udp.port                         << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"(  network:)"                                                                                                         << "\n";
      ss << R"(    # Specify the transport type for network registration)"                                                          << "\n";
      ss << R"(    #   "udp": udp based registration (default))"                                                                    << "\n";
      ss << R"(    transport_type: )"                                << quoteString(config_.registration.network.transport_type)    << "\n";
      ss << R"(    udp:)"                                                                                                           << "\n";
      ss << R"(    # Specify port for network registration traffic)"                                                                << "\n";
      ss << R"(      port: )"                                        << config_.registration.network.udp.port                       << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"()"                                                                                                                   << "\n";
      ss << R"(# Transport layer configuration)"                                                                                    << "\n";
      ss << R"(transport_layer:)"                                                                                                   << "\n";
      ss << R"(  udp:)"                                                                                                             << "\n";
      ss << R"(    # UDP configuration version (Since eCAL 5.12.))"                                                                 << "\n";
      ss << R"(    # v1: default behavior)"                                                                                         << "\n";
      ss << R"(    # v2: new behavior, comes with a bit more intuitive handling regarding masking of the groups)"                   << "\n";
      ss << R"(    config_version: )"                                << quoteString(config_.transport_layer.udp.config_version)     << "\n";
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
      ss << R"(    # Local mode multicast group and ttl)"                                                                           << "\n";
      ss << R"(    local:)"                                                                                                         << "\n";
      ss << R"(      # Multicast group base. All registration and logging is sent on this address)"                                 << "\n";
      ss << R"(      group: )"                                       << quoteString(config_.transport_layer.udp.local.group)        << "\n";
      ss << R"(      # TTL (hop limit) is used to determine the amount of routers being traversed towards the destination)"         << "\n";
      ss << R"(      ttl: )"                                         << config_.transport_layer.udp.local.ttl                       << "\n";
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
      ss << R"(      # Default memory file size for new publisher)"                                                                 << "\n";
      ss << R"(      memfile_min_size_bytes: )"                      << config_.publisher.layer.shm.memfile_min_size_bytes          << "\n";
      ss << R"(      # Dynamic file size reserve before recreating memory file if topic size changes)"                              << "\n";
      ss << R"(      memfile_reserve_percent: )"                     << config_.publisher.layer.shm.memfile_reserve_percent         << "\n";
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
      ss << R"(  # Configuration for file, udp and console log provider)"                                                           << "\n";
      ss << R"(  # Possible levels are: "info", "warning", "error", "fatal", "debug1", "debug2", "debug3", "debug4")"               << "\n";
      ss << R"(  # Can be combined like)"                                                                                           << "\n";      
      ss << R"(  # log_level: ["warning", "fatal", "debug1", ...])"                                                                 << "\n";
      ss << R"(  provider:)"                                                                                                        << "\n";
      ss << R"(    # Console logging configuration)"                                                                                << "\n";
      ss << R"(    console:)"                                                                                                       << "\n";
      ss << R"(      # Enable console logging)"                                                                                     << "\n";
      ss << R"(      enable: )"                                       << config_.logging.provider.console.enable                    << "\n";
      ss << R"(      # Log level for console output)"                                                                               << "\n";
      ss << R"(      log_level: )"                                    << logToArray(config_.logging.provider.console.log_level)     << "\n";
      ss << R"(    # File logging configuration)"                                                                                   << "\n";
      ss << R"(    file:)"                                                                                                          << "\n";
      ss << R"(      # Enable file logging)"                                                                                        << "\n";
      ss << R"(      enable: )"                                       << config_.logging.provider.file.enable                       << "\n";
      ss << R"(      # Log level for file output)"                                                                                  << "\n";
      ss << R"(      log_level: )"                                    << logToArray(config_.logging.provider.file.log_level)        << "\n";
      ss << R"(    # UDP logging configuration)"                                                                                    << "\n";
      ss << R"(    udp:)"                                                                                                           << "\n";
      ss << R"(      # Enable UDP logging)"                                                                                         << "\n";
      ss << R"(      enable: )"                                       << config_.logging.provider.udp.enable                        << "\n";
      ss << R"(      # Log level for UDP output)"                                                                                   << "\n";
      ss << R"(      log_level: )"                                    << logToArray(config_.logging.provider.udp.log_level)         << "\n";
      ss << R"(    # File specific configuration)"                                                                                  << "\n";
      ss << R"(    file_config: )"                                                                                                  << "\n";
      ss << R"(      # Log file path)"                                                                                              << "\n";
      ss << R"(      path: )"                                         << quoteString(config_.logging.provider.file_config.path)     << "\n";
      ss << R"(    # UDP specific configuration)"                                                                                   << "\n";
      ss << R"(    udp_config:)"                                                                                                    << "\n";
      ss << R"(      # UDP Port for sending logging data)"                                                                          << "\n";
      ss << R"(      port: )"                                         << config_.logging.provider.udp_config.port                   << "\n";
      ss << R"(  # Configuration for udp log receiver)"                                                                             << "\n";
      ss << R"(  receiver:)"                                                                                                        << "\n";
      ss << R"(    # Enable log receiving (UDP only))"                                                                              << "\n";
      ss << R"(    enable: )"                                         << config_.logging.receiver.enable                            << "\n";
      ss << R"(    # UDP specific configuration)"                                                                                   << "\n";
      ss << R"(    udp_config:)"                                                                                                    << "\n";
      ss << R"(      # UDP Port for sending logging data)"                                                                          << "\n";
      ss << R"(      port: )"                                         << config_.logging.receiver.udp_config.port                   << "\n";
      ss << R"()"                                                                                                                   << "\n";
    
      return ss;
    }

    std::stringstream getTimeConfigAsYamlSS()
    {
      std::stringstream ss;
      ss << std::boolalpha;
      ss << R"(# ---------------------------------------------)"                                                                                    << "\n";
      ss << R"(# ecaltime-linuxptp Settings)"                                                                                                       << "\n";
      ss << R"(# ---------------------------------------------)"                                                                                    << "\n";
      ss << R"(#)"                                                                                                                                  << "\n";
      ss << R"(# device = /dev/ptp0                                 The device can be any ptp clock.)"                                              << "\n";
      ss << R"(#                                                    Alternatively, you can use:)"                                                   << "\n";
      ss << R"(#                                                      - CLOCK_MONOTONIC    (a steady clock with undefined epoche))"                 << "\n";
      ss << R"(#                                                      - CLOCK_REALTIME     (the current system time))"                              << "\n";
      ss << R"(#                                                      - CLOCK_TAI          (Like CLOCK_REALTIME but in International Atomic Time))" << "\n";
      ss << R"(#)"                                                                                                                                  << "\n";
      ss << R"(# ---------------------------------------------)"                                                                                    << "\n";
      ss << R"(linuxptp:)"                                                                                                                          << "\n";
      ss << R"(  device: "/dev/ptp0")"                                                                                                              << "\n";

      return ss;
    }
  }
}