/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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
 * @brief  Utility class for parsing cmd line arguments into eCAL useful structures.
**/

#ifndef CONFIGURATION_TO_YAML_H
#define CONFIGURATION_TO_YAML_H

#include <ecal/config/configuration.h>

#ifndef YAML_CPP_STATIC_DEFINE 
#define YAML_CPP_STATIC_DEFINE 
#endif
#include <yaml-cpp/yaml.h>

namespace YAML
{
  // Utility function to be used also in other files
  template<typename AS, typename MEM>
  inline void AssignValue(MEM& member, const YAML::Node& node_, const char* key)
  {
    if (node_[key])
      member = node_[key].as<AS>();
  }

  /*
       ___           _     __           __  _         
      / _ \___ ___ _(_)__ / /________ _/ /_(_)__  ___ 
     / , _/ -_) _ `/ (_-</ __/ __/ _ `/ __/ / _ \/ _ \
    /_/|_|\__/\_, /_/___/\__/_/  \_,_/\__/_/\___/_//_/
             /___/                                    
  */

  template<>
  struct convert<eCAL::Registration::Network::UDP::Configuration>
  {
    static Node encode(const eCAL::Registration::Network::UDP::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Registration::Network::UDP::Configuration& config_);
  };

  template<>
  struct convert<eCAL::Registration::Network::Configuration>
  {
    static Node encode(const eCAL::Registration::Network::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Registration::Network::Configuration& config_);
  };

  template<>
  struct convert<eCAL::Registration::Local::SHM::Configuration>
  {
    static Node encode(const eCAL::Registration::Local::SHM::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Registration::Local::SHM::Configuration& config_);
  };

  template<>
  struct convert<eCAL::Registration::Local::UDP::Configuration>
  {
    static Node encode(const eCAL::Registration::Local::UDP::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Registration::Local::UDP::Configuration& config_);
  };

  template<>
  struct convert<eCAL::Registration::Local::Configuration>
  {
    static Node encode(const eCAL::Registration::Local::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Registration::Local::Configuration& config_);
  };

  template<>
  struct convert<eCAL::Registration::Configuration>
  {
    static Node encode(const eCAL::Registration::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Registration::Configuration& config_);
  };


  /*
     ______                                __  __                    
    /_  __/______ ____  ___ ___  ___  ____/ /_/ /  ___ ___ _____ ____
     / / / __/ _ `/ _ \(_-</ _ \/ _ \/ __/ __/ /__/ _ `/ // / -_) __/
    /_/ /_/  \_,_/_//_/___/ .__/\___/_/  \__/____/\_,_/\_, /\__/_/   
                         /_/                          /___/          
  */
  template<>
  struct convert<eCAL::TransportLayer::TCP::Configuration>
  {
    static Node encode(const eCAL::TransportLayer::TCP::Configuration& config_);

    static bool decode(const Node& node_, eCAL::TransportLayer::TCP::Configuration& config_);
  };

  template<>
  struct convert<eCAL::TransportLayer::UDP::MulticastConfiguration>
  {
    static Node encode(const eCAL::TransportLayer::UDP::MulticastConfiguration& config_);

    static bool decode(const Node& node_, eCAL::TransportLayer::UDP::MulticastConfiguration& config_);
  };

  template<>
  struct convert<eCAL::TransportLayer::UDP::Configuration>
  {
    static Node encode(const eCAL::TransportLayer::UDP::Configuration& config_);

    static bool decode(const Node& node_, eCAL::TransportLayer::UDP::Configuration& config_);
  };

  template<>
  struct convert<eCAL::TransportLayer::Configuration>
  {
    static Node encode(const eCAL::TransportLayer::Configuration& config_);

    static bool decode(const Node& node_, eCAL::TransportLayer::Configuration& config_);
  };


  /*
       ___       __   ___     __          
      / _ \__ __/ /  / (_)__ / /  ___ ____
     / ___/ // / _ \/ / (_-</ _ \/ -_) __/
    /_/   \_,_/_.__/_/_/___/_//_/\__/_/   
  */
  template<>
  struct convert<eCAL::Publisher::Layer::SHM::Configuration>
  {
    static Node encode(const eCAL::Publisher::Layer::SHM::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Publisher::Layer::SHM::Configuration& config_);
  };

  template<>
  struct convert<eCAL::Publisher::Layer::UDP::Configuration>
  {
    static Node encode(const eCAL::Publisher::Layer::UDP::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Publisher::Layer::UDP::Configuration& config_);
  };

  template<>
  struct convert<eCAL::Publisher::Layer::TCP::Configuration>
  {
    static Node encode(const eCAL::Publisher::Layer::TCP::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Publisher::Layer::TCP::Configuration& config_);
  };

  template<>
  struct convert<eCAL::Publisher::Layer::Configuration>
  {
    static Node encode(const eCAL::Publisher::Layer::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Publisher::Layer::Configuration& config_);
  };

  template<>
  struct convert<eCAL::Publisher::Configuration>
  {
    static Node encode(const eCAL::Publisher::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Publisher::Configuration& config_);
  };


  /*
       ____     __               _ __          
      / __/_ __/ /  ___ ________(_) /  ___ ____
     _\ \/ // / _ \(_-</ __/ __/ / _ \/ -_) __/
    /___/\_,_/_.__/___/\__/_/ /_/_.__/\__/_/   
  */
  template<>
  struct convert<eCAL::Subscriber::Layer::SHM::Configuration>
  {
    static Node encode(const eCAL::Subscriber::Layer::SHM::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Subscriber::Layer::SHM::Configuration& config_);
  };

  template<>
  struct convert<eCAL::Subscriber::Layer::UDP::Configuration>
  {
    static Node encode(const eCAL::Subscriber::Layer::UDP::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Subscriber::Layer::UDP::Configuration& config_);
  };

  template<>
  struct convert<eCAL::Subscriber::Layer::TCP::Configuration>
  {
    static Node encode(const eCAL::Subscriber::Layer::TCP::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Subscriber::Layer::TCP::Configuration& config_);
  };

  template<>
  struct convert<eCAL::Subscriber::Layer::Configuration>
  {
    static Node encode(const eCAL::Subscriber::Layer::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Subscriber::Layer::Configuration& config_);
  };

  template<>
  struct convert<eCAL::Subscriber::Configuration>
  {
    static Node encode(const eCAL::Subscriber::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Subscriber::Configuration& config_);
  };


  /*
     _______          
    /_  __(_)_ _  ___ 
     / / / /  ' \/ -_)
    /_/ /_/_/_/_/\__/ 
  */
  template<>
  struct convert<eCAL::Time::Configuration>
  {
    static Node encode(const eCAL::Time::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Time::Configuration& config_);
  };


  /*
       ___             ___          __  _         
      / _ | ___  ___  / (_)______ _/ /_(_)__  ___ 
     / __ |/ _ \/ _ \/ / / __/ _ `/ __/ / _ \/ _ \
    /_/ |_/ .__/ .__/_/_/\__/\_,_/\__/_/\___/_//_/
         /_/  /_/                                 
  */
  template<>
  struct convert<eCAL::Application::Startup::Configuration>
  { 
    static Node encode(const eCAL::Application::Startup::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Application::Startup::Configuration& config_);
  };

  template<>
  struct convert<eCAL::Application::Sys::Configuration>
  {
    static Node encode(const eCAL::Application::Sys::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Application::Sys::Configuration& config_);
  };


  template<>
  struct convert<eCAL::Application::Configuration>
  {
    static Node encode(const eCAL::Application::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Application::Configuration& config_);
  };

  /*
       __                  _          
      / /  ___  ___ ____ _(_)__  ___ _
     / /__/ _ \/ _ `/ _ `/ / _ \/ _ `/
    /____/\___/\_, /\_, /_/_//_/\_, / 
              /___//___/       /___/  
  */
  template<>
  struct convert<eCAL::Logging::Receiver::UDP::Configuration>
  {
    static Node encode(const eCAL::Logging::Receiver::UDP::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Logging::Receiver::UDP::Configuration& config_);
  };

  template<>
  struct convert<eCAL::Logging::Receiver::Configuration>
  {
    static Node encode(const eCAL::Logging::Receiver::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Logging::Receiver::Configuration& config_);
  };

  template<>
  struct convert<eCAL::Logging::Provider::File::Configuration>
  {
    static Node encode(const eCAL::Logging::Provider::File::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Logging::Provider::File::Configuration& config_);
  };

  template<>
  struct convert<eCAL::Logging::Provider::UDP::Configuration>
  {
    static Node encode(const eCAL::Logging::Provider::UDP::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Logging::Provider::UDP::Configuration& config_);
  };

  template<>
  struct convert<eCAL::Logging::Provider::Sink>
  {
    static Node encode(const eCAL::Logging::Provider::Sink& config_);

    static bool decode(const Node& node_, eCAL::Logging::Provider::Sink& config_);
  };

  template<>
  struct convert<eCAL::Logging::Provider::Configuration>
  {
    static Node encode(const eCAL::Logging::Provider::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Logging::Provider::Configuration& config_);
  };

  template<>
  struct convert<eCAL::Logging::Configuration>
  {
    static Node encode(const eCAL::Logging::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Logging::Configuration& config_);
  };


  /*
       __  ___     _                      ____                    __  _         
      /  |/  /__ _(_)__    _______  ___  / _(_)__ ___ _________ _/ /_(_)__  ___ 
     / /|_/ / _ `/ / _ \  / __/ _ \/ _ \/ _/ / _ `/ // / __/ _ `/ __/ / _ \/ _ \
    /_/  /_/\_,_/_/_//_/  \__/\___/_//_/_//_/\_, /\_,_/_/  \_,_/\__/_/\___/_//_/
                                            /___/                               
  */
  template<>
  struct convert<eCAL::Configuration>
  {
    static Node encode(const eCAL::Configuration& config_);

    static bool decode(const Node& node_, eCAL::Configuration& config_);
  };  
}

#endif // CONFIGURATION_TO_YAML_H