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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include <ecal/core/pb/monitoring.pb.h>
#include <ecal/core/pb/process.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

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
      std::string          monitoring_string;
      eCAL::pb::Monitoring monitoring_pb;

      if (eCAL::Monitoring::GetMonitoring(monitoring_string))
      {
        monitoring_pb.Clear();
        monitoring_pb.ParseFromString(monitoring_string);

        auto running_enabled_rec_clients = get_running_enabled_rec_clients_function_();
        
        {
          std::unique_lock<decltype(monitoring_mutex_)> monitoring_lock(monitoring_mutex_);

          if (IsInterrupted()) return;

          // Clear host list
          hosts_running_ecal_rec_.clear();

          // Clear topic info map
          topic_info_map_.clear();

          // Re-create host list
          for (const auto& process : monitoring_pb.processes())
          {
            std::string host_name = process.host_name();
            auto existing_host_it = hosts_running_ecal_rec_.find(host_name);

            // Add host if it didn't exist already
            if (existing_host_it == hosts_running_ecal_rec_.end())
            {
              existing_host_it = hosts_running_ecal_rec_.emplace(host_name, false).first;
            }

            // set whether this host has an eCAL Rec client
            if (process.unit_name() == "eCALRecClient")
            {
              existing_host_it->second = true;
            }
          }

          // Update topic list
          for (const auto& topic : monitoring_pb.topics())
          {
            // Create combined encoding:type type (to be fully compatible to old behavior)
            std::string combined_enc_type = eCAL::Util::CombinedTopicEncodingAndType(topic.datatype_information().encoding(), topic.datatype_information().name());

            auto topic_info_map_it = topic_info_map_.find(topic.topic_name());
            if (topic_info_map_it != topic_info_map_.end())
            {
              // Only update the values if there are information available
              if (!combined_enc_type.empty() || !topic.datatype_information().name().empty())
              {
                topic_info_map_it->second.type_ = combined_enc_type;
              }
            }
            else
            {
              // Create a new topic entry
              topic_info_map_.emplace(topic.topic_name(), eCAL::rec_server::TopicInfo(combined_enc_type));
              topic_info_map_it = topic_info_map_.find(topic.topic_name());
            }

            // Set the topic publisher
            if (EcalUtils::String::Icompare(topic.direction(), "publisher"))
            {
              auto existing_publisher_it = topic_info_map_it->second.publishers_.find(topic.host_name());
              if (existing_publisher_it != topic_info_map_it->second.publishers_.end())
              {
                existing_publisher_it->second.emplace(topic.unit_name());
              }
              else
              {
                topic_info_map_it->second.publishers_.emplace(topic.host_name(), std::set<std::string>{topic.unit_name()});
              }
            }

            // Set the subscribing eCAL Rec instances
            if (((topic.unit_name() == "eCALRecClient") || (topic.unit_name() == "eCALRecGUI"))
              && EcalUtils::String::Icompare(topic.direction(), "subscriber"))
            {
              auto running_enabled_rec_client_it = running_enabled_rec_clients.find(topic.host_name());
              if ((running_enabled_rec_client_it != running_enabled_rec_clients.end()
                && (running_enabled_rec_client_it->second == topic.process_id())))
              {
                topic_info_map_it->second.rec_subscribers_[{topic.host_name(), topic.process_id()}] = (static_cast<double>(topic.data_frequency()) / 1000.0);
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