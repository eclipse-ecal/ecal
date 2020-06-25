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
#pragma warning(disable: 4100 4127 4146 4505 4800) // disable proto warnings
#endif
#include <ecal/pb/monitoring.pb.h>
#include <ecal/pb/process.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "rec_client_core/ecal_rec_logger.h"
#include "ecal_rec_impl.h"

namespace eCAL
{
  namespace rec
  {
    MonitoringThread::MonitoringThread(EcalRecImpl& recorder)
      : InterruptibleLoopThread(std::chrono::seconds(1))
      , recorder_(recorder)
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

          // Clear publisher lists of all topics
          for (auto& topic_info : topic_info_map_)
          {
            topic_info.second.publishers_.clear();
          }

          // Collect all descriptors
          std::map<std::string, std::pair<int, std::pair<std::string, std::string>>> channel_descriptor_map; // ChannelName -> {Type, Descriptor}
          std::map<std::string, std::pair<int, std::string>> type_descriptor_map;                            // Type        -> Descriptor           (used for topics that we know the type of, but have no other information available)

          static const int DESCRIPTION_AVAILABLE_QUALITYBIT         = 0x1 << 3;  // Having a descriptor at all is the most important thing
          static const int INFO_COMES_FROM_CORRECT_TOPIC_QUALITYBIT = 0x1 << 2;  // The information comes from the current topic (and has not been borrowed from another topic)
          static const int INFO_COMES_FROM_PUBLISHER_QUALITYBIT     = 0x1 << 1;  // A descriptor coming from the publisher is better than one from a subsriber, as we assume that the publisher knows best what he is publishing
          static const int TYPE_AVAILABLE_QUALITYBIT                = 0x1 << 0;  // Having information about the type's name available is nice but not that important to us.

          for (const auto& topic : monitoring_pb.topics())
          {
            // Lookup the topic map entry
            auto topic_info_map_it = topic_info_map_.find(topic.tname());
            if (topic_info_map_it == topic_info_map_.end())
            {
              // Create a new topic entry
              topic_info_map_.emplace(topic.tname(), eCAL::rec::TopicInfo("", ""));
              topic_info_map_it = topic_info_map_.find(topic.tname());
            }

            // Evaluate the quality of the current descriptor information
            int this_topic_info_quality = 0;

            if (!topic.tdesc().empty())
            {
              this_topic_info_quality |= DESCRIPTION_AVAILABLE_QUALITYBIT;
            }

            this_topic_info_quality |= INFO_COMES_FROM_CORRECT_TOPIC_QUALITYBIT;

            if (EcalUtils::String::Icompare(topic.direction(), "publisher"))
            {
              this_topic_info_quality |= INFO_COMES_FROM_PUBLISHER_QUALITYBIT;

              // Also update the publisher list
              auto existing_publisher_it = topic_info_map_it->second.publishers_.find(topic.hname());
              if (existing_publisher_it != topic_info_map_it->second.publishers_.end())
              {
                existing_publisher_it->second.emplace(topic.uname());
              }
              else
              {
                topic_info_map_it->second.publishers_.emplace(topic.hname(), std::set<std::string>{topic.uname()});
              }
            }

            if (!topic.ttype().empty())
            {
              this_topic_info_quality |= TYPE_AVAILABLE_QUALITYBIT;
            }

            // Update the channel_descriptor_map
            {
              auto channel_descriptor_map_it = channel_descriptor_map.find(topic.tname());
              if (channel_descriptor_map_it == channel_descriptor_map.end())
              {
                // Save the new descriptor
                channel_descriptor_map.emplace(topic.tname(), std::make_pair(this_topic_info_quality, std::make_pair(topic.ttype(), topic.tdesc())));
              }
              else
              {
                if(channel_descriptor_map_it->second.first < this_topic_info_quality)
                {
                  // If the old descriptor has a lower quality than the current descriptor, we may overwrite it!
                  channel_descriptor_map_it->second = std::make_pair(this_topic_info_quality, std::make_pair(topic.ttype(), topic.tdesc()));
                }
              }
            }

            // Update the type_descriptor_map (can of course only work if we have the type information available)
            if (!topic.ttype().empty())
            {
              int quality_for_other_channels = (this_topic_info_quality & ~INFO_COMES_FROM_CORRECT_TOPIC_QUALITYBIT);

              auto type_descriptor_map_it = type_descriptor_map.find(topic.ttype());
              if (type_descriptor_map_it == type_descriptor_map.end())
              {
                // Save the new descriptor
                type_descriptor_map.emplace(topic.ttype(), std::make_pair(quality_for_other_channels, topic.tdesc()));
              }
              else
              {
                if(type_descriptor_map_it->second.first < quality_for_other_channels)
                {
                  // If the old descriptor has a lower quality than the current descriptor, we may overwrite it!
                  type_descriptor_map_it->second = std::make_pair(quality_for_other_channels, topic.tdesc());
                }
              }
            }
          }

          // Update the type/descriptor information of the topic_info_map_
          for (auto& topic_info_map_entry : topic_info_map_)
          {
            auto channel_descriptor_entry_it = channel_descriptor_map.find(topic_info_map_entry.first);
            if ((channel_descriptor_entry_it != channel_descriptor_map.end())
              && (channel_descriptor_entry_it->second.first >= topic_info_map_entry.second.description_quality_))
            {
              topic_info_map_entry.second.type_                = channel_descriptor_entry_it->second.second.first;
              topic_info_map_entry.second.description_         = channel_descriptor_entry_it->second.second.second;
              topic_info_map_entry.second.description_quality_ = channel_descriptor_entry_it->second.first;
            }

            if (!topic_info_map_entry.second.type_.empty())
            {
              auto type_descriptor_entry_it = type_descriptor_map.find(topic_info_map_entry.second.type_);
              if ((type_descriptor_entry_it != type_descriptor_map.end())
                && (type_descriptor_entry_it->second.first >= topic_info_map_entry.second.description_quality_)) 
              {
                topic_info_map_entry.second.description_         = type_descriptor_entry_it->second.second;
                topic_info_map_entry.second.description_quality_ = type_descriptor_entry_it->second.first;
              }
            }
          }
        }

        recorder_.SetTopicInfo(topic_info_map_);
      }
      else
      {
        EcalRecLogger::Instance()->debug("eCAL::Monitoring::GetMonitoring - failure");
      }
    }

    TopicInfo MonitoringThread::GetTopicInfo(const std::string& topic_name) const
    {
      std::lock_guard<decltype(monitoring_mutex_)> monitoring_lock(monitoring_mutex_);

      auto topic_info_map_it = topic_info_map_.find(topic_name);
      if (topic_info_map_it != topic_info_map_.end())
      {
        return topic_info_map_it->second;
      }
      else
      {
        return TopicInfo();
      }
    }

    std::map<std::string, TopicInfo> MonitoringThread::GetTopicInfoMap() const
    {
      std::lock_guard<decltype(monitoring_mutex_)> monitoring_lock(monitoring_mutex_);
      return topic_info_map_;
    }
  }
}