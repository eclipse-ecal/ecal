#pragma once

#include <string>

std::string default_config = R"(#  _____     _ _                                ____    _    _                             
# | ____|___| (_)_ __  ___  ___            ___ / ___|  / \  | |                            
# |  _| / __| | | '_ \/ __|/ _ \  _____   / _ \ |     / _ \ | |                            
# | |__| (__| | | |_) \__ \  __/ |_____| |  __/ |___ / ___ \| |___                         
# |_____\___|_|_| .__/|___/\___|          \___|\____/_/   \_\_____|                        
#               |_|                                                                            
#        _       _           _                    __ _                       _   _             
#   __ _| | ___ | |__   __ _| |   ___ ___  _ __  / _(_) __ _ _   _ _ __ __ _| |_(_) ___  _ __  
#  / _` | |/ _ \| '_ \ / _` | |  / __/ _ \| '_ \| |_| |/ _` | | | | '__/ _` | __| |/ _ \| '_ \ 
# | (_| | | (_) | |_) | (_| | | | (_| (_) | | | |  _| | (_| | |_| | | | (_| | |_| | (_) | | | |
#  \__, |_|\___/|_.__/ \__,_|_|  \___\___/|_| |_|_| |_|\__, |\__,_|_|  \__,_|\__|_|\___/|_| |_|
#  |___/                                               |___/                                   


# Registration layer configuration
registration:
  # Topic registration refresh cylce (has to be smaller then registration timeout! Default: 1000)
  registration_refresh: 1000   
  # Timeout for topic registration in ms (internal, Default: 60000)
  registration_timeout: 60000
  # Enable to receive registration information on the same local machine
  loopback: true
  # Host group name that enables interprocess mechanisms across (virtual)
  # host borders (e.g, Docker); by default equivalent to local host name
  host_group_name: ""
  # true  = all eCAL components communicate over network boundaries
  # false = local host only communication (Default: false)
  network_enabled: false

  layer:
    shm:
      enable: false
      # Domain name for shared memory based registration
      domain: "ecal_mon"
      # Queue size of registration events
      queue_size: 1024

    udp:
      enable: true
      port: 14000
  

# Monitoring configuration
monitoring:
  # Timeout for topic monitoring in ms (Default: 1000), increase in 1000er steps
  timeout: 1000
  # Topics blacklist as regular expression (will not be monitored)
  filter_excl: "^__.*$"
  # Topics whitelist as regular expression (will be monitored only) (Default: "")
  filter_incl: ""
  

# Transport layer configuration
transport_layer:
  udp:
    # UDP configuration version (Since eCAL 5.12.)
    # v1: default behavior
    # v2: new behavior, comes with a bit more intuitive handling regarding masking of the groups
    config_version: "v2"
    # Valid modes: local, network (Default: local)
    mode: "local"
    # Multicast port number
    port: 14002
    # v1: Mask maximum number of dynamic multicast group (range 0.0.0.1-0.0.0.255)
    # v2: Masks are now considered like routes masking (range 255.0.0.0-255.255.255.255)
    mask: "255.255.255.240"
    # Send buffer in bytes
    send_buffer: 5242880
    # Receive buffer in bytes
    receive_buffer: 5242880
    # Linux specific setting to join all network interfaces independend of their link state.
    # Enabling ensures that eCAL processes receive data when they are started before the
    # network devices are up and running.
    join_all_interfaces: false
    # Windows specific setting to enable receiving UDP traffic with the Npcap based receiver
    npcap_enabled: false 
    
    # In local mode multicast group and ttl are set by default and are not adjustable
    local:
      # Multicast group base. All registration and logging is sent on this address 
      # group: "127.0.0.1" 
      # TTL (hop limit) is used to determine the amount of routers being traversed towards the destination
      # ttl: 0

    network:
      # Multicast group base. All registration and logging is sent on this address 
      group: "239.0.0.1" 
      # TTL (hop limit) is used to determine the amount of routers being traversed towards the destination
      ttl: 3   
    
  tcp: 
    # Reader amount of threads that shall execute workload
    number_executor_reader: 4
    # Writer amount of threads that shall execute workload
    number_executor_writer: 4
    # Reconnection attemps the session will try to reconnect in case of an issue
    max_reconnections: 5

  shm:     
    # Default memory file size for new publisher
    memfile_min_size_bytes: 4096
    # Dynamic file size reserve before recreating memory file if topic size changes
    memfile_reserve_percent: 50
    

# Publisher specific base settings
publisher:
  # Base configuration for shared memory publisher
  shm:
    # Enable layer
    enable: true
    # Enable zero copy shared memory transport mode
    zero_copy_mode: false
    # Force connected subscribers to send acknowledge event after processing the message.
    # The publisher send call is blocked on this event with this timeout (0 == no handshake).
    acknowledge_timeout_ms: 0
    # Maximum number of used buffers (needs to be greater than 1, default = 1)
    memfile_buffer_count: 1
  
  # Base configuration for UDP publisher
  udp:
    # Enable layer
    enable: true
       
  # Base configuration for TCP publisher
  tcp:
    # Enable layer
    enable: true 

  # Share topic type via registration
  share_topic_type: true
  # Share topic description via registration
  share_topic_description: true
  # Priority list for layer usage in local mode (Default: SHM > UDP > TCP)
  priority_local: ["shm", "udp", "tcp"]
  # Priority list for layer usage in cloud mode (Default: UDP > TCP)
  priority_network: ["udp", "tcp"]


# Subscriber specific base configuration
subscriber:
  # Base configuration for shared memory subscriber
  shm:
    # Enable layer
    enable: true

  # Base configuration for UDP subscriber
  udp:
    # Enabler layer
    enable: true
  
  # Base configuration for TCP subscriber
  tcp:
    # Enable layer
    enable: false 
  
  # Enable dropping of payload messages that arrive out of order
  drop_out_of_order_messages: true
  

# Time configuration
time:
  # Time synchronisation interface name (dynamic library)
  # The name will be extended with platform suffix (32|64), debug suffix (d) and platform extension (.dll|.so)
  # Available modules are:
  #   - ecaltime-localtime    local system time without synchronization        
  #   - ecaltime-linuxptp     For PTP / gPTP synchronization over ethernet on Linux
  #                           (device configuration in ecaltime.ini)
  rt: "ecaltime-localtime"
  # Specify the module name for replaying
  replay: ""


# Service configuration
service:
  # Support service protocol v0, eCAL 5.11 and older (0 = off, 1 = on)
  protocol_v0: true
  # Support service protocol v1, eCAL 5.12 and newer (0 = off, 1 = on)
  protocol_v1: false


# eCAL Application Configuration
application:
  # Configuration for eCAL Sys
  sys:
    # Apps blacklist to be excluded when importing tasks from cloud
    filter_excl: "^eCALSysClient$|^eCALSysGUI$|^eCALSys$*"
  # Process specific configuration
  terminal:
    # Linux only command for starting applications with an external terminal emulator. 
    # e.g. /usr/bin/x-terminal-emulator -e
    #      /usr/bin/gnome-terminal -x
    #      /usr/bin/xterm -e
    # If empty, the command will be ignored.
    emulator: ""


# Logging configuration
logging:
  sinks:
    # Console logging configuration
    console:
      # Enable console logging
      enable: false
      # Log level for console output
      level: ["info", "warning", "error", "fatal"]
    # File logging configuration
    file:
      # Enable file logging
      enable: false
      # Log level for file output
      level: []
      # Log file path
      path: "ecal.log"
    # UDP logging configuration
    udp:
      # Enable UDP logging
      enable: true
      # Log level for UDP output
      level:  ["info", "warning", "error", "fatal"]
      # UDP
      port: 14001

)";