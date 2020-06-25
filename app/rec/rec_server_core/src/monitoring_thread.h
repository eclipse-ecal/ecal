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
#include <shared_mutex>
#include <functional>
#include <vector>
#include <string>

#include <rec_server_core/rec_server_types.h>

namespace eCAL
{
  namespace rec_server
  {
    class MonitoringThread : public InterruptibleLoopThread
    {
    /////////////////////////////////////////
    // Constructor and destructor
    /////////////////////////////////////////
    public:
      MonitoringThread(const std::function<std::map<std::string, int32_t>(void)>& get_running_enabled_rec_clients_function);
      ~MonitoringThread();

    /////////////////////////////////////////
    // Public API
    /////////////////////////////////////////
    public:
      void SetPostUpdateCallbacks(const std::vector<PostUpdateCallback_T>& post_update_callbacks);

      std::map<std::string, eCAL::rec_server::TopicInfo> GetTopicInfoMap() const;
      std::map<std::string, bool> GetHostsRunningEcalRec() const;

    /////////////////////////////////////////
    // Interruptible Thread overrides
    /////////////////////////////////////////
    protected:
      void Loop() override;

    /////////////////////////////////////////
    // Member variables
    /////////////////////////////////////////
    private:
      mutable std::shared_timed_mutex   monitoring_mutex_;                      ///< Mutex protecting the result values
      
      TopicInfoMap_T                    topic_info_map_;                        ///< Result from monitoring: Evaluated topic information (the PIDs are filted using the get_running_enabled_rec_clients_function_)
      HostsRunningEcalRec_T             hosts_running_ecal_rec_;                ///< Result from monitoring: A list of all hosts and whether the monitor found a running eCAL Rec client on it

      const std::function<std::map<std::string, int32_t>(void)> get_running_enabled_rec_clients_function_; ///< Function that is called to determine which recorders are currently running. The output is used to filter the monitoring result
      std::vector<PostUpdateCallback_T> post_update_callbacks_;                 ///< List of callback functions executed after a monitoring loop (executed in the monitoring thread!)
    };
  }
}