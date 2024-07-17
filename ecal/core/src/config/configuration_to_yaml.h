#ifndef CONFIGURATION_TO_YAML_H
#define CONFIGURATION_TO_YAML_H

#include <ecal/config/configuration.h>
#include <ecal_def.h>

#ifndef YAML_CPP_STATIC_DEFINE 
#define YAML_CPP_STATIC_DEFINE 
#endif
#include <yaml-cpp/yaml.h>

// utility functions for yaml node handling
namespace YAML
{
  template<typename AS, typename MEM>
  void AssignValue(MEM& member, const YAML::Node& node_, const char* key)
  {
    if (node_[key])
      member = node_[key].as<AS>();
  }

  // Operator overload for assigning a ConstrainedInteger to a YAML::Node
  template<int MIN, int STEP, int MAX>
  YAML::Node operator<<(YAML::Node node, const eCAL::Types::ConstrainedInteger<MIN, STEP, MAX>& constrainedInt) {
      node = static_cast<int>(constrainedInt);
      return node;
  }
}

namespace YAML
{


  // 3rd layer configuration objects
  // application objects
  template<>
  struct convert<eCAL::Application::Startup::Configuration>
  { 
    //static constexpr char* emulator_key = "emulator";

    static Node encode(const eCAL::Application::Startup::Configuration& config_)
    {
      Node node;
      node["emulator"] = config_.terminal_emulator;

      return node;
    }

    static bool decode(const Node& node_, eCAL::Application::Startup::Configuration& config_)
    {
      AssignValue<std::string>(config_.terminal_emulator, node_, "emulator");

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
      AssignValue<std::string>(config_.filter_excl, node_, "filter_excl");
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
      AssignValue<std::string>(config_.shm_monitoring_domain, node_, "domain");
      AssignValue<size_t>(config_.shm_monitoring_queue_size, node_, "queue_size");
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

      return node;
    }

    static bool decode(const Node& node_, eCAL::Publisher::SHM::Configuration& config_)
    {
      AssignValue<bool>(config_.enable, node_, "enable");
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

      return node;
    }

    static bool decode(const Node& node_, eCAL::Publisher::UDP::Configuration& config_)
    {
      AssignValue<bool>(config_.enable, node_, "enable");
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
      AssignValue<bool>(config_.enable, node_, "enable");
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
      AssignValue<bool>(config_.enable, node_, "enable");
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
      AssignValue<bool>(config_.enable, node_, "enable");
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
      AssignValue<bool>(config_.enable, node_, "enable");
      return true;
    }
  };

  template<>
  struct convert<eCAL::TransportLayer::SHM::Configuration>
  {
    static Node encode(const eCAL::TransportLayer::SHM::Configuration& config_)
    {
      Node node;      
      node["zero_copy_mode"] = config_.zero_copy_mode;
      node["acknowledge_timeout_ms"] = config_.acknowledge_timeout_ms;
      node["memfile_buffer_count"] << config_.memfile_buffer_count;
      node["memfile_min_size_bytes"] << config_.memfile_min_size_bytes;
      node["memfile_reserve_percent"] << config_.memfile_reserve_percent;
      return node;
    }

    static bool decode(const Node& node_, eCAL::TransportLayer::SHM::Configuration& config_)
    {
      AssignValue<bool>(config_.zero_copy_mode, node_, "zero_copy_mode");
      AssignValue<unsigned int>(config_.acknowledge_timeout_ms, node_, "acknowledge_timeout_ms");
      AssignValue<unsigned int>(config_.memfile_buffer_count, node_, "memfile_buffer_count");
      AssignValue<unsigned int>(config_.memfile_min_size_bytes, node_, "memfile_min_size_bytes");
      AssignValue<unsigned int>(config_.memfile_reserve_percent, node_, "memfile_reserve_percent");
      return true;
    }
  };

  template<>
  struct convert<eCAL::TransportLayer::TCP::Configuration>
  {
    static Node encode(const eCAL::TransportLayer::TCP::Configuration& config_)
    {
      Node node;
      node["number_executor_reader"] = config_.number_executor_reader;
      node["number_executor_writer"] = config_.number_executor_writer;
      node["max_reconnections"] = config_.max_reconnections;

      return node;
    }

    static bool decode(const Node& node_, eCAL::TransportLayer::TCP::Configuration& config_)
    {
      AssignValue<unsigned int>(config_.number_executor_reader, node_, "number_executor_reader");
      AssignValue<unsigned int>(config_.number_executor_writer, node_, "number_executor_writer");
      AssignValue<unsigned int>(config_.max_reconnections, node_, "max_reconnections");
      return true;
    }
  };

  void AssignLayersUdpGeneralSettings(eCAL::Layer::UDP::General *config_, const Node& node_)
  {
    auto& general_config = *config_;
    AssignValue<bool>(general_config.join_all_interfaces, node_, "join_all_interfaces");
    AssignValue<bool>(general_config.npcap_enabled, node_, "npcap_enabled");
    AssignValue<unsigned int>(general_config.port, node_, "port");
    AssignValue<unsigned int>(general_config.receive_buffer, node_, "receive_buffer");
    AssignValue<unsigned int>(general_config.send_buffer, node_, "send_buffer");
  }

  void LayersUdpGeneralSettingsToYaml(const eCAL::Layer::UDP::General *config_, Node& node_)
  {
    node_["join_all_interfaces"] = config_->join_all_interfaces;
    node_["npcap_enable"]        = config_->npcap_enabled;
    node_["port"]                << config_->port;
    node_["receive_buffer"]      << config_->receive_buffer;
    node_["send_buffer"]         << config_->send_buffer;
  }


  template<>
  struct convert<eCAL::Layer::UDP::Local>
  {
    static Node encode(const eCAL::Layer::UDP::Local& config_)
    {
      Node node;
      LayersUdpGeneralSettingsToYaml(&config_, node);
      return node;
    }

    static bool decode(const Node& node_, eCAL::Layer::UDP::Local& config_)
    {
      AssignLayersUdpGeneralSettings(&config_, node_);
      return true;
    }
  };

  template<>
  struct convert<eCAL::Layer::UDP::Network>
  {
    static Node encode(const eCAL::Layer::UDP::Network& config_)
    {
      Node node;
      LayersUdpGeneralSettingsToYaml(&config_, node);
      
      node["group"] = config_.group.Get();
      node["ttl"] = config_.ttl;
      return node;
    }

    static bool decode(const Node& node_, eCAL::Layer::UDP::Network& config_)
    {
      AssignLayersUdpGeneralSettings(&config_, node_);

      AssignValue<std::string>(config_.group, node_, "group");
      AssignValue<unsigned int>(config_.ttl, node_, "ttl");
      return true;
    }
  };

  template<>
  struct convert<eCAL::TransportLayer::UDP::Configuration>
  {
    static Node encode(const eCAL::TransportLayer::UDP::Configuration& config_)
    {
      Node node;
      node["config_version"] = config_.config_version == eCAL::Types::UdpConfigVersion::V1 ? "v1" : "v2";
      node["mode"]           = config_.mode == eCAL::Layer::UDP::MODE::LOCAL ? "local" : "network";
      node["mask"]           = config_.mask.Get();
      node["local"]          = config_.local;
      node["network"]        = config_.network;

      return node;
    }

    static bool decode(const Node& node_, eCAL::TransportLayer::UDP::Configuration& config_)
    {
      std::string temp_string = "v2";
      AssignValue<std::string>(temp_string, node_, "config_version");
      config_.config_version = temp_string == "v2" ? eCAL::Types::UdpConfigVersion::V2 : eCAL::Types::UdpConfigVersion::V1;
      temp_string = "local";
      AssignValue<std::string>(temp_string, node_, "mode");
      config_.mode = temp_string == "local" ? eCAL::Layer::UDP::MODE::LOCAL : eCAL::Layer::UDP::MODE::CLOUD;
      AssignValue<std::string>(config_.mask, node_, "mask");
      AssignValue<eCAL::Layer::UDP::Local>(config_.local, node_, "local");
      AssignValue<eCAL::Layer::UDP::Network>(config_.network, node_, "network");
      return true;
    }
  };

  // 2nd layer configuration objects
  template<>
  struct convert<eCAL::TransportLayer::Configuration>
  {
    static Node encode(const eCAL::TransportLayer::Configuration& config_)
    {
      Node node;
      node["shm"] = config_.shm;
      node["udp"] = config_.udp;
      node["tcp"] = config_.tcp;

      return node;
    }

    static bool decode(const Node& node_, eCAL::TransportLayer::Configuration& config_)
    {
      AssignValue<eCAL::TransportLayer::SHM::Configuration>(config_.shm, node_, "shm");
      AssignValue<eCAL::TransportLayer::UDP::Configuration>(config_.udp, node_, "udp");
      AssignValue<eCAL::TransportLayer::TCP::Configuration>(config_.tcp, node_, "tcp");
      return true;
    }
  };

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
      AssignValue<eCAL::Application::Startup::Configuration>(config_.startup, node_, "terminal");
      AssignValue<eCAL::Application::Sys::Configuration>(config_.sys, node_, "sys");
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
      AssignValue<unsigned int>(config_.monitoring_timeout, node_, "timeout");
      AssignValue<std::string>(config_.filter_excl, node_, "filter_excl");
      AssignValue<std::string>(config_.filter_incl, node_, "filter_incl");
      AssignValue<eCAL::Monitoring::UDP::Configuration>(config_.udp_options, node_, "udp");
      AssignValue<eCAL::Monitoring::SHM::Configuration>(config_.shm_options, node_, "shm");
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
      node["priority_local"]          = config_.priority_local;
      node["priority_network"]        = config_.priority_network;
      return node;
    }

    static bool decode(const Node& node_, eCAL::Publisher::Configuration& config_)
    {
      AssignValue<bool>(config_.share_topic_description, node_, "share_topic_description");
      AssignValue<bool>(config_.share_topic_type, node_, "share_topic_type");
      AssignValue<std::vector<std::string>>(config_.priority_local, node_, "priority_local");
      AssignValue<std::vector<std::string>>(config_.priority_network, node_, "priority_network");
      AssignValue<eCAL::Publisher::SHM::Configuration>(config_.shm, node_, "shm");
      AssignValue<eCAL::Publisher::UDP::Configuration>(config_.udp, node_, "udp");
      AssignValue<eCAL::Publisher::TCP::Configuration>(config_.tcp, node_, "tcp");      
      return true;
    }
  };

  template<>
  struct convert<eCAL::Registration::Configuration>
  {
    static Node encode(const eCAL::Registration::Configuration& config_)
    {
      Node node;
      node["registration_timeout"] = config_.getTimeoutMS();
      node["registration_refresh"] = config_.getRefreshMS();
      node["network_enabled"] = config_.network_enabled;
      node["shm_registration_enabled"] = config_.shm_registration_enabled;
      node["loopback"] = config_.loopback;
      node["host_group_name"] = config_.host_group_name;
      return node;
    }

    static bool decode(const Node& node_, eCAL::Registration::Configuration& config_)
    {
      unsigned int reg_timeout = 60000;
      unsigned int reg_refresh = 1000;
      AssignValue<unsigned int>(reg_timeout, node_, "registration_timeout");
      AssignValue<unsigned int>(reg_refresh, node_, "registration_refresh");
      
      config_ = {reg_timeout, reg_refresh};
      AssignValue<bool>(config_.network_enabled, node_, "network_enabled");
      AssignValue<bool>(config_.shm_registration_enabled, node_, "shm_registration_enabled");
      AssignValue<bool>(config_.loopback, node_, "loopback");
      AssignValue<std::string>(config_.host_group_name, node_, "host_group_name");
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
      AssignValue<bool>(config_.protocol_v0, node_, "protocol_v0");
      AssignValue<bool>(config_.protocol_v1, node_, "protocol_v1");
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
      node["drop_out_of_order_message"] = config_.drop_out_of_order_messages;
      return node;
    }

    static bool decode(const Node& node_, eCAL::Subscriber::Configuration& config_)
    {
      AssignValue<eCAL::Subscriber::SHM::Configuration>(config_.shm, node_, "shm");
      AssignValue<eCAL::Subscriber::TCP::Configuration>(config_.tcp, node_, "tcp");
      AssignValue<eCAL::Subscriber::UDP::Configuration>(config_.udp, node_, "udp");
      AssignValue<bool>(config_.drop_out_of_order_messages, node_, "dropt_out_of_order_messages");
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
      AssignValue<std::string>(config_.timesync_module_replay, node_, "replay");
      AssignValue<std::string>(config_.timesync_module_rt, node_, "rt");
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

      return node;
    }

    static bool decode(const Node& node_, eCAL::Configuration& config_)
    {
      AssignValue<eCAL::TransportLayer::Configuration>(config_.transport_layer, node_, "transport_layer");
      AssignValue<eCAL::Publisher::Configuration>(config_.publisher, node_, "publisher");
      AssignValue<eCAL::Subscriber::Configuration>(config_.subscriber, node_, "subscriber");
      AssignValue<eCAL::Registration::Configuration>(config_.registration, node_, "registration");
      AssignValue<eCAL::Monitoring::Configuration>(config_.monitoring, node_, "monitoring");
      AssignValue<eCAL::Time::Configuration>(config_.timesync, node_, "time");
      AssignValue<eCAL::Service::Configuration>(config_.service, node_, "service");
      AssignValue<eCAL::Application::Configuration>(config_.application, node_, "application");
      AssignValue<eCAL::Logging::Configuration>(config_.logging, node_, "logging");
      return true;
    }
  };  
}

#endif // CONFIGURATION_TO_YAML_H