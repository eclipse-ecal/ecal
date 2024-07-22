#include <string>

static const std::string ini_file_as_string_deprecated = 
"; --------------------------------------------------\n"
"; NETWORK SETTINGS\n"
"; --------------------------------------------------\n"
"; network_enabled                  = true / false                  true  = all eCAL components communicate over network boundaries\n"
";                                                                  false = local host only communication\n"
";\n"
"; multicast_config_version         = v1 / v2                       UDP configuration version (Since eCAL 5.12.)\n"
";                                                                    v1: default behavior\n"
";                                                                    v2: new behavior, comes with a bit more intuitive handling regarding masking of the groups\n"
"; multicast_group                  = 239.0.0.1                     UDP multicast group base\n"
";                                                                  All registration and logging is sent on this address\n"
"; multicast_mask                   = 0.0.0.1-0.0.0.255             v1: Mask maximum number of dynamic multicast group\n"
";                                    255.0.0.0-255.255.255.255     v2: masks are now considered like routes masking\n"
";\n"
"; multicast_port                   = 14000 + x                     UDP multicast port number (eCAL will use at least the 2 following port\n"
";                                                                    numbers too, so please modify in steps of 10 (e.g. 1010, 1020 ...)\n"
";\n"
"; multicast_ttl                    = 0 + x                         UDP ttl value, also known as hop limit, is used in determining \n"
";                                                                    the intermediate routers being traversed towards the destination\n"
";\n"
"; multicast_sndbuf                 = 1024 * x                      UDP send buffer in bytes\n"
";  \n"
"; multicast_rcvbuf                 = 1024 * x                      UDP receive buffer in bytes\n"
";\n"
"; multicast_join_all_if            = false                         Linux specific setting to enable joining multicast groups on all network interfacs\n"
";                                                                    independent of their link state. Enabling this makes sure that eCAL processes\n"
";                                                                    receive data if they are started before network devices are up and running.\n"
";  \n"
"; shm_rec_enabled                  = true                          Enable to receive on eCAL shared memory layer\n"
"; tcp_rec_enabled                  = true                          Enable to receive on eCAL tcp layer\n"
"; udp_mc_rec_enabled               = true                          Enable to receive on eCAL udp multicast layer\n"
";\n"
"; npcap_enabled                    = false                         Enable to receive UDP traffic with the Npcap based receiver\n"
";\n"
"; tcp_pubsub_num_executor_reader   = 4                             Tcp_pubsub reader amount of threads that shall execute workload\n"
"; tcp_pubsub_num_executor_writer   = 4                             Tcp_pubsub writer amount of threads that shall execute workload\n"
"; tcp_pubsub_max_reconnections     = 5                             Tcp_pubsub reconnection attemps the session will try to reconnect in \n"
";                                                                    case of an issue (a negative value means infinite reconnection attemps)\n"
";\n"
"; host_group_name                  =                               Common host group name that enables interprocess mechanisms across \n"
";                                                                    (virtual) host borders (e.g, Docker); by default equivalent to local host name\n"
"; --------------------------------------------------\n"
"\n"
"[network]\n"
"network_enabled                    = false\n"
"multicast_config_version           = v1\n"
"multicast_group                    = 239.0.0.1\n"
"multicast_mask                     = 0.0.0.15\n"
"multicast_port                     = 14000\n"
"multicast_ttl                      = 2\n"
"multicast_sndbuf                   = 5242880\n"
"multicast_rcvbuf                   = 5242880\n"
"\n"
"multicast_join_all_if              = false\n"
"\n"
"shm_rec_enabled                    = true\n"
"tcp_rec_enabled                    = true\n"
"udp_mc_rec_enabled                 = true\n"
"\n"
"npcap_enabled                      = false\n"
"\n"
"tcp_pubsub_num_executor_reader     = 4\n"
"tcp_pubsub_num_executor_writer     = 4\n"
"tcp_pubsub_max_reconnections       = 5\n"
"\n"
"host_group_name                    =\n"
"\n"
"; --------------------------------------------------\n"
"; COMMON SETTINGS\n"
"; --------------------------------------------------\n"
"; registration_timeout             = 60000                         Timeout for topic registration in ms (internal)\n"
"; registration_refresh             = 1000                          Topic registration refresh cylce (has to be smaller then registration timeout !)\n"
"\n"
"; --------------------------------------------------\n"
"[common]\n"
"registration_timeout               = 60000\n"
"registration_refresh               = 1000\n"
"\n"
"; --------------------------------------------------\n"
"; TIME SETTINGS\n"
"; --------------------------------------------------\n"
"; timesync_module_rt               = ecaltime-localtime            Time synchronisation interface name (dynamic library)\n"
";                                                                  The name will be extended with platform suffix (32|64), debug suffix (d) and platform extension (.dll|.so)\n"
";\n"
";                                                                  Available modules are:\n"
";                                                                    - ecaltime-localtime    local system time without synchronization        \n"
";                                                                    - ecaltime-linuxptp     For PTP / gPTP synchronization over ethernet on Linux\n"
";                                                                                            (device configuration in ecaltime.ini)\n"
"; --------------------------------------------------\n"
"[time]\n"
"timesync_module_rt                 = ecaltime-localtime\n"
"\n"
"; ---------------------------------------------\n"
"; PROCESS SETTINGS\n"
"; ---------------------------------------------\n"
";\n"
"; terminal_emulator                = /usr/bin/x-terminal-emulator -e    command for starting applications with an external terminal emulator. If empty, the command will be ignored. Ignored on Windows.\n"
";                                                                       e.g.  /usr/bin/x-terminal-emulator -e\n"
";                                                                             /usr/bin/gnome-terminal -x\n"
";                                                                             /usr/bin/xterm -e\n"
";\n"
"; ---------------------------------------------\n"
"[process]\n"
"terminal_emulator                  = \n"
"\n"
"; --------------------------------------------------\n"
"; PUBLISHER SETTINGS\n"
"; --------------------------------------------------\n"
"; use_shm                          = 0, 1, 2                       Use shared memory transport layer (0 = off, 1 = on, 2 = auto, default = 2)\n"
"; use_tcp                          = 0, 1, 2                       Use tcp transport layer           (0 = off, 1 = on, 2 = auto, default = 0)\n"
"; use_udp_mc                       = 0, 1, 2                       Use udp multicast transport layer (0 = off, 1 = on, 2 = auto, default = 2)\n"
";\n"
"; memfile_minsize                  = x * 4096 kB                   Default memory file size for new publisher\n"
";\n"
"; memfile_reserve                  = 50 .. x %                     Dynamic file size reserve before recreating memory file if topic size changes\n"
";\n"
"; memfile_ack_timeout              = 0 .. x ms                     Publisher timeout for ack event from subscriber that memory file content is processed\n"
";\n"
"; memfile_buffer_count             = 1 .. x                        Number of parallel used memory file buffers for 1:n publish/subscribe ipc connections (default = 1)\n"
"; memfile_zero_copy                = 0, 1                          Allow matching subscriber to access memory file without copying its content in advance (blocking mode)\n"
";\n"
"; share_ttype                      = 0, 1                          Share topic type via registration layer\n"
"; share_tdesc                      = 0, 1                          Share topic description via registration layer (switch off to disable reflection)\n"
"; --------------------------------------------------\n"
"[publisher]\n"
"use_shm                            = 2\n"
"use_tcp                            = 0\n"
"use_udp_mc                         = 2\n"
"\n"
"memfile_minsize                    = 4096\n"
"memfile_reserve                    = 50\n"
"memfile_ack_timeout                = 0\n"
"memfile_buffer_count               = 1\n"
"memfile_zero_copy                  = 0\n"
"\n"
"share_ttype                        = 1\n"
"share_tdesc                        = 1\n"
"\n"
"; --------------------------------------------------\n"
"; SERVICE SETTINGS\n"
"; --------------------------------------------------\n"
"; protocol_v0                      = 0, 1                          Support service protocol v0, eCAL 5.11 and older (0 = off, 1 = on)\n"
"; protocol_v1                      = 0, 1                          Support service protocol v1, eCAL 5.12 and newer (0 = off, 1 = on)\n"
"; --------------------------------------------------\n"
"[service]\n"
"protocol_v0                        = 1\n"
"protocol_v1                        = 1\n"
"\n"
"; --------------------------------------------------\n"
"; MONITORING SETTINGS\n"
"; --------------------------------------------------\n"
"; timeout                          = 1000 + (x * 1000)             Timeout for topic monitoring in ms\n"
"; filter_excl                      =  __.*                         Topics blacklist as regular expression (will not be monitored)\n"
"; filter_incl                      =                               Topics whitelist as regular expression (will be monitored only)\n"
"; filter_log_con                   = info, warning, error, fatal   Log messages logged to console (all, info, warning, error, fatal, debug1, debug2, debug3, debug4)\n"
"; filter_log_file                  =                               Log messages to logged into file system\n"
"; filter_log_udp                   = info, warning, error, fatal   Log messages logged via udp network\n"
"; --------------------------------------------------\n"
"[monitoring]\n"
"timeout                            = 5000\n"
"filter_excl                        = __.*\n"
"filter_incl                        =\n"
"filter_log_con                     = info, warning, error, fatal\n"
"filter_log_file                    =\n"
"filter_log_udp                     = info, warning, error, fatal\n"
"\n"
"; --------------------------------------------------\n"
"; SYS SETTINGS\n"
"; --------------------------------------------------\n"
"; filter_excl                      = App1,App2                     Apps blacklist to be excluded when importing tasks from cloud\n"
"; --------------------------------------------------\n"
"[sys]\n"
"filter_excl                        = ^eCALSysClient$|^eCALSysGUI$|^eCALSys$\n"
"\n"
"; --------------------------------------------------\n"
"; EXPERIMENTAL SETTINGS\n"
"; --------------------------------------------------\n"
"; shm_monitoring_enabled           = false                         Enable distribution of monitoring/registration information via shared memory\n"
"; shm_monitoring_domain            = ecal_monitoring               Domain name for shared memory based monitoring/registration\n"
"; shm_monitoring_queue_size        = 1024                          Queue size of monitoring/registration events\n"
"; network_monitoring_enabled       = true                          Enable distribution of monitoring/registration information via network\n"
";\n"
"; drop_out_of_order_messages       = false                         Enable dropping of payload messages that arrive out of order\n"
"; --------------------------------------------------\n"
"[experimental]\n"
"shm_monitoring_enabled             = false\n"
"shm_monitoring_domain              = ecal_mon\n"
"shm_monitoring_queue_size          = 1024\n"
"network_monitoring_enabled         = true\n"
"drop_out_of_order_messages         = false\n"
;

std::string ini_file_as_string_yaml = R"(#  _____     _ _                                ____    _    _                             
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
    port: 14010
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
    npcap_enabled: true 
    
    # In local mode multicast group and ttl are set by default and are not adjustable
    local:
      # Multicast group base. All registration and logging is sent on this address 
      # group: "127.0.0.1" 
      # TTL (hop limit) is used to determine the amount of routers being traversed towards the destination
      # ttl: 0

    network:
      # Multicast group base. All registration and logging is sent on this address 
      group: "239.5.0.1" 
      # TTL (hop limit) is used to determine the amount of routers being traversed towards the destination
      ttl: 3   
    
  tcp: 
    # Reader amount of threads that shall execute workload
    number_executor_reader: 4
    # Writer amount of threads that shall execute workload
    number_executor_writer: 4
    # Reconnection attemps the session will try to reconnect in case of an issue
    max_reconnections: 7

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
    enable: false
    # Enable zero copy shared memory transport mode
    zero_copy_mode: false
    # Force connected subscribers to send acknowledge event after processing the message.
    # The publisher send call is blocked on this event with this timeout (0 == no handshake).
    acknowledge_timeout_ms: 346
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
    enable: false

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
    emulator: "myTestTerminal"


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