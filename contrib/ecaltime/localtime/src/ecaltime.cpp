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

#include <ecaltime.h>
#include <chrono>
#include <thread>
#include <string.h>

ECALTIME_API int etime_initialize(void)
{
  return 0;
}

ECALTIME_API int etime_finalize(void)
{
  return 0;
}

ECALTIME_API long long etime_get_nanoseconds()
{
  auto now = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
}

ECALTIME_API int etime_set_nanoseconds(long long /*time_*/)
{
  return -1;
}

ECALTIME_API int etime_is_synchronized()
{
  return 1;
}

ECALTIME_API int etime_is_master()
{
  return 1;
}

ECALTIME_API void etime_sleep_for_nanoseconds(long long duration_nsecs_) {
  std::chrono::nanoseconds duration(duration_nsecs_);
  std::this_thread::sleep_for(duration);
}

ECALTIME_API void etime_get_status(int* error_, char* status_message_, int max_len_) {
  if (error_) {
    *error_ = 0;
  }
  if (status_message_ && max_len_ > 0) {
    static const char* status{ "everything is fine."};
    strncpy(status_message_, status, max_len_ - 1);
    status_message_[max_len_ -1] = (char)0x0;
  }
}
