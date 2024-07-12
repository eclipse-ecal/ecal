#ifndef CONFIGURATION_TO_YAML_H
#define CONFIGURATION_TO_YAML_H

#include <ecal/config/configuration.h>

#ifndef YAML_CPP_STATIC_DEFINE 
#define YAML_CPP_STATIC_DEFINE 
#endif
#include <yaml-cpp/yaml.h>


namespace YAML
{
  // 3rd layer configuration objects
  // application objects
  template<>
  struct convert<eCAL::Application::Startup::Configuration>
  {
    static Node encode(const eCAL::Application::Startup::Configuration& config_)
    {
      Node node;
      node["emulator"] = config_.terminal_emulator;

      return node;
    }

    static bool decode(const Node& node_, eCAL::Application::Startup::Configuration& config_)
    {
      if (node_["emulator"])
        config_.terminal_emulator = node_["emulator"].as<std::string>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::Application::Sys::Configuration>
  {
    static Node encode(const eCAL::Application::Sys::Configuration& config_)
    {
      Node node;
      node["filter_excl"] = config_.filter_excl;

      return node;
    }

    static bool decode(const Node& node_, eCAL::Application::Sys::Configuration& config_)
    {
      if (node_["filter_excl"])
        config_.filter_excl = node_["filter_excl"].as<std::string>();
      return true;
    }
  };

  // monitoring objects
  template<>
  struct convert<eCAL::Monitoring::SHM::Configuration>
  {
    static Node encode(const eCAL::Monitoring::SHM::Configuration& config_)
    {
      Node node;
      node["domain"]     = config_.shm_monitoring_domain;
      node["queue_size"] = config_.shm_monitoring_queue_size;
      return node;
    }

    static bool decode(const Node& node_, eCAL::Monitoring::SHM::Configuration& config_)
    {
      if (node_["domain"])
        config_.shm_monitoring_domain     = node_["domain"].as<std::string>();
      if (node_["queue_size"])
        config_.shm_monitoring_queue_size = node_["queue_size"].as<size_t>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::Monitoring::UDP::Configuration>
  {
    static Node encode(const eCAL::Monitoring::UDP::Configuration&)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node&, eCAL::Monitoring::UDP::Configuration&)
    {
      // Empty struct here at the moment
      return true;
    }
  };

  // publisher configuration objects
  template<>
  struct convert<eCAL::Publisher::SHM::Configuration>
  {
    static Node encode(const eCAL::Publisher::SHM::Configuration& config_)
    {
      Node node;
      node["enable"]                  = config_.enable;
      node["zero_copy_mode"]          = config_.zero_copy_mode;
      node["acknowledge_timeout_ms"]  = config_.acknowledge_timeout_ms;
      node["memfile_min_size_bytes"]  = static_cast<int>(config_.memfile_min_size_bytes);
      node["memfile_reserve_percent"] = static_cast<int>(config_.memfile_reserve_percent);
      node["memfile_buffer_count"]    = static_cast<int>(config_.memfile_buffer_count);

      return node;
    }

    static bool decode(const Node& node_, eCAL::Publisher::SHM::Configuration& config_)
    {
      if (node_["enable"])
        config_.enable                  = node_["enable"].as<bool>();
      if (node_["zero_copy_mode"])
        config_.zero_copy_mode          = node_["zero_copy_mode"].as<bool>();
      if (node_["acknowledge_timeout_ms"])
        config_.acknowledge_timeout_ms  = node_["acknowledge_timeout_ms"].as<unsigned int>();
      if (node_["memfile_min_size_bytes"])
        config_.memfile_min_size_bytes  = node_["memfile_min_size_bytes"].as<unsigned int>();
      if (node_["memfile_reserve_percent"])
        config_.memfile_reserve_percent = node_["memfile_reserve_percent"].as<unsigned int>();
      if (node_["memfile_buffer_count"])
        config_.memfile_buffer_count    = node_["memfile_buffer_count"].as<unsigned int>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::Publisher::UDP::Configuration>
  {
    static Node encode(const eCAL::Publisher::UDP::Configuration& config_)
    {
      Node node;
      node["enable"]             = config_.enable;
      node["loopback"]           = config_.loopback;
      node["sndbuff_size_bytes"] = static_cast<int>(config_.sndbuf_size_bytes);

      return node;
    }

    static bool decode(const Node& node_, eCAL::Publisher::UDP::Configuration& config_)
    {
      if (node_["enable"])
        config_.enable            = node_["enable"].as<bool>();
      if (node_["loopback"])
        config_.loopback          = node_["loopback"].as<bool>();
      if (node_["sndbuff_size_bytes"])
        config_.sndbuf_size_bytes = node_["sndbuff_size_bytes"].as<unsigned int>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::Publisher::TCP::Configuration>
  {
    static Node encode(const eCAL::Publisher::TCP::Configuration& config_)
    {
      Node node;
      node["enable"] = config_.enable;

      return node;
    }

    static bool decode(const Node& node_, eCAL::Publisher::TCP::Configuration& config_)
    {
      if (node_["enable"])
        config_.enable = node_["enable"].as<bool>();
      return true;
    }
  };

  // subscriber configuration objects
  template<>
  struct convert<eCAL::Subscriber::SHM::Configuration>
  {
    static Node encode(const eCAL::Subscriber::SHM::Configuration& config_)
    {
      Node node;
      node["enable"] = config_.enable;

      return node;
    }

    static bool decode(const Node& node_, eCAL::Subscriber::SHM::Configuration& config_)
    {
      if (node_["enable"])
        config_.enable = node_["enable"].as<bool>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::Subscriber::UDP::Configuration>
  {
    static Node encode(const eCAL::Subscriber::UDP::Configuration& config_)
    {
      Node node;
      node["enable"] = config_.enable;

      return node;
    }

    static bool decode(const Node& node_, eCAL::Subscriber::UDP::Configuration& config_)
    {
      if (node_["enable"])
        config_.enable = node_["enable"].as<bool>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::Subscriber::TCP::Configuration>
  {
    static Node encode(const eCAL::Subscriber::TCP::Configuration& config_)
    {
      Node node;
      node["enable"] = config_.enable;

      return node;
    }

    static bool decode(const Node& node_, eCAL::Subscriber::TCP::Configuration& config_)
    {
      if (node_["enable"])
        config_.enable = node_["enable"].as<bool>();
      return true;
    }
  };

  // transport layer configuration objects
  template<>
  struct convert<eCAL::TransportLayer::TCPPubSub::Configuration>
  {
    static Node encode(const eCAL::TransportLayer::TCPPubSub::Configuration& config_)
    {
      Node node;
      node["number_executor_reader"] = config_.num_executor_reader;
      node["number_executor_writer"] = config_.num_executor_writer;
      node["max_reconnections"]      = config_.max_reconnections;

      return node;
    }

    static bool decode(const Node& node_, eCAL::TransportLayer::TCPPubSub::Configuration& config_)
    {
      if (node_["number_executor_reader"])
        config_.num_executor_reader = node_["number_executor_reader"].as<size_t>();
      if (node_["number_executor_writer"])
        config_.num_executor_writer = node_["number_executor_writer"].as<size_t>();
      if (node_["max_reconnections"])
        config_.max_reconnections   = node_["max_reconnections"].as<size_t>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::TransportLayer::SHM::Configuration>
  {
    static Node encode(const eCAL::TransportLayer::SHM::Configuration& config_)
    {
      Node node;
      node["host_group_name"] = config_.host_group_name;

      return node;
    }

    static bool decode(const Node& node_, eCAL::TransportLayer::SHM::Configuration& config_)
    {
      if (node_["host_group_name"])
        config_.host_group_name = node_["host_group_name"].as<std::string>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::TransportLayer::UDPMC::Configuration>
  {
    static Node encode(const eCAL::TransportLayer::UDPMC::Configuration& config_)
    {
      Node node;
      node["config_version"]      = config_.config_version == eCAL::Types::UdpConfigVersion::V1 ? "v1" : "v2";
      node["group"]               = config_.group.Get();
      node["mask"]                = config_.mask.Get();
      node["port"]                = static_cast<int>(config_.port);
      node["ttl"]                 = config_.ttl;
      node["send_buffer"]         = static_cast<int>(config_.sndbuf);
      node["receive_buffer"]      = static_cast<int>(config_.recbuf);
      node["join_all_interfaces"] = config_.join_all_interfaces;
      node["npcap_enabled"]       = config_.npcap_enabled;

      return node;
    }

    static bool decode(const Node& node_, eCAL::TransportLayer::UDPMC::Configuration& config_)
    {
      if (node_["config_version"])
        config_.config_version      = node_["config_version"].as<std::string>() == "v1" ? eCAL::Types::UdpConfigVersion::V1 : eCAL::Types::UdpConfigVersion::V2;
      if (node_["group"])
        config_.group               = node_["group"].as<std::string>();
      if (node_["mask"])
        config_.mask                = node_["mask"].as<std::string>();
      if (node_["port"])
        config_.port                = node_["port"].as<unsigned int>();
      if (node_["ttl"])
        config_.ttl                 = node_["ttl"].as<unsigned int>();
      if (node_["send_buffer"])
        config_.sndbuf              = node_["send_buffer"].as<unsigned int>();
      if (node_["receive_buffer"])
        config_.recbuf              = node_["receive_buffer"].as<unsigned int>();
      if (node_["join_all_interfaces"])
        config_.join_all_interfaces = node_["join_all_interfaces"].as<bool>();
      if (node_["npcap_enabled"])
        config_.npcap_enabled       = node_["npcap_enabled"].as<bool>();
      return true;
    }
  };

  // 2nd layer configuration objects
  template<>
  struct convert<eCAL::Application::Configuration>
  {
    static Node encode(const eCAL::Application::Configuration& config_)
    {
      Node node;
      node["terminal"] = config_.startup;
      node["sys"]      = config_.sys;
      
      return node;
    }

    static bool decode(const Node& node_, eCAL::Application::Configuration& config_)
    {
      if (node_["terminal"])
        config_.startup = node_["terminal"].as<eCAL::Application::Startup::Configuration>();
      if (node_["sys"])
        config_.sys     = node_["sys"].as<eCAL::Application::Sys::Configuration>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::Logging::Configuration>
  {
    static Node encode(const eCAL::Logging::Configuration&)
    {
      Node node;
      // TODO PG: add proper list read out when "eCAL_Logging_Filter" changed to handling with list
      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node&, eCAL::Logging::Configuration& config_)
    {
      // TODO PG: add proper list read out when "eCAL_Logging_Filter" changed to handling with list
      config_.filter_log_con  = {};
      config_.filter_log_file = {};
      config_.filter_log_udp  = {};
      return true;
    }
  };

  template<>
  struct convert<eCAL::Monitoring::Configuration>
  {
    static Node encode(const eCAL::Monitoring::Configuration& config_)
    {
      Node node;
      // TODO PG:
      //config_.monitoring_mode
      node["mode"] = std::list<std::string>{};
      node["timeout"] = static_cast<int>(config_.monitoring_timeout);
      node["filter_excl"] = config_.filter_excl;
      node["filter_incl"] = config_.filter_incl;
      node["udp"] = config_.udp_options;
      node["shm"] = config_.shm_options;

      return node;
    }

    static bool decode(const Node& node_, eCAL::Monitoring::Configuration& config_)
    {
      if (node_["timeout"])
        config_.monitoring_timeout = node_["timeout"].as<unsigned int>();
      if (node_["filter_excl"])
        config_.filter_excl = node_["filter_excl"].as<std::string>();
      if (node_["filter_incl"])
        config_.filter_incl = node_["filter_incl"].as<std::string>();
      if (node_["udp"])
        config_.udp_options = node_["udp"].as<eCAL::Monitoring::UDP::Configuration>();
      if (node_["shm"])
        config_.shm_options = node_["shm"].as<eCAL::Monitoring::SHM::Configuration>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::Publisher::Configuration>
  {
    static Node encode(const eCAL::Publisher::Configuration& config_)
    {
      Node node;
      node["share_topic_description"] = config_.share_topic_description;
      node["share_topic_type"]        = config_.share_topic_type;
      node["shm"]                     = config_.shm;
      node["udp"]                     = config_.udp;
      node["tcp"]                     = config_.tcp;

      return node;
    }

    static bool decode(const Node& node_, eCAL::Publisher::Configuration& config_)
    {
      if (node_["share_topic_description"])
        config_.share_topic_description = node_["share_topic_description"].as<bool>();
      if (node_["share_topic_type"])
        config_.share_topic_type        = node_["share_topic_type"].as<bool>();
      if (node_["shm"])
        config_.shm                     = node_["shm"].as<eCAL::Publisher::SHM::Configuration>();
      if (node_["udp"])
        config_.udp                     = node_["udp"].as<eCAL::Publisher::UDP::Configuration>();
      if (node_["tcp"])
        config_.tcp                     = node_["tcp"].as<eCAL::Publisher::TCP::Configuration>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::Registration::Configuration>
  {
    static Node encode(const eCAL::Registration::Configuration& config_)
    {
      Node node;
      node["share_ttype"] = config_.share_ttype;
      node["share_tdesc"] = config_.share_tdesc;
      node["registration_timeout"] = config_.getTimeoutMS();
      node["registration_refresh"] = config_.getRefreshMS();
      node["network_enabled"] = config_.network_enabled;
      node["shm_registration_enabled"] = config_.shm_registration_enabled;

      return node;
    }

    static bool decode(const Node& node_, eCAL::Registration::Configuration& config_)
    {
      if (node_["registration_timeout"] && node_["registration_refresh"])
      {
        unsigned int reg_timeout = node_["registration_timeout"].as<unsigned int>();
        unsigned int reg_refresh = node_["registration_refresh"].as<unsigned int>();
        config_             = {reg_timeout, reg_refresh};
      }
      
      if (node_["share_ttype"])
        config_.share_ttype = node_["share_ttype"].as<bool>();
      if (node_["share_tdesc"])
        config_.share_tdesc = node_["share_tdesc"].as<bool>();
      if (node_["network_enabled"])
        config_.network_enabled = node_["network_enabled"].as<bool>();
      if (node_["shm_registration_enabled"])
        config_.shm_registration_enabled = node_["shm_registration_enabled"].as<bool>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::Service::Configuration>
  {
    static Node encode(const eCAL::Service::Configuration& config_)
    {
      Node node;
      node["protocol_v0"] = config_.protocol_v0;
      node["protocol_v1"] = config_.protocol_v1;

      return node;
    }

    static bool decode(const Node& node_, eCAL::Service::Configuration& config_)
    {
      if (node_["protocol_v0"])
        config_.protocol_v0 = node_["protocol_v0"].as<bool>();
      if (node_["protocol_v1"])
        config_.protocol_v1 = node_["protocol_v1"].as<bool>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::Subscriber::Configuration>
  {
    static Node encode(const eCAL::Subscriber::Configuration& config_)
    {
      Node node;
      node["shm"] = config_.shm;
      node["tcp"] = config_.tcp;
      node["udp"] = config_.udp;

      return node;
    }

    static bool decode(const Node& node_, eCAL::Subscriber::Configuration& config_)
    {
      if (node_["shm"])
        config_.shm = node_["shm"].as<eCAL::Subscriber::SHM::Configuration>();
      if (node_["tcp"])
        config_.tcp = node_["tcp"].as<eCAL::Subscriber::TCP::Configuration>();
      if (node_["udp"])
        config_.udp = node_["udp"].as<eCAL::Subscriber::UDP::Configuration>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::Time::Configuration>
  {
    static Node encode(const eCAL::Time::Configuration& config_)
    {
      Node node;
      node["replay"] = config_.timesync_module_replay;
      node["rt"]     = config_.timesync_module_rt;

      return node;
    }

    static bool decode(const Node& node_, eCAL::Time::Configuration& config_)
    {
      if (node_["replay"])
        config_.timesync_module_replay = node_["replay"].as<std::string>();
      if (node_["rt"])
        config_.timesync_module_rt     = node_["rt"].as<std::string>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::TransportLayer::Configuration>
  {
    static Node encode(const eCAL::TransportLayer::Configuration& config_)
    {
      Node node;
      node["drop_out_of_order_messages"] = config_.drop_out_of_order_messages;
      node["udp_mc"]                     = config_.mc_options;
      node["tcppubsub"]                  = config_.tcp_options;
      node["shm"]                        = config_.shm_options;

      return node;
    }

    static bool decode(const Node& node_, eCAL::TransportLayer::Configuration& config_)
    {
      if (node_["drop_out_of_order_messages"])
        config_.drop_out_of_order_messages = node_["drop_out_of_order_messages"].as<bool>();
      if (node_["udp_mc"])
        config_.mc_options                 = node_["udp_mc"].as<eCAL::TransportLayer::UDPMC::Configuration>();
      if (node_["tcppubsub"])
        config_.tcp_options                = node_["tcppubsub"].as<eCAL::TransportLayer::TCPPubSub::Configuration>();
      if (node_["shm"])
        config_.shm_options                = node_["shm"].as<eCAL::TransportLayer::SHM::Configuration>();
      return true;
    }
  };

  // Main configuration object
  template<>
  struct convert<eCAL::Configuration>
  {
    static Node encode(const eCAL::Configuration& config_)
    {
      Node node;
      node["publisher"]       = config_.publisher;
      node["subscriber"]      = config_.subscriber;
      node["registration"]    = config_.registration;
      node["monitoring"]      = config_.monitoring;
      node["time"]            = config_.timesync;
      node["service"]         = config_.service;
      node["application"]     = config_.application;
      node["logging"]         = config_.logging;
      node["transport_layer"] = config_.transport_layer;

      return node;
    }

    static bool decode(const Node& node_, eCAL::Configuration& config_)
    {
      if (node_["publisher"])
        config_.publisher              = node_["publisher"].as<eCAL::Publisher::Configuration>();
      if (node_["subscriber"])
        config_.subscriber             = node_["subscriber"].as<eCAL::Subscriber::Configuration>();
      if (node_["registration"])
        config_.registration           = node_["registration"].as<eCAL::Registration::Configuration>();
      if (node_["monitoring"])
        config_.monitoring             = node_["monitoring"].as<eCAL::Monitoring::Configuration>();
      if (node_["time"])
        config_.timesync               = node_["time"].as<eCAL::Time::Configuration>();
      if (node_["service"])
        config_.service                = node_["service"].as<eCAL::Service::Configuration>();
      if (node_["application"])
        config_.application            = node_["application"].as<eCAL::Application::Configuration>();
      if (node_["logging"])
        config_.logging                = node_["logging"].as<eCAL::Logging::Configuration>();
      if (node_["transport_layer"])
        config_.transport_layer        = node_["transport_layer"].as<eCAL::TransportLayer::Configuration>();
      return true;
    }
  };  
}

#endif // CONFIGURATION_TO_YAML_H