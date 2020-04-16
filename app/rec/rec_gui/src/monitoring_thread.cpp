/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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
#pragma warning(disable: 4146)
#endif
#include <ecal/pb/monitoring.pb.h>
#include <ecal/pb/process.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <rec_core/ecal_rec_logger.h>

MonitoringThread::MonitoringThread()
  : QObject(nullptr)
  , InterruptibleLoopThread(std::chrono::seconds(1))
{}

MonitoringThread::~MonitoringThread()
{}

void MonitoringThread::Loop()
{
  std::string          monitoring_string;
  eCAL::pb::Monitoring monitoring_pb;

  if (eCAL::Monitoring::GetMonitoring(monitoring_string))
  {
    monitoring_pb.Clear();
    monitoring_pb.ParseFromString(monitoring_string);

    {
      std::lock_guard<decltype(monitoring_mutex_)> monitoring_lock(monitoring_mutex_);

      if (IsInterrupted()) return;

      // Clear host list
      hosts_running_ecal_rec_.clear();

      // Clear topic info map
      topic_info_map_.clear();

      // Re-create host list
      for (const auto& process : monitoring_pb.processes())
      {
        std::string host_name = process.hname();
        auto existing_host_it = hosts_running_ecal_rec_.find(host_name);

        // Add host if it didn't exist already
        if (existing_host_it == hosts_running_ecal_rec_.end())
        {
          existing_host_it = hosts_running_ecal_rec_.emplace(host_name, false).first;
        }

        // set whether this host has an eCAL RPC Service
        if (process.uname() == "eCALRec")
        {
          existing_host_it->second = true;
        }
      }

      // Update topic list
      for (const auto& topic : monitoring_pb.topics())
      {
        auto topic_info_map_it = topic_info_map_.find(topic.tname());
        if (topic_info_map_it != topic_info_map_.end())
        {
          // Only update the values if there are information available
          if (!topic.ttype().empty() || !topic.tdesc().empty())
          {
            topic_info_map_it->second.type_        = topic.ttype();
            topic_info_map_it->second.description_ = topic.tdesc();
          }
        }
        else
        {
          // Create a new topic entry
          topic_info_map_.emplace(topic.tname(), eCAL::rec::TopicInfo(topic.ttype(), topic.tdesc()));
          topic_info_map_it = topic_info_map_.find(topic.tname());
        }

        // Set the topic publisher
        if (EcalUtils::String::Icompare(topic.direction(), "publisher"))
        {
          topic_info_map_it->second.publisher_hosts_.emplace(topic.hname());
        }
      }

      emit monitorUpdatedSignal(topic_info_map_);
    }
  }
  else
  {
    eCAL::rec::EcalRecLogger::Instance()->debug("eCAL::Monitoring::GetMonitoring - failure");
  }
}

std::map<std::string, eCAL::rec::TopicInfo> MonitoringThread::topicInfoMap() const
{
  std::lock_guard<decltype(monitoring_mutex_)> monitoring_lock(monitoring_mutex_);
  return topic_info_map_;
}

std::map<std::string, bool> MonitoringThread::hostsRunningEcalRec() const
{
  std::lock_guard<decltype(monitoring_mutex_)> monitoring_lock(monitoring_mutex_);
  return hosts_running_ecal_rec_;
}
