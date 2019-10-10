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

/**
 * @brief  Implementation of the eCAL time synchronization dll interface
**/

#include <ecaltime.h>
#include <string>

#include "ecal_time_simtime.h"

static eCAL::CSimTime replaytime_adapter;

ECALTIME_API int etime_initialize(void)
{
  if (replaytime_adapter.initialize()) {
    return 0;
  }
  else { 
    return -1;
  };
}

ECALTIME_API int etime_finalize(void)
{
  if (replaytime_adapter.finalize()) {
    return 0;
  }
  else {
    return -1;
  };
}

ECALTIME_API long long etime_get_nanoseconds()
{
  uint64_t master_ns = 0;
  if (replaytime_adapter.getMasterTime(master_ns))
  {
    return (long long) master_ns;
  }
  else {
    return 0;
  }
}

ECALTIME_API int etime_set_nanoseconds(long long /*time_*/)
{
  return -1;
}

ECALTIME_API int etime_is_synchronized()
{
  if (replaytime_adapter.isSynchronized())
  {
    return 1;
  }
  else {
    return 0;
  }
}

ECALTIME_API int etime_is_master()
{
  return 0;
}

ECALTIME_API void etime_sleep_for_nanoseconds(long long duration_nsecs_) {
  replaytime_adapter.sleep_for(duration_nsecs_);
}

ECALTIME_API void etime_get_status(int* error_, char* status_message_, int max_len_) {
  int error;
  if (status_message_ && max_len_ > 0) {
    std::string status_message;
    replaytime_adapter.getStatus(error, &status_message);
    strncpy(status_message_, status_message.c_str(), max_len_ - 1);
    status_message_[max_len_ - 1] = (char)0x0;
  }
  else {
    replaytime_adapter.getStatus(error, nullptr);
  }
  if (error_) {
    *error_ = error;
  }
}
