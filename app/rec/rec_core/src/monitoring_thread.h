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

#pragma once

#include <ThreadingUtils/InterruptibleLoopThread.h>

#include <map>

#include "rec_core/topic_info.h"

namespace eCAL
{
  namespace rec
  {
    class Recorder;

    class MonitoringThread : public InterruptibleLoopThread
    {
    public:
      MonitoringThread(Recorder& recorder);
      ~MonitoringThread();

      TopicInfo GetTopicInfo(const std::string& topic_name) const;

      std::map<std::string, TopicInfo> GetTopicInfoMap() const;
      std::map<std::string, bool> GetHostsRunningEcalRpcService() const;

    protected:
      void Loop() override;

    private:
      Recorder&                          recorder_;

      mutable std::mutex                 monitoring_mutex_;
      std::map<std::string, TopicInfo>   topic_info_map_;
      std::map<std::string, bool>        hosts_running_ecalrpcservice_;
    };
  }
}