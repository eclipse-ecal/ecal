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

#include "monitoring_thread.h"

#include <ecal/ecal.h>

#include <ecal_utils/ecal_utils.h>

#include <rec_client_core/ecal_rec_logger.h>

#include <iostream>

namespace eCAL
{
  namespace rec_server
  {
    /////////////////////////////////////////
    // Constructor and destructor
    /////////////////////////////////////////

    MonitoringThread::MonitoringThread(const std::function<std::map<std::string, int32_t>(void)>& get_running_enabled_rec_clients_function)
      : InterruptibleLoopThread(std::chrono::seconds(1))
      , get_running_enabled_rec_clients_function_(get_running_enabled_rec_clients_function)
    {}

    MonitoringThread::~MonitoringThread()
    {}

    /////////////////////////////////////////
    // Public API
    /////////////////////////////////////////

    // TODO: Document that these update callbacks must never call the two getters, as those lock the same mutex!
    void MonitoringThread::SetPostUpdateCallbacks(const std::vector<PostUpdateCallback_T>& post_update_callbacks)
    {
      std::unique_lock<decltype(monitoring_mutex_)> monitoring_lock(monitoring_mutex_);
      post_update_callbacks_ = post_update_callbacks;
    }

    std::map<std::string, eCAL::rec_server::TopicInfo> MonitoringThread::GetTopicInfoMap() const
    {
      std::shared_lock<decltype(monitoring_mutex_)> monitoring_lock(monitoring_mutex_);
      return topic_info_map_;
    }

    std::map<std::string, bool> MonitoringThread::GetHostsRunningEcalRec() const
    {
      std::shared_lock<decltype(monitoring_mutex_)> monitoring_lock(monitoring_mutex_);
      return hosts_running_ecal_rec_;
    }

    /////////////////////////////////////////
    // Interruptible Thread overrides
    /////////////////////////////////////////

    void MonitoringThread::Loop()
    {
      eCAL::Monitoring::SMonitoring monitoring;

      if (eCAL::Monitoring::GetMonitoring(monitoring))
      {
        auto running_enabled_rec_clients = get_running_enabled_rec_clients_function_();
        
        {
          std::unique_lock<decltype(monitoring_mutex_)> monitoring_lock(monitoring_mutex_);

          if (IsInterrupted()) return;

          // Clear host list
          hosts_running_ecal_rec_.clear();

          // Clear topic info map
          topic_info_map_.clear();

          // Re-create host list
          for (const auto& process : monitoring.processes)
          {
            std::string host_name = process.host_name;
            auto existing_host_it = hosts_running_ecal_rec_.find(host_name);

            // Add host if it didn't exist already
            if (existing_host_it == hosts_running_ecal_rec_.end())
            {
              existing_host_it = hosts_running_ecal_rec_.emplace(host_name, false).first;
            }

            // set whether this host has an eCAL Rec client
            if (process.unit_name == "eCALRecClient")
            {
              existing_host_it->second = true;
            }
          }

          auto add_type_info = [this](auto topic) -> auto
           {
              auto topic_info_map_it = topic_info_map_.find(topic.topic_name);
              if (topic_info_map_it == topic_info_map_.end())
              {
                // Create a new topic entry
                auto emplace_result = topic_info_map_.emplace(topic.topic_name, eCAL::rec_server::TopicInfo());
                topic_info_map_it = emplace_result.first;
              }
              topic_info_map_it->second.AddTypeInfo(topic.datatype_information);
              return topic_info_map_it;
            };

          // Update topic list
          for (const auto& publisher : monitoring.publishers)
          {
            auto topic_info_map_it = add_type_info(publisher);

            // Set the topic publisher
            auto existing_publisher_it = topic_info_map_it->second.publishers_.find(publisher.host_name);
            if (existing_publisher_it != topic_info_map_it->second.publishers_.end())
            {
              existing_publisher_it->second.emplace(publisher.unit_name);
            }
            else
            {
              topic_info_map_it->second.publishers_.emplace(publisher.host_name, std::set<std::string>{publisher.unit_name});
            }
          }

          for (const auto& subscriber : monitoring.subscribers)
          {
            auto topic_info_map_it = add_type_info(subscriber);

            // Set the subscribing eCAL Rec instances
            if ((subscriber.unit_name == "eCALRecClient") || (subscriber.unit_name == "eCALRecGUI"))
            {
              auto running_enabled_rec_client_it = running_enabled_rec_clients.find(subscriber.host_name);
              if ((running_enabled_rec_client_it != running_enabled_rec_clients.end()
                && (running_enabled_rec_client_it->second == subscriber.process_id)))
              {
                topic_info_map_it->second.rec_subscribers_[{subscriber.host_name, subscriber.process_id}] = (static_cast<double>(subscriber.data_frequency) / 1000.0);
              }
            }
          }
        }

        {
          std::shared_lock<decltype(monitoring_mutex_)> monitoring_lock(monitoring_mutex_);
          if (IsInterrupted()) return;

          // Execute update callbacks
          for (const auto& callback : post_update_callbacks_)
          {
            callback(topic_info_map_, hosts_running_ecal_rec_);
            if (IsInterrupted()) return;
          }
        }
      }
      else
      {
        eCAL::rec::EcalRecLogger::Instance()->debug("eCAL::Monitoring::GetMonitoring - failure");
      }
    }
  }
}