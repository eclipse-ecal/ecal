/* ========================= eCAL LICENSE =================================
 *
 * Copyright 2026 AUMOVIO and subsidiaries. All rights reserved.
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

#include "launch_configuration.h"

namespace
{
    void fillPubSubNode(eCAL::Config::LaunchConfiguration& launch_configuration_, const YAML::Node& pubsub_node_)
    {
      if (!pubsub_node_ || !pubsub_node_.IsMap()) return;

      // handle publisher topics remap
      auto publisher_topics = pubsub_node_["publisher"];
      if (publisher_topics && publisher_topics.IsMap())
      {
        for (auto it = publisher_topics.begin(); it != publisher_topics.end(); it++)
        {
          auto from = it->first.as<std::string>();
          auto to   = it->second.as<std::string>();
          launch_configuration_.publisher_topics[from] = to;
        }
      }

      // handle subscriber topics remap
      auto subscriber_topics = pubsub_node_["subscriber"];
      if (subscriber_topics && subscriber_topics.IsMap())
      {
        for (auto it = subscriber_topics.begin(); it != subscriber_topics.end(); it++)
        {
          auto from = it->first.as<std::string>();
          auto to   = it->second.as<std::string>();
          launch_configuration_.subscriber_topics[from] = to;
        }
      }
    }

		void fillServiceNode(eCAL::Config::LaunchConfiguration& launch_configuration_, const YAML::Node& service_node_)
		{
			if (!service_node_ || !service_node_.IsMap()) return;

			// handle client calls remap
			auto client_calls = service_node_["client"];
			if (client_calls && client_calls.IsMap())
			{
				for (auto it = client_calls.begin(); it != client_calls.end(); it++)
				{
					auto from = it->first.as<std::string>();
					auto to   = it->second.as<std::string>();
					launch_configuration_.client_calls[from] = to;
				}
			}

			// handle server methods remap
			auto server_methods = service_node_["server"];
			if (server_methods && server_methods.IsMap())
			{
				for (auto it = server_methods.begin(); it != server_methods.end(); it++)
				{
					auto from = it->first.as<std::string>();
					auto to   = it->second.as<std::string>();
					launch_configuration_.server_methods[from] = to;
				}
			}
		}
}

namespace eCAL
{
  namespace Config
  {
    // Create a launch configuration from a launch yaml node
    LaunchConfiguration CreateLaunchConfiguration(const YAML::Node& launch_yaml_)
    {
      LaunchConfiguration launch_configuration;

      auto remap_node = launch_yaml_["remap"];

      if (!remap_node || !remap_node.IsMap()) return launch_configuration;

      auto pubsub_node = remap_node["pubsub"];

      fillPubSubNode(launch_configuration, pubsub_node);

      auto service_node = remap_node["service"];

      fillServiceNode(launch_configuration, service_node);

      return launch_configuration;
    }

  } // Config
} // eCAL