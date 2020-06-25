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

#include <chrono>
#include <map>
#include <string>
#include <list>

#include <rec_client_core/job_config.h>
#include <rec_client_core/state.h>

#include "rec_server_types.h"

namespace eCAL
{
  namespace rec_server
  {
    struct ClientJobStatus
    {
      ClientJobStatus()
        : client_pid_(-1)
        , info_last_command_response_{true, ""}
      {}

      int                          client_pid_;
      eCAL::rec::JobStatus         job_status_;
      std::pair<bool, std::string> info_last_command_response_;
    };

    struct JobHistoryEntry
    {
      JobHistoryEntry()
        : local_start_time_(std::chrono::system_clock::duration(0))
        , is_deleted_      (false)
      {}

      std::chrono::system_clock::time_point  local_start_time_;
      eCAL::rec::JobConfig                   local_evaluated_job_config_;

      bool                                   is_deleted_;

      std::map<std::string, ClientJobStatus> client_statuses_;
    };

    struct RecServerStatus
    {
      RecServerStatus()
        : activated_        (false)
        , recording_meas_id_(0)
      {}

      std::string                config_path_;
      bool                       activated_;
      int64_t                    recording_meas_id_;
      std::list<JobHistoryEntry> job_history_;
      RecorderStatusMap_T        client_statuses_;
    };
  }
}