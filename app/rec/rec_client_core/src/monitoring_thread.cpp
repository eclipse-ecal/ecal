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
      TopicInfoMap topic_info_snapshot;
      std::set<STopicId> publisher_ids;
      std::ignore = eCAL::Registration::GetPublisherIDs(publisher_ids);

      for (const auto& publisher_id : publisher_ids)
      {
        SDataTypeInformation datatype_info;
        eCAL::Registration::GetPublisherInfo(publisher_id, datatype_info);
        topic_info_snapshot.emplace(std::make_pair( publisher_id , std::move(datatype_info)));
      }

      std::lock_guard<decltype(monitoring_mutex_)> monitoring_lock(monitoring_mutex_);
      topic_info_map_ = std::move(topic_info_snapshot);
    }

    TopicInfo MonitoringThread::GetTopicInfo(const STopicId& topic) const
    {
      std::lock_guard<decltype(monitoring_mutex_)> monitoring_lock(monitoring_mutex_);

      auto topic_info_map_it = topic_info_map_.find(topic);
      if (topic_info_map_it != topic_info_map_.end())
      {
        return topic_info_map_it->second;
      }
      else
      {
        return TopicInfo();
      }
    }

    TopicInfoMap MonitoringThread::GetTopicInfoMap() const
    {
      std::lock_guard<decltype(monitoring_mutex_)> monitoring_lock(monitoring_mutex_);
      return topic_info_map_;
    }
  }
}