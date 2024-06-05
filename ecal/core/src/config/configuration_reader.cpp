/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/

/**
 * @brief  Global eCAL configuration interface
**/

#include "configuration_reader.h"

#include <yaml-cpp/yaml.h>

namespace YAML
{
  // 3rd layer configuration objects
  // application objects
  template<>
  struct convert<eCAL::Application::Startup::Configuration>
  {
    static Node encode(const eCAL::Application::Startup::Configuration&)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node& node_, eCAL::Application::Startup::Configuration& config_)
    {
      config_.terminal_emulator = node_["terminal_emulator"].as<std::string>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::Application::Sys::Configuration>
  {
    static Node encode(const eCAL::Application::Sys::Configuration&)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node& node_, eCAL::Application::Sys::Configuration& config_)
    {
      config_.filter_excl = node_["filter_excl"].as<std::string>();
      return true;
    }
  };

  // monitoring objects
  template<>
  struct convert<eCAL::Monitoring::SHM::Configuration>
  {
    static Node encode(const eCAL::Monitoring::SHM::Configuration&)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node& node_, eCAL::Monitoring::SHM::Configuration& config_)
    {
      config_.shm_monitoring_domain = node_["domain"].as<std::string>();
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

  template<>
  struct convert<eCAL::Monitoring::Types::Mode>
  {
    static Node encode(const eCAL::Monitoring::Types::Mode&)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node&, eCAL::Monitoring::Types::Mode& config_)
    {
      // TODO PG: Change this here after refactoring monitoring mode to a list
      config_ = eCAL::Monitoring::Types::Mode::shm_monitoring;
      return true;
    }

    // TODO PG: just a trial, remove if not necessary anymore
    static bool decode(const Node&, eCAL::Monitoring::Types::Mode_Filter& config_)
    {
      // TODO PG: Change this here after refactoring monitoring mode to a list
      config_ = static_cast<eCAL::Monitoring::Types::Mode_Filter>(eCAL::Monitoring::Types::Mode::shm_monitoring);
      return true;
    }
  };

  // publisher configuration objects
  template<>
  struct convert<eCAL::Publisher::SHM::Configuration>
  {
    static Node encode(const eCAL::Publisher::SHM::Configuration&)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node& node_, eCAL::Publisher::SHM::Configuration& config_)
    {
      config_.enable                  = node_["enable"].as<bool>();
      config_.zero_copy_mode          = node_["zero_copy_mode"].as<bool>();
      config_.acknowledge_timeout_ms  = node_["acknowledge_timeout_ms"].as<unsigned int>();
      config_.memfile_min_size_bytes  = node_["memfile_min_size_bytes"].as<unsigned int>();
      config_.memfile_reserve_percent = node_["memfile_reserve_percent"].as<unsigned int>();
      config_.memfile_buffer_count    = node_["memfile_buffer_count"].as<unsigned int>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::Publisher::UDP::Configuration>
  {
    static Node encode(const eCAL::Publisher::UDP::Configuration&)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node& node_, eCAL::Publisher::UDP::Configuration& config_)
    {
      config_.enable            = node_["enable"].as<bool>();
      config_.loopback          = node_["loopback"].as<bool>();
      config_.sndbuf_size_bytes = node_["sndbuf_size_bytes"].as<unsigned int>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::Publisher::TCP::Configuration>
  {
    static Node encode(const eCAL::Publisher::TCP::Configuration&)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node& node_, eCAL::Publisher::TCP::Configuration& config_)
    {
      config_.enable = node_["enable"].as<bool>();
      return true;
    }
  };

  // subscriber configuration objects
  template<>
  struct convert<eCAL::Subscriber::SHM::Configuration>
  {
    static Node encode(const eCAL::Subscriber::SHM::Configuration&)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node& node_, eCAL::Subscriber::SHM::Configuration& config_)
    {
      config_.enable = node_["enable"].as<bool>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::Subscriber::UDP::Configuration>
  {
    static Node encode(const eCAL::Subscriber::UDP::Configuration&)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node& node_, eCAL::Subscriber::UDP::Configuration& config_)
    {
      config_.enable = node_["enable"].as<bool>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::Subscriber::TCP::Configuration>
  {
    static Node encode(const eCAL::Subscriber::TCP::Configuration&)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node& node_, eCAL::Subscriber::TCP::Configuration& config_)
    {
      config_.enable = node_["enable"].as<bool>();
      return true;
    }
  };

  // transport layer configuration objects
  template<>
  struct convert<eCAL::TransportLayer::TCPPubSub::Configuration>
  {
    static Node encode(const eCAL::TransportLayer::TCPPubSub::Configuration&)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node& node_, eCAL::TransportLayer::TCPPubSub::Configuration& config_)
    {
      config_.num_executor_reader = node_["number_executor_reader"].as<size_t>();
      config_.num_executor_writer = node_["number_executor_writer"].as<size_t>();
      config_.max_reconnections   = node_["max_reconnections"].as<size_t>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::TransportLayer::SHM::Configuration>
  {
    static Node encode(const eCAL::TransportLayer::SHM::Configuration&)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node& node_, eCAL::TransportLayer::SHM::Configuration& config_)
    {
      config_.host_group_name = node_["host_group_name"].as<std::string>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::TransportLayer::UDPMC::Configuration>
  {
    static Node encode(const eCAL::TransportLayer::UDPMC::Configuration&)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node& node_, eCAL::TransportLayer::UDPMC::Configuration& config_)
    {
      config_.config_version      = node_["config_version"].as<std::string>() == "v1" ? eCAL::Types::UdpConfigVersion::V1 : eCAL::Types::UdpConfigVersion::V2;
      config_.group               = node_["group"].as<std::string>();
      config_.mask                = node_["mask"].as<std::string>();
      config_.port                = node_["port"].as<unsigned int>();
      config_.ttl                 = node_["ttl"].as<unsigned int>();
      config_.sndbuf              = node_["send_buffer"].as<unsigned int>();
      config_.recbuf              = node_["receive_buffer"].as<unsigned int>();
      config_.join_all_interfaces = node_["join_all_interfaces"].as<bool>();
      config_.npcap_enabled       = node_["npcap_enabled"].as<bool>();
      return true;
    }
  };

  // 2nd layer configuration objects
  template<>
  struct convert<eCAL::Application::Configuration>
  {
    static Node encode(const eCAL::Application::Configuration&)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node& node_, eCAL::Application::Configuration& config_)
    {
      config_.startup = node_["startup"].as<eCAL::Application::Startup::Configuration>();
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
    static Node encode(const eCAL::Monitoring::Configuration&)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node& node_, eCAL::Monitoring::Configuration& config_)
    {
      config_.monitoring_mode = node_["mode"].as<eCAL::Monitoring::Types::Mode_Filter>();
      config_.monitoring_timeout = node_["timeout"].as<unsigned int>();
      config_.network_monitoring = node_["network"].as<bool>();
      config_.filter_excl = node_["filter_excl"].as<std::string>();
      config_.filter_incl = node_["filter_incl"].as<std::string>();
      config_.udp_options = node_["udp"].as<eCAL::Monitoring::UDP::Configuration>();
      config_.shm_options = node_["tcp"].as<eCAL::Monitoring::SHM::Configuration>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::Publisher::Configuration>
  {
    static Node encode(const eCAL::Publisher::Configuration&)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node& node_, eCAL::Publisher::Configuration& config_)
    {
      config_.share_topic_description = node_["share_topic_description"].as<bool>();
      config_.share_topic_type        = node_["share_topic_type"].as<bool>();
      config_.shm                     = node_["shm"].as<eCAL::Publisher::SHM::Configuration>();
      config_.udp                     = node_["udp"].as<eCAL::Publisher::UDP::Configuration>();
      config_.tcp                     = node_["tcp"].as<eCAL::Publisher::TCP::Configuration>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::Registration::Configuration>
  {
    static Node encode(const eCAL::Registration::Configuration&)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node& node_, eCAL::Registration::Configuration& config_)
    {
      unsigned int reg_timeout = node_["registration_timeout"].as<unsigned int>();
      unsigned int reg_refresh = node_["registration_refresh"].as<unsigned int>();

      config_             = {reg_timeout, reg_refresh};
      config_.share_ttype = node_["share_ttype"].as<bool>();
      config_.share_tdesc = node_["share_tdesc"].as<bool>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::Service::Configuration>
  {
    static Node encode(const eCAL::Service::Configuration&)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node& node_, eCAL::Service::Configuration& config_)
    {
      config_.protocol_v0 = node_["protocol_v0"].as<bool>();
      config_.protocol_v1 = node_["protocol_v1"].as<bool>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::Subscriber::Configuration>
  {
    static Node encode(const eCAL::Subscriber::Configuration&)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node& node_, eCAL::Subscriber::Configuration& config_)
    {
      config_.shm = node_["shm"].as<eCAL::Subscriber::SHM::Configuration>();
      config_.tcp = node_["tcp"].as<eCAL::Subscriber::TCP::Configuration>();
      config_.udp = node_["udp"].as<eCAL::Subscriber::UDP::Configuration>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::Time::Configuration>
  {
    static Node encode(const eCAL::Time::Configuration&)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node& node_, eCAL::Time::Configuration& config_)
    {
      config_.timesync_module_replay = node_["replay"].as<std::string>();
      config_.timesync_module_rt     = node_["rt"].as<std::string>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::TransportLayer::Configuration>
  {
    static Node encode(const eCAL::TransportLayer::Configuration&)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node& node_, eCAL::TransportLayer::Configuration& config_)
    {
      config_.network_enabled            = node_["network_enable"].as<bool>();
      config_.drop_out_of_order_messages = node_["drop_out_of_order_messages"].as<bool>();
      config_.mc_options                 = node_["udp_mc"].as<eCAL::TransportLayer::UDPMC::Configuration>();
      config_.tcp_options                = node_["tcppubsub"].as<eCAL::TransportLayer::TCPPubSub::Configuration>();
      config_.shm_options                = node_["shm"].as<eCAL::TransportLayer::SHM::Configuration>();
      return true;
    }
  };

  template<>
  struct convert<eCAL::Cli::Configuration>
  {
    static Node encode(const eCAL::Cli::Configuration&)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node&, eCAL::Cli::Configuration& config_)
    {
      config_.config_keys      = {};
      config_.config_keys_map  = {};
      config_.dump_config      = false;
      config_.specified_config = {};
      return true;
    }
  };


  // Main configuration object
  template<>
  struct convert<eCAL::Configuration>
  {
    static Node encode(const eCAL::Configuration&)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node& node_, eCAL::Configuration& config_)
    {
      config_.application            = node_["application"].as<eCAL::Application::Configuration>();
      config_.logging                = node_["logging"].as<eCAL::Logging::Configuration>();
      config_.monitoring             = node_["monitoring"].as<eCAL::Monitoring::Configuration>();
      config_.publisher              = node_["publisher"].as<eCAL::Publisher::Configuration>();
      config_.registration           = node_["registration"].as<eCAL::Registration::Configuration>();
      config_.service                = node_["service"].as<eCAL::Service::Configuration>();
      config_.subscriber             = node_["subscriber"].as<eCAL::Subscriber::Configuration>();
      config_.timesync               = node_["time"].as<eCAL::Time::Configuration>();
      config_.transport_layer        = node_["transport_layer"].as<eCAL::TransportLayer::Configuration>();
      config_.command_line_arguments = node_["user_arguments"].as<eCAL::Cli::Configuration>();
      return true;
    }
  };  
}

namespace eCAL
{
  namespace Config
  {
    eCAL::Configuration parseYaml(std::string& filename_)
    {
      YAML::Node config = YAML::LoadFile(filename_);

      return config.as<eCAL::Configuration>();
    }
  }
}