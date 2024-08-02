#include "configuration_to_yaml.h"

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
  YAML::Node operator<<(YAML::Node node, const eCAL::Types::ConstrainedInteger<MIN, STEP, MAX>& constrainedInt) 
  {
      node = static_cast<int>(constrainedInt);
      return node;
  }

  eCAL_Logging_Filter ParseLogLevel(const std::vector<std::string>& filter_)
  {
    // create excluding filter list
    char filter_mask = log_level_none;
    for (auto& it : filter_)
    {
      if (it == "all")     filter_mask |= log_level_all;
      if (it == "info")    filter_mask |= log_level_info;
      if (it == "warning") filter_mask |= log_level_warning;
      if (it == "error")   filter_mask |= log_level_error;
      if (it == "fatal")   filter_mask |= log_level_fatal;
      if (it == "debug1")  filter_mask |= log_level_debug1;
      if (it == "debug2")  filter_mask |= log_level_debug2;
      if (it == "debug3")  filter_mask |= log_level_debug3;
      if (it == "debug4")  filter_mask |= log_level_debug4;
    }

    return(filter_mask);
  };

  std::vector<std::string> LogLevelToVector(eCAL_Logging_Filter filter_mask) 
  {
    std::vector<std::string> filter;
    if (filter_mask & log_level_all)    filter.push_back("all");
    if (filter_mask & log_level_info)   filter.push_back("info");
    if (filter_mask & log_level_warning)filter.push_back("warning");
    if (filter_mask & log_level_error)  filter.push_back("error");
    if (filter_mask & log_level_fatal)  filter.push_back("fatal");
    if (filter_mask & log_level_debug1) filter.push_back("debug1");
    if (filter_mask & log_level_debug2) filter.push_back("debug2");
    if (filter_mask & log_level_debug3) filter.push_back("debug3");
    if (filter_mask & log_level_debug4) filter.push_back("debug4");

    return filter;
  }

  eCAL::Publisher::Configuration::LayerPriorityVector transformLayerStrToEnum(const std::vector<std::string>& string_vector_)
  {
    eCAL::Publisher::Configuration::LayerPriorityVector layer_priority_vector;
    for (auto& layer_as_string : string_vector_)
    {
      if (layer_as_string == "shm") layer_priority_vector.push_back(eCAL::TLayer::tlayer_shm);
      if (layer_as_string == "udp") layer_priority_vector.push_back(eCAL::TLayer::tlayer_udp_mc);
      if (layer_as_string == "tcp") layer_priority_vector.push_back(eCAL::TLayer::tlayer_tcp);
    }

    return layer_priority_vector;
  }

  std::vector<std::string> transformLayerEnumToStr(const eCAL::Publisher::Configuration::LayerPriorityVector& enum_vector_)
  {
    std::vector<std::string> layer_priority_vector;
    for (auto& layer_as_enum : enum_vector_)
    {
      switch (layer_as_enum)
      {
        case eCAL::TLayer::tlayer_shm:
          layer_priority_vector.push_back("shm");
          break;
        case eCAL::TLayer::tlayer_udp_mc:
          layer_priority_vector.push_back("udp");
          break;
        case eCAL::TLayer::tlayer_tcp:
          layer_priority_vector.push_back("tcp");
          break;
        default:
          break;
      }
    }

    return layer_priority_vector;
  }
}

namespace YAML
{
  /*
       ___           _     __           __  _         
      / _ \___ ___ _(_)__ / /________ _/ /_(_)__  ___ 
     / , _/ -_) _ `/ (_-</ __/ __/ _ `/ __/ / _ \/ _ \
    /_/|_|\__/\_, /_/___/\__/_/  \_,_/\__/_/\___/_//_/
             /___/                                    
  */
  Node convert<eCAL::Registration::Layer::UDP::Configuration>::encode(const eCAL::Registration::Layer::UDP::Configuration& config_)
  {
    Node node;
    node["enable"] = config_.enable;
    node["port"]   = config_.port;
    return node;
  }

  bool convert<eCAL::Registration::Layer::UDP::Configuration>::decode(const Node& node_, eCAL::Registration::Layer::UDP::Configuration& config_)
  {
    AssignValue<bool>(config_.enable, node_, "enable");
    AssignValue<unsigned int>(config_.port, node_, "port");
    return true;
  }

  
  Node convert<eCAL::Registration::Layer::SHM::Configuration>::encode(const eCAL::Registration::Layer::SHM::Configuration& config_)
  {
    Node node;
    node["enable"]     = config_.enable;
    node["domain"]     = config_.domain;
    node["queue_size"] = config_.queue_size;
    return node;
  }

  bool convert<eCAL::Registration::Layer::SHM::Configuration>::decode(const Node& node_, eCAL::Registration::Layer::SHM::Configuration& config_)
  {
    AssignValue<bool>(config_.enable, node_, "enable");
    AssignValue<std::string>(config_.domain, node_, "domain");
    AssignValue<size_t>(config_.queue_size, node_, "queue_size");
    return true;
  }
  
  Node convert<eCAL::Registration::Layer::Configuration>::encode(const eCAL::Registration::Layer::Configuration& config_)
  {
    Node node;
    node["shm"] = config_.shm;
    node["udp"] = config_.udp;
    return node;
  }

  bool convert<eCAL::Registration::Layer::Configuration>::decode(const Node& node_, eCAL::Registration::Layer::Configuration& config_)
  {
    AssignValue<eCAL::Registration::Layer::SHM::Configuration>(config_.shm, node_, "shm");
    AssignValue<eCAL::Registration::Layer::UDP::Configuration>(config_.udp, node_, "udp");
    return true;
  }
  
  Node convert<eCAL::Registration::Configuration>::encode(const eCAL::Registration::Configuration& config_)
  {
    Node node;
    node["registration_timeout"] = config_.registration_timeout;
    node["registration_refresh"] = config_.registration_refresh;
    node["network_enabled"]      = config_.network_enabled;
    node["loopback"]             = config_.loopback;
    node["host_group_name"]      = config_.host_group_name;
    return node;
  }

  bool convert<eCAL::Registration::Configuration>::decode(const Node& node_, eCAL::Registration::Configuration& config_)
  {
    AssignValue<unsigned int>(config_.registration_timeout, node_, "registration_timeout");
    AssignValue<unsigned int>(config_.registration_refresh, node_, "registration_refresh");
    AssignValue<bool>(config_.network_enabled, node_, "network_enabled");
    AssignValue<bool>(config_.loopback, node_, "loopback");
    AssignValue<std::string>(config_.host_group_name, node_, "host_group_name");
    AssignValue<eCAL::Registration::Layer::Configuration>(config_.layer, node_, "layer");
    return true;
  }


  /*
       __  ___          _ __           _          
      /  |/  /__  ___  (_) /____  ____(_)__  ___ _
     / /|_/ / _ \/ _ \/ / __/ _ \/ __/ / _ \/ _ `/
    /_/  /_/\___/_//_/_/\__/\___/_/ /_/_//_/\_, / 
                                           /___/  
  */
  
  Node convert<eCAL::Monitoring::Configuration>::encode(const eCAL::Monitoring::Configuration& config_)
  {
    Node node;
    node["timeout"]    << config_.timeout;
    node["filter_excl"] = config_.filter_excl;
    node["filter_incl"] = config_.filter_incl;

    return node;
  }

  bool convert<eCAL::Monitoring::Configuration>::decode(const Node& node_, eCAL::Monitoring::Configuration& config_)
  {
    AssignValue<unsigned int>(config_.timeout, node_, "timeout");
    AssignValue<std::string>(config_.filter_excl, node_, "filter_excl");
    AssignValue<std::string>(config_.filter_incl, node_, "filter_incl");
    return true;
  }


  /*
     ______                                __  __                    
    /_  __/______ ____  ___ ___  ___  ____/ /_/ /  ___ ___ _____ ____
     / / / __/ _ `/ _ \(_-</ _ \/ _ \/ __/ __/ /__/ _ `/ // / -_) __/
    /_/ /_/  \_,_/_//_/___/ .__/\___/_/  \__/____/\_,_/\_, /\__/_/   
                         /_/                          /___/          
  */
  
  Node convert<eCAL::TransportLayer::SHM::Configuration>::encode(const eCAL::TransportLayer::SHM::Configuration& config_)
  {
    Node node;
    node["memfile_min_size_bytes"]  << config_.memfile_min_size_bytes;
    node["memfile_reserve_percent"] << config_.memfile_reserve_percent;
    return node;
  }

  bool convert<eCAL::TransportLayer::SHM::Configuration>::decode(const Node& node_, eCAL::TransportLayer::SHM::Configuration& config_)
  {
    AssignValue<unsigned int>(config_.memfile_min_size_bytes, node_, "memfile_min_size_bytes");
    AssignValue<unsigned int>(config_.memfile_reserve_percent, node_, "memfile_reserve_percent");
    return true;
  }
  
  Node convert<eCAL::TransportLayer::TCP::Configuration>::encode(const eCAL::TransportLayer::TCP::Configuration& config_)
  {
    Node node;
    node["number_executor_reader"] = config_.number_executor_reader;
    node["number_executor_writer"] = config_.number_executor_writer;
    node["max_reconnections"] = config_.max_reconnections;

    return node;
  }

  bool convert<eCAL::TransportLayer::TCP::Configuration>::decode(const Node& node_, eCAL::TransportLayer::TCP::Configuration& config_)
  {
    AssignValue<unsigned int>(config_.number_executor_reader, node_, "number_executor_reader");
    AssignValue<unsigned int>(config_.number_executor_writer, node_, "number_executor_writer");
    AssignValue<unsigned int>(config_.max_reconnections, node_, "max_reconnections");
    return true;
  }

  Node convert<eCAL::TransportLayer::UDP::Network::Configuration>::encode(const eCAL::TransportLayer::UDP::Network::Configuration& config_)
  {
    Node node;
    node["group"] = config_.group.Get();
    node["ttl"]   = config_.ttl;
    return node;
  }

  bool convert<eCAL::TransportLayer::UDP::Network::Configuration>::decode(const Node& node_, eCAL::TransportLayer::UDP::Network::Configuration& config_)
  {
    AssignValue<std::string>(config_.group, node_, "group");
    AssignValue<unsigned int>(config_.ttl, node_, "ttl");
    return true;
  }

  Node convert<eCAL::TransportLayer::UDP::Configuration>::encode(const eCAL::TransportLayer::UDP::Configuration& config_)
  {
    Node node;
    node["config_version"]      = config_.config_version == eCAL::Types::UdpConfigVersion::V1 ? "v1" : "v2";
    node["mode"]                = config_.mode == eCAL::Types::UDPMode::LOCAL ? "local" : "network";
    node["port"]                = config_.port;
    node["mask"]                = config_.mask.Get();
    node["send_buffer"]        << config_.send_buffer;
    node["receive_buffer"]     << config_.receive_buffer;
    node["join_all_interfaces"] = config_.join_all_interfaces;
    node["npcap_enabled"]       = config_.npcap_enabled;
    node["network"]             = config_.network;
    return node;
  }

  bool convert<eCAL::TransportLayer::UDP::Configuration>::decode(const Node& node_, eCAL::TransportLayer::UDP::Configuration& config_)
  {
    std::string temp_string = "v2";
    AssignValue<std::string>(temp_string, node_, "config_version");
    config_.config_version = temp_string == "v2" ? eCAL::Types::UdpConfigVersion::V2 : eCAL::Types::UdpConfigVersion::V1;
    temp_string = "local";
    AssignValue<std::string>(temp_string, node_, "mode");
    config_.mode = temp_string == "local" ? eCAL::Types::UDPMode::LOCAL : eCAL::Types::UDPMode::NETWORK;
    AssignValue<unsigned int>(config_.port, node_, "port");
    AssignValue<std::string>(config_.mask, node_, "mask");
    AssignValue<unsigned int>(config_.send_buffer, node_, "send_buffer");
    AssignValue<unsigned int>(config_.receive_buffer, node_, "receive_buffer");
    AssignValue<bool>(config_.join_all_interfaces, node_, "join_all_interfaces");
    AssignValue<bool>(config_.npcap_enabled, node_, "npcap_enabled");

    AssignValue<eCAL::TransportLayer::UDP::Network::Configuration>(config_.network, node_, "network");
    return true;
  }
  
  Node convert<eCAL::TransportLayer::Configuration>::encode(const eCAL::TransportLayer::Configuration& config_)
  {
    Node node;
    node["shm"] = config_.shm;
    node["udp"] = config_.udp;
    node["tcp"] = config_.tcp;

    return node;
  }

  bool convert<eCAL::TransportLayer::Configuration>::decode(const Node& node_, eCAL::TransportLayer::Configuration& config_)
  {
    AssignValue<eCAL::TransportLayer::SHM::Configuration>(config_.shm, node_, "shm");
    AssignValue<eCAL::TransportLayer::UDP::Configuration>(config_.udp, node_, "udp");
    AssignValue<eCAL::TransportLayer::TCP::Configuration>(config_.tcp, node_, "tcp");
    return true;
  }


  /*
       ___       __   ___     __          
      / _ \__ __/ /  / (_)__ / /  ___ ____
     / ___/ // / _ \/ / (_-</ _ \/ -_) __/
    /_/   \_,_/_.__/_/_/___/_//_/\__/_/   
  */
  
  Node convert<eCAL::Publisher::Layer::SHM::Configuration>::encode(const eCAL::Publisher::Layer::SHM::Configuration& config_)
  {
    Node node;
    node["enable"]                 = config_.enable;
    node["zero_copy_mode"]         = config_.zero_copy_mode;
    node["acknowledge_timeout_ms"] = config_.acknowledge_timeout_ms;
    node["memfile_buffer_count"]   = config_.memfile_buffer_count;
    return node;
  }

  bool convert<eCAL::Publisher::Layer::SHM::Configuration>::decode(const Node& node_, eCAL::Publisher::Layer::SHM::Configuration& config_)
  {
    AssignValue<bool>(config_.enable, node_, "enable");
    AssignValue<bool>(config_.zero_copy_mode, node_, "zero_copy_mode");
    AssignValue<unsigned int>(config_.acknowledge_timeout_ms, node_, "acknowledge_timeout_ms");
    AssignValue<unsigned int>(config_.memfile_buffer_count, node_, "memfile_buffer_count");
    return true;
  }
  
  Node convert<eCAL::Publisher::Layer::UDP::Configuration>::encode(const eCAL::Publisher::Layer::UDP::Configuration& config_)
  {
    Node node;
    node["enable"] = config_.enable;

    return node;
  }

  bool convert<eCAL::Publisher::Layer::UDP::Configuration>::decode(const Node& node_, eCAL::Publisher::Layer::UDP::Configuration& config_)
  {
    AssignValue<bool>(config_.enable, node_, "enable");
    return true;
  }
  
  Node convert<eCAL::Publisher::Layer::TCP::Configuration>::encode(const eCAL::Publisher::Layer::TCP::Configuration& config_)
  {
    Node node;
    node["enable"] = config_.enable;

    return node;
  }

  bool convert<eCAL::Publisher::Layer::TCP::Configuration>::decode(const Node& node_, eCAL::Publisher::Layer::TCP::Configuration& config_)
  {
    AssignValue<bool>(config_.enable, node_, "enable");
    return true;
  }
  
  Node convert<eCAL::Publisher::Layer::Configuration>::encode(const eCAL::Publisher::Layer::Configuration& config_)
  {
    Node node;
    node["shm"] = config_.shm;
    node["udp"] = config_.udp;
    node["tcp"] = config_.tcp;
    return node;
  }

  bool convert<eCAL::Publisher::Layer::Configuration>::decode(const Node& node_, eCAL::Publisher::Layer::Configuration& config_)
  {
    AssignValue<eCAL::Publisher::Layer::SHM::Configuration>(config_.shm, node_, "shm");
    AssignValue<eCAL::Publisher::Layer::UDP::Configuration>(config_.udp, node_, "udp");
    AssignValue<eCAL::Publisher::Layer::TCP::Configuration>(config_.tcp, node_, "tcp");
    return true;
  }
  
  Node convert<eCAL::Publisher::Configuration>::encode(const eCAL::Publisher::Configuration& config_)
  {
    Node node;
    node["share_topic_description"] = config_.share_topic_description;
    node["share_topic_type"]        = config_.share_topic_type;
    node["layer"]                   = config_.layer;
    node["priority_local"]          = transformLayerEnumToStr(config_.layer_priority_local);
    node["priority_network"]        = transformLayerEnumToStr(config_.layer_priority_remote);
    return node;
  }

  bool convert<eCAL::Publisher::Configuration>::decode(const Node& node_, eCAL::Publisher::Configuration& config_)
  {
    AssignValue<bool>(config_.share_topic_description, node_, "share_topic_description");
    AssignValue<bool>(config_.share_topic_type, node_, "share_topic_type");
    
    std::vector<std::string> tmp;
    AssignValue<std::vector<std::string>>(tmp, node_, "priority_local");
    config_.layer_priority_local = transformLayerStrToEnum(tmp);
    tmp.clear();
    AssignValue<std::vector<std::string>>(tmp, node_, "priority_network");
    config_.layer_priority_remote = transformLayerStrToEnum(tmp);

    AssignValue<eCAL::Publisher::Layer::Configuration>(config_.layer, node_, "layer");    
    return true;
  }


  /*
       ____     __               _ __          
      / __/_ __/ /  ___ ________(_) /  ___ ____
     _\ \/ // / _ \(_-</ __/ __/ / _ \/ -_) __/
    /___/\_,_/_.__/___/\__/_/ /_/_.__/\__/_/   
  */
  
  Node convert<eCAL::Subscriber::Layer::SHM::Configuration>::encode(const eCAL::Subscriber::Layer::SHM::Configuration& config_)
  {
    Node node;
    node["enable"] = config_.enable;
    return node;
  }

  bool convert<eCAL::Subscriber::Layer::SHM::Configuration>::decode(const Node& node_, eCAL::Subscriber::Layer::SHM::Configuration& config_)
  {
    AssignValue<bool>(config_.enable, node_, "enable");
    return true;
  }
  
  Node convert<eCAL::Subscriber::Layer::UDP::Configuration>::encode(const eCAL::Subscriber::Layer::UDP::Configuration& config_)
  {
    Node node;
    node["enable"] = config_.enable;
    return node;
  }

  bool convert<eCAL::Subscriber::Layer::UDP::Configuration>::decode(const Node& node_, eCAL::Subscriber::Layer::UDP::Configuration& config_)
  {
    AssignValue<bool>(config_.enable, node_, "enable");
    return true;
  }

  Node convert<eCAL::Subscriber::Layer::TCP::Configuration>::encode(const eCAL::Subscriber::Layer::TCP::Configuration& config_)
  {
    Node node;
    node["enable"] = config_.enable;
    return node;
  }

  bool convert<eCAL::Subscriber::Layer::TCP::Configuration>::decode(const Node& node_, eCAL::Subscriber::Layer::TCP::Configuration& config_)
  {
    AssignValue<bool>(config_.enable, node_, "enable");
    return true;
  }

  Node convert<eCAL::Subscriber::Layer::Configuration>::encode(const eCAL::Subscriber::Layer::Configuration& config_)
  {
    Node node;
    node["shm"] = config_.shm;
    node["udp"] = config_.udp;
    node["tcp"] = config_.tcp;
    return node;
  }

  bool convert<eCAL::Subscriber::Layer::Configuration>::decode(const Node& node_, eCAL::Subscriber::Layer::Configuration& config_)
  {
    AssignValue<eCAL::Subscriber::Layer::SHM::Configuration>(config_.shm, node_, "shm");
    AssignValue<eCAL::Subscriber::Layer::UDP::Configuration>(config_.udp, node_, "udp");
    AssignValue<eCAL::Subscriber::Layer::TCP::Configuration>(config_.tcp, node_, "tcp");
    return true;
  }

  Node convert<eCAL::Subscriber::Configuration>::encode(const eCAL::Subscriber::Configuration& config_)
  {
    Node node;
    node["layer"] = config_.layer;
    node["drop_out_of_order_message"] = config_.drop_out_of_order_messages;
    return node;
  }

  bool convert<eCAL::Subscriber::Configuration>::decode(const Node& node_, eCAL::Subscriber::Configuration& config_)
  {
    AssignValue<eCAL::Subscriber::Layer::Configuration>(config_.layer, node_, "layer");
    AssignValue<bool>(config_.drop_out_of_order_messages, node_, "dropt_out_of_order_messages");
    return true;
  }


  /*
     _______          
    /_  __(_)_ _  ___ 
     / / / /  ' \/ -_)
    /_/ /_/_/_/_/\__/ 
  */
  
  Node convert<eCAL::Time::Configuration>::encode(const eCAL::Time::Configuration& config_)
  {
    Node node;
    node["replay"] = config_.timesync_module_replay;
    node["rt"]     = config_.timesync_module_rt;

    return node;
  }

  bool convert<eCAL::Time::Configuration>::decode(const Node& node_, eCAL::Time::Configuration& config_)
  {
    AssignValue<std::string>(config_.timesync_module_replay, node_, "replay");
    AssignValue<std::string>(config_.timesync_module_rt, node_, "rt");
    return true;
  }


  /*
       ____             _        
      / __/__ _____  __(_)______ 
     _\ \/ -_) __/ |/ / / __/ -_)
    /___/\__/_/  |___/_/\__/\__/                             
  */
  
  Node convert<eCAL::Service::Configuration>::encode(const eCAL::Service::Configuration& config_)
  {
    Node node;
    node["protocol_v0"] = config_.protocol_v0;
    node["protocol_v1"] = config_.protocol_v1;

    return node;
  }

  bool convert<eCAL::Service::Configuration>::decode(const Node& node_, eCAL::Service::Configuration& config_)
  {
    AssignValue<bool>(config_.protocol_v0, node_, "protocol_v0");
    AssignValue<bool>(config_.protocol_v1, node_, "protocol_v1");
    return true;
  }


  /*
       ___             ___          __  _         
      / _ | ___  ___  / (_)______ _/ /_(_)__  ___ 
     / __ |/ _ \/ _ \/ / / __/ _ `/ __/ / _ \/ _ \
    /_/ |_/ .__/ .__/_/_/\__/\_,_/\__/_/\___/_//_/
         /_/  /_/                                 
  */
  
  Node convert<eCAL::Application::Startup::Configuration>::encode(const eCAL::Application::Startup::Configuration& config_)
  {
    Node node;
    node["emulator"] = config_.terminal_emulator;

    return node;
  }

  bool convert<eCAL::Application::Startup::Configuration>::decode(const Node& node_, eCAL::Application::Startup::Configuration& config_)
  {
    AssignValue<std::string>(config_.terminal_emulator, node_, "emulator");

    return true;
  }
  
  Node convert<eCAL::Application::Sys::Configuration>::encode(const eCAL::Application::Sys::Configuration& config_)
  {
    Node node;
    node["filter_excl"] = config_.filter_excl;

    return node;
  }

  bool convert<eCAL::Application::Sys::Configuration>::decode(const Node& node_, eCAL::Application::Sys::Configuration& config_)
  {
    AssignValue<std::string>(config_.filter_excl, node_, "filter_excl");
    return true;
  }
  
  Node convert<eCAL::Application::Configuration>::encode(const eCAL::Application::Configuration& config_)
  {
    Node node;
    node["terminal"] = config_.startup;
    node["sys"]      = config_.sys;
    
    return node;
  }

  bool convert<eCAL::Application::Configuration>::decode(const Node& node_, eCAL::Application::Configuration& config_)
  {
    AssignValue<eCAL::Application::Startup::Configuration>(config_.startup, node_, "terminal");
    AssignValue<eCAL::Application::Sys::Configuration>(config_.sys, node_, "sys");
    return true;
  }

  /*
       __                  _          
      / /  ___  ___ ____ _(_)__  ___ _
     / /__/ _ \/ _ `/ _ `/ / _ \/ _ `/
    /____/\___/\_, /\_, /_/_//_/\_, / 
              /___//___/       /___/  
  */
  
  Node convert<eCAL::Logging::Sinks::UDP::Configuration>::encode(const eCAL::Logging::Sinks::UDP::Configuration& config_)
  {
    Node node;
    node["enable"] = config_.enable;
    node["port"]   = config_.port;
    node["level"]  = LogLevelToVector(config_.filter_log_udp);
    return node;
  }

  bool convert<eCAL::Logging::Sinks::UDP::Configuration>::decode(const Node& node_, eCAL::Logging::Sinks::UDP::Configuration& config_)
  {
    AssignValue<bool>(config_.enable, node_, "enable");
    AssignValue<unsigned int>(config_.port, node_, "port");

    std::vector<std::string> tmp;
    AssignValue<std::vector<std::string>>(tmp, node_, "level");
    config_.filter_log_udp = ParseLogLevel(tmp);
    return true;
  }
  
  Node convert<eCAL::Logging::Sinks::Console::Configuration>::encode(const eCAL::Logging::Sinks::Console::Configuration& config_)
  {
    Node node;
    node["enable"] = config_.enable;
    node["level"] = LogLevelToVector(config_.filter_log_con);
    return node;
  }

  bool convert<eCAL::Logging::Sinks::Console::Configuration>::decode(const Node& node_, eCAL::Logging::Sinks::Console::Configuration& config_)
  {
    AssignValue<bool>(config_.enable, node_, "enable");
    std::vector<std::string> tmp;
    AssignValue<std::vector<std::string>>(tmp, node_, "level");
    config_.filter_log_con = ParseLogLevel(tmp);
    return true;
  }
 
  Node convert<eCAL::Logging::Sinks::File::Configuration>::encode(const eCAL::Logging::Sinks::File::Configuration& config_)
  {
    Node node;
    node["enable"] = config_.enable;
    node["path"]   = config_.path;
    node["level"] = LogLevelToVector(config_.filter_log_file);
    return node;
  }

  bool convert<eCAL::Logging::Sinks::File::Configuration>::decode(const Node& node_, eCAL::Logging::Sinks::File::Configuration& config_)
  {
    AssignValue<bool>(config_.enable, node_, "enable");
    AssignValue<std::string>(config_.path, node_, "path");
    
    std::vector<std::string> tmp;
    AssignValue<std::vector<std::string>>(tmp, node_, "level");
    config_.filter_log_file = ParseLogLevel(tmp);
    return true;
  }
  
  Node convert<eCAL::Logging::Sinks::Configuration>::encode(const eCAL::Logging::Sinks::Configuration& config_)
  {
    Node node;
    node["console"] = config_.console;
    node["file"]    = config_.file;
    node["udp"]     = config_.udp;
    return node;
  }

  bool convert<eCAL::Logging::Sinks::Configuration>::decode(const Node& node_, eCAL::Logging::Sinks::Configuration& config_)
  {
    AssignValue<eCAL::Logging::Sinks::Console::Configuration>(config_.console, node_, "console");
    AssignValue<eCAL::Logging::Sinks::File::Configuration>(config_.file, node_, "file");
    AssignValue<eCAL::Logging::Sinks::UDP::Configuration>(config_.udp, node_, "udp");
    return true;
  }

  Node convert<eCAL::Logging::Configuration>::encode(const eCAL::Logging::Configuration& config_)
  {
    Node node;
    node["sinks"] = config_.sinks;
    return node;
  }

  bool convert<eCAL::Logging::Configuration>::decode(const Node& node_, eCAL::Logging::Configuration& config_)
  {
    AssignValue<eCAL::Logging::Sinks::Configuration>(config_.sinks, node_, "sinks");
    return true;
  }


  /*
       __  ___     _                      ____                    __  _         
      /  |/  /__ _(_)__    _______  ___  / _(_)__ ___ _________ _/ /_(_)__  ___ 
     / /|_/ / _ `/ / _ \  / __/ _ \/ _ \/ _/ / _ `/ // / __/ _ `/ __/ / _ \/ _ \
    /_/  /_/\_,_/_/_//_/  \__/\___/_//_/_//_/\_, /\_,_/_/  \_,_/\__/_/\___/_//_/
                                            /___/                               
  */
 
  Node convert<eCAL::Configuration>::encode(const eCAL::Configuration& config_)
  {
    Node node;
    node["publisher"]    = config_.publisher;
    node["subscriber"]   = config_.subscriber;
    node["registration"] = config_.registration;
    node["monitoring"]   = config_.monitoring;
    node["time"]         = config_.timesync;
    node["service"]      = config_.service;
    node["application"]  = config_.application;
    node["logging"]      = config_.logging;
    return node;
  }

  bool convert<eCAL::Configuration>::decode(const Node& node_, eCAL::Configuration& config_)
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
}