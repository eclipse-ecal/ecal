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

#include <ecal/rec/recorder_impl_base.h>

#include <thread>
#include <shared_mutex>


class RecorderImpl : public eCAL::rec::addon::RecorderImplBase
{
public:
  bool Initialize();
  bool Deinitialize();
  bool StartRecording(std::int64_t measurement_id, const std::string& path);
  bool FlushFrame(std::int64_t measurement_id, const std::shared_ptr<eCAL::rec::addon::BaseFrame>& frame);
  bool StopRecording(std::int64_t measurement_id);
  eCAL::rec::addon::Info GetInfo() const;

private:
  bool is_initialized_ = false;
  std::thread thread_;
  std::shared_timed_mutex mutex_;
};
