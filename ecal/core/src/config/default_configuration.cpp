#include "default_configuration.h"

#include <string>

#include "ecal_def.h"

namespace {
  // Overload the << operator for std::array
  template<typename T, std::size_t N>
  std::ostream& operator<<(std::ostream& os, const std::array<T, N>& arr) {
      os << "[";
      for (std::size_t i = 0; i < N; ++i) {
          os << arr[i];
          if (i < N - 1) {
              os << ", ";
          }
      }
      os << "]";
      return os;
  }

  std::string quoteString(const char* str_) {
    return std::string("\"") + std::string(str_) + std::string("\"");
  }

  std::string quoteString(eCAL::Types::UdpConfigVersion config_version_) {
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

  std::string quoteString(eCAL::Types::UDPMode mode_)
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
}

namespace eCAL
{
  namespace Config
  {
    std::stringstream getConfigAsYamlSS()
    {
      std::stringstream ss;
      ss << std::boolalpha;
      ss << R"(#  _____     _ _                                ____    _    _                             )"                    << "\n";
      ss << R"(# | ____|___| (_)_ __  ___  ___            ___ / ___|  / \  | |                            )"                    << "\n";
      ss << R"(# |  _| / __| | | '_ \/ __|/ _ \  _____   / _ \ |     / _ \ | |                            )"                    << "\n";
      ss << R"(# | |__| (__| | | |_) \__ \  __/ |_____| |  __/ |___ / ___ \| |___                         )"                    << "\n";
      ss << R"(# |_____\___|_|_| .__/|___/\___|          \___|\____/_/   \_\_____|                        )"                    << "\n";
      ss << R"(#               |_|                                                                            )"                << "\n";
      ss << R"(#        _       _           _                    __ _                       _   _             )"                << "\n";
      ss << R"(#   __ _| | ___ | |__   __ _| |   ___ ___  _ __  / _(_) __ _ _   _ _ __ __ _| |_(_) ___  _ __  )"                << "\n";
      ss << R"(#  / _` | |/ _ \| '_ \ / _` | |  / __/ _ \| '_ \| |_| |/ _` | | | | '__/ _` | __| |/ _ \| '_ \ )"                << "\n";
      ss << R"(# | (_| | | (_) | |_) | (_| | | | (_| (_) | | | |  _| | (_| | |_| | | | (_| | |_| | (_) | | | |)"                << "\n";
      ss << R"(#  \__, |_|\___/|_.__/ \__,_|_|  \___\___/|_| |_|_| |_|\__, |\__,_|_|  \__,_|\__|_|\___/|_| |_|)"                << "\n";
      ss << R"(#  |___/                                               |___/                                   )"                << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"(# Registration layer configuration)"                                                                             << "\n";
      ss << R"(registration:)"                                                                                                  << "\n";
      ss << R"(  # Topic registration refresh cylce (has to be smaller then registration timeout! Default: 1000))"              << "\n";
      ss << R"(  registration_refresh: )"                            << REGISTRATION_REGISTRATION_REFRESH                       << "\n";
      ss << R"(  # Timeout for topic registration in ms (internal, Default: 60000)"                                             << "\n";
      ss << R"(  registration_timeout: )"                            << REGISTRATION_REGISTRATION_TIMEOUT                       << "\n";
      ss << R"(  # Enable to receive registration information on the same local machine)"                                       << "\n";
      ss << R"(  loopback: )"                                        << REGISTRATION_LOOPBACK                                   << "\n";
      ss << R"(  # Host group name that enables interprocess mechanisms across (virtual))"                                      << "\n";
      ss << R"(  # host borders (e.g, Docker); by default equivalent to local host name)"                                       << "\n";
      ss << R"(  host_group_name: )"                                 << quoteString(REGISTRATION_HOST_GROUP_NAME)               << "\n";
      ss << R"(  # true  = all eCAL components communicate over network boundaries)"                                            << "\n";
      ss << R"(  # false = local host only communication (Default: false))"                                                     << "\n";
      ss << R"(  network_enabled: )"                                 << REGISTRATION_NETWORK_ENABLED                            << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"(  layer:)"                                                                                                       << "\n";
      ss << R"(    shm:)"                                                                                                       << "\n";
      ss << R"(      enable: )"                                      << REGISTRATION_LAYER_SHM_ENABLE                           << "\n";
      ss << R"(      # Domain name for shared memory based registration)"                                                       << "\n";
      ss << R"(      domain:  )"                                     << quoteString(REGISTRATION_LAYER_SHM_DOMAIN)              << "\n";
      ss << R"(      # Queue size of registration events)"                                                                      << "\n";
      ss << R"(      queue_size: )"                                  << REGISTRATION_LAYER_SHM_QUEUE_SIZE                       << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"(    udp:)"                                                                                                       << "\n";
      ss << R"(      enable: )"                                      << REGISTRATION_LAYER_UDP_ENABLE                           << "\n";
      ss << R"(      port: )"                                        << REGISTRATION_LAYER_UDP_PORT                             << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"(# Monitoring configuration)"                                                                                     << "\n";
      ss << R"(monitoring:)"                                                                                                    << "\n";
      ss << R"(  # Timeout for topic monitoring in ms (Default: 5000), increase in 1000er steps)"                               << "\n";
      ss << R"(  timeout: )"                                         << MONITORING_TIMEOUT                                      << "\n";
      ss << R"(  # Topics blacklist as regular expression (will not be monitored)"                                              << "\n";
      ss << R"(  filter_excl: )"                                     << quoteString(MONITORING_FILTER_EXCL)                     << "\n";
      ss << R"(  # Topics whitelist as regular expression (will be monitored only) (Default: "")"                               << "\n";
      ss << R"(  filter_incl: )"                                     << quoteString(MONITORING_FILTER_INCL)                     << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"(# Transport layer configuration)"                                                                                << "\n";
      ss << R"(transport_layer:)"                                                                                               << "\n";
      ss << R"(  udp:)"                                                                                                         << "\n";
      ss << R"(    # UDP configuration version (Since eCAL 5.12.))"                                                             << "\n";
      ss << R"(    # v1: default behavior)"                                                                                     << "\n";
      ss << R"(    # v2: new behavior, comes with a bit more intuitive handling regarding masking of the groups)"               << "\n";
      ss << R"(    config_version: )"                                << quoteString(TRANSPORT_LAYER_UDP_CONFIG_VERSION)         << "\n";
      ss << R"(    # Valid modes: local, network (Default: local)"                                                              << "\n";
      ss << R"(    mode: )"                                          << quoteString(TRANSPORT_LAYER_UDP_MODE)                   << "\n";
      ss << R"(    # Multicast port number)"                                                                                    << "\n";
      ss << R"(    port: )"                                          << TRANSPORT_LAYER_UDP_PORT                                << "\n";
      ss << R"(    # v1: Mask maximum number of dynamic multicast group (range 0.0.0.1-0.0.0.255))"                             << "\n";
      ss << R"(    # v2: Masks are now considered like routes masking (range 255.0.0.0-255.255.255.255))"                       << "\n";
      ss << R"(    mask: )"                                          << TRANSPORT_LAYER_UDP_MASK                                << "\n";
      ss << R"(    # Send buffer in bytes)"                                                                                     << "\n";
      ss << R"(    send_buffer: )"                                   << TRANSPORT_LAYER_UDP_SEND_BUFFER                         << "\n";
      ss << R"(    # Receive buffer in bytes)"                                                                                  << "\n";
      ss << R"(    receive_buffer: )"                                << TRANSPORT_LAYER_UDP_RECEIVE_BUFFER                      << "\n";
      ss << R"(    # Linux specific setting to join all network interfaces independend of their link state.)"                   << "\n";
      ss << R"(    # Enabling ensures that eCAL processes receive data when they are started before the)"                       << "\n";
      ss << R"(    # network devices are up and running.)"                                                                      << "\n";
      ss << R"(    join_all_interfaces: )"                           << TRANSPORT_LAYER_UDP_JOIN_ALL_INTERFACES                 << "\n";
      ss << R"(    # Windows specific setting to enable receiving UDP traffic with the Npcap based receiver)"                   << "\n";
      ss << R"(    npcap_enabled: )"                                 << TRANSPORT_LAYER_UDP_NPCAP_ENABLED                       << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"(    # In local mode multicast group and ttl are set by default and are not adjustable)"                          << "\n";
      ss << R"(    local:)"                                                                                                     << "\n";
      ss << R"(      # Multicast group base. All registration and logging is sent on this address)"                             << "\n";
      ss << R"(      # group: "127.0.0.1")"                                                                                     << "\n";
      ss << R"(      # TTL (hop limit) is used to determine the amount of routers being traversed towards the destination)"     << "\n";
      ss << R"(      # ttl: 0)"                                                                                                 << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"(    network:)"                                                                                                   << "\n";
      ss << R"(      # Multicast group base. All registration and logging is sent on this address)"                             << "\n";
      ss << R"(      group: )"                                       << quoteString(TRANSPORT_LAYER_UDP_NETWORK_GROUP)          << "\n";
      ss << R"(      # TTL (hop limit) is used to determine the amount of routers being traversed towards the destination)"     << "\n";
      ss << R"(      ttl: )"                                         << TRANSPORT_LAYER_UDP_NETWORK_TTL                         << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"(  tcp: )"                                                                                                        << "\n";
      ss << R"(    # Reader amount of threads that shall execute workload)"                                                     << "\n";
      ss << R"(    number_executor_reader: )"                        << TRANSPORT_LAYER_TCP_NUMBER_EXECUTOR_READER              << "\n";
      ss << R"(    # Writer amount of threads that shall execute workload)"                                                     << "\n";
      ss << R"(    number_executor_writer: )"                        << TRANSPORT_LAYER_TCP_NUMBER_EXECUTOR_WRITER              << "\n";
      ss << R"(    # Reconnection attemps the session will try to reconnect in case of an issue)"                               << "\n";
      ss << R"(    max_reconnections: )"                             << TRANSPORT_LAYER_TCP_MAX_RECONNECTIONS                   << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"(  shm:)"                                                                                                         << "\n";
      ss << R"(    # Default memory file size for new publisher)"                                                               << "\n";
      ss << R"(    memfile_min_size_bytes: )"                        << TRANSPORT_LAYER_SHM_MEMFILE_MIN_SIZE_BYTES              << "\n";
      ss << R"(    # Dynamic file size reserve before recreating memory file if topic size changes)"                            << "\n";
      ss << R"(    memfile_reserve_percent: )"                       << TRANSPORT_LAYER_SHM_MEMFILE_RESERVE_PERCENT             << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"(# Publisher specific base settings)"                                                                             << "\n";
      ss << R"(publisher:)"                                                                                                     << "\n";
      ss << R"(  layer:)"                                                                                                       << "\n";
      ss << R"(    # Base configuration for shared memory publisher)"                                                           << "\n";
      ss << R"(    shm:)"                                                                                                       << "\n";
      ss << R"(      # Enable layer)"                                                                                           << "\n";
      ss << R"(      enable: )"                                      << PUBLISHER_LAYER_SHM_ENABLE                              << "\n";
      ss << R"(      # Enable zero copy shared memory transport mode)"                                                          << "\n";
      ss << R"(      zero_copy_mode: )"                              << PUBLISHER_LAYER_SHM_ZERO_COPY_MODE                      << "\n";
      ss << R"(      # Force connected subscribers to send acknowledge event after processing the message.)"                    << "\n";
      ss << R"(      # The publisher send call is blocked on this event with this timeout (0 == no handshake).)"                << "\n";
      ss << R"(      acknowledge_timeout_ms: )"                      << PUBLISHER_LAYER_SHM_ACKNOWLEDGE_TIMEOUT_MS              << "\n";
      ss << R"(      # Maximum number of used buffers (needs to be greater than 1, default = 1)"                                << "\n";
      ss << R"(      memfile_buffer_count: )"                        << PUBLISHER_LAYER_SHM_MEMFILE_BUFFER_COUNT                << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"(    # Base configuration for UDP publisher)"                                                                     << "\n";
      ss << R"(    udp:)"                                                                                                       << "\n";
      ss << R"(      # Enable layer)"                                                                                           << "\n";
      ss << R"(      enable: )"                                      << PUBLISHER_LAYER_UDP_ENABLE                              << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"(    # Base configuration for TCP publisher)"                                                                     << "\n";
      ss << R"(    tcp:)"                                                                                                       << "\n";
      ss << R"(      # Enable layer)"                                                                                           << "\n";
      ss << R"(      enable: )"                                      << PUBLISHER_LAYER_TCP_ENABLE                              << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"(  # Share topic type via registration)"                                                                          << "\n";
      ss << R"(  share_topic_type: )"                                << PUBLISHER_SHARE_TOPIC_TYPE                              << "\n";
      ss << R"(  # Share topic description via registration)"                                                                   << "\n";
      ss << R"(  share_topic_description: )"                         << PUBLISHER_SHARE_TOPIC_DESCRIPTION                       << "\n";
      ss << R"(  # Priority list for layer usage in local mode (Default: SHM > UDP > TCP)"                                      << "\n";
      ss << R"(  priority_local: )"                                  << PUBLISHER_PRIORITY_LOCAL                                << "\n";
      ss << R"(  # Priority list for layer usage in cloud mode (Default: UDP > TCP)"                                            << "\n";
      ss << R"(  priority_network: )"                                << PUBLISHER_PRIORITY_NETWORK                              << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"(# Subscriber specific base configuration)"                                                                       << "\n";
      ss << R"(subscriber:)"                                                                                                    << "\n";
      ss << R"(  layer:)"                                                                                                       << "\n";
      ss << R"(    # Base configuration for shared memory subscriber)"                                                          << "\n";
      ss << R"(    shm:)"                                                                                                       << "\n";
      ss << R"(      # Enable layer)"                                                                                           << "\n";
      ss << R"(      enable: )"                                        << SUBSCRIBER_LAYER_SHM_ENABLE                           << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"(    # Base configuration for UDP subscriber)"                                                                    << "\n";
      ss << R"(    udp:)"                                                                                                       << "\n";
      ss << R"(      # Enabler layer)"                                                                                          << "\n";
      ss << R"(      enable: )"                                        << SUBSCRIBER_LAYER_UDP_ENABLE                           << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"(    # Base configuration for TCP subscriber)"                                                                    << "\n";
      ss << R"(    tcp:)"                                                                                                       << "\n";
      ss << R"(      # Enable layer)"                                                                                           << "\n";
      ss << R"(      enable: )"                                        << SUBSCRIBER_LAYER_TCP_ENABLE                           << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"(  # Enable dropping of payload messages that arrive out of order)"                                               << "\n";
      ss << R"(  drop_out_of_order_messages: )"                        << SUBSCRIBER_DROP_OUT_OF_ORDER_MESSAGES                 << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"(# Time configuration)"                                                                                           << "\n";
      ss << R"(time:)"                                                                                                          << "\n";
      ss << R"(  # Time synchronisation interface name (dynamic library)"                                                       << "\n";
      ss << R"(  # The name will be extended with platform suffix (32|64), debug suffix (d) and platform extension (.dll|.so))" << "\n";
      ss << R"(  # Available modules are:)"                                                                                     << "\n";
      ss << R"(  #   - ecaltime-localtime    local system time without synchronization)"                                        << "\n";
      ss << R"(  #   - ecaltime-linuxptp     For PTP / gPTP synchronization over ethernet on Linux)"                            << "\n";
      ss << R"(  #                           (device configuration in ecaltime.ini)"                                            << "\n";
      ss << R"(  rt: )"                                               << quoteString(TIME_RT)                                   << "\n";
      ss << R"(  # Specify the module name for replaying)"                                                                      << "\n";
      ss << R"(  replay: )"                                           << quoteString(TIME_REPLAY)                               << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"(# Service configuration)"                                                                                        << "\n";
      ss << R"(service:)"                                                                                                       << "\n";
      ss << R"(  # Support service protocol v0, eCAL 5.11 and older)"                                                           << "\n";
      ss << R"(  protocol_v0: )"                                      << SERVICE_PROTOCOL_V0                                    << "\n";
      ss << R"(  # Support service protocol v1, eCAL 5.12 and newer)"                                                           << "\n";
      ss << R"(  protocol_v1: )"                                      << SERVICE_PROTOCOL_V1                                    << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"(application:)"                                                                                                   << "\n";
      ss << R"(  # Configuration for eCAL Sys)"                                                                                 << "\n";
      ss << R"(  sys:)"                                                                                                         << "\n";
      ss << R"(    # Apps blacklist to be excluded when importing tasks from cloud)"                                            << "\n";
      ss << R"(    filter_excl: )"                                    << quoteString(APPLICATION_SYS_FILTER_EXCL)               << "\n";
      ss << R"(  # Process specific configuration)"                                                                             << "\n";
      ss << R"(  terminal:)"                                                                                                    << "\n";
      ss << R"(    # Linux only command for starting applications with an external terminal emulator. )"                        << "\n";
      ss << R"(    # e.g. /usr/bin/x-terminal-emulator -e)"                                                                     << "\n";
      ss << R"(    #      /usr/bin/gnome-terminal -x)"                                                                          << "\n";
      ss << R"(    #      /usr/bin/xterm -e)"                                                                                   << "\n";
      ss << R"(    # If empty, the command will be ignored.)"                                                                   << "\n";
      ss << R"(    emulator: )"                                       << quoteString(APPLICATION_TERMINAL_EMULATOR)             << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"()"                                                                                                               << "\n";
      ss << R"(# Logging configuration)"                                                                                        << "\n";
      ss << R"(logging:)"                                                                                                       << "\n";
      ss << R"(  sinks:)"                                                                                                       << "\n";
      ss << R"(      # Console logging configuration)"                                                                          << "\n";
      ss << R"(    console:)"                                                                                                   << "\n";
      ss << R"(      # Enable console logging)"                                                                                 << "\n";
      ss << R"(      enable: )"                                       << LOGGING_SINKS_CONSOLE_ENABLE                           << "\n";
      ss << R"(      # Log level for console output)"                                                                           << "\n";
      ss << R"(      level: )"                                        << LOGGING_SINKS_CONSOLE_LEVEL                            << "\n";
      ss << R"(    # File logging configuration)"                                                                               << "\n";
      ss << R"(    file:)"                                                                                                      << "\n";
      ss << R"(      # Enable file logging)"                                                                                    << "\n";
      ss << R"(      enable: )"                                       << LOGGING_SINKS_FILE_ENABLE                              << "\n";
      ss << R"(      # Log level for file output)"                                                                              << "\n";
      ss << R"(      level: )"                                        << LOGGING_SINKS_FILE_LEVEL                               << "\n";
      ss << R"(      # Log file path)"                                                                                          << "\n";
      ss << R"(      path: )"                                         << quoteString(LOGGING_SINKS_FILE_PATH)                   << "\n";
      ss << R"(    # UDP logging configuration)"                                                                                << "\n";
      ss << R"(    udp:)"                                                                                                       << "\n";
      ss << R"(      # Enable UDP logging)"                                                                                     << "\n";
      ss << R"(      enable: )"                                       << LOGGING_SINKS_UDP_ENABLE                               << "\n";
      ss << R"(      # Log level for UDP output)"                                                                               << "\n";
      ss << R"(      level:  )"                                       << LOGGING_SINKS_UDP_LEVEL                                << "\n";
      ss << R"(      # UDP)"                                                                                                    << "\n";
      ss << R"(      port: )"                                         << LOGGING_SINKS_UDP_PORT                                 << "\n";
      ss << R"()"                                                                                                               << "\n";
    
      return ss;
    }
  }
}