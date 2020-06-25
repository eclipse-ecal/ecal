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

#include <vector>
#include <string>
#include <chrono>
#include <ecal/ecal_time.h>
#include <ecal/ecal_callback.h>

namespace eCAL
{
  namespace rec
  {
    class Frame
    {
    public:
      Frame(const eCAL::SReceiveCallbackData* const callback_data, const std::string& topic_name, const eCAL::Time::ecal_clock::time_point receive_time, std::chrono::steady_clock::time_point system_receive_time)
        : ecal_publish_time_(std::chrono::duration_cast<eCAL::Time::ecal_clock::duration>(std::chrono::microseconds(callback_data->time)))
        , ecal_receive_time_(receive_time)
        , system_receive_time_(system_receive_time)
        , topic_name_(topic_name)
        , clock_(callback_data->clock)
        , id_(callback_data->id)
      {
        data_.reserve(callback_data->size);
        data_.assign((char*)callback_data->buf, (char*)callback_data->buf + callback_data->size);
      }

      Frame()
        : data_()
        , ecal_publish_time_(eCAL::Time::ecal_clock::time_point(eCAL::Time::ecal_clock::duration(0)))
        , ecal_receive_time_(eCAL::Time::ecal_clock::time_point(eCAL::Time::ecal_clock::duration(0)))
        , system_receive_time_(std::chrono::steady_clock::time_point(std::chrono::steady_clock::duration(0)))
        , clock_(0)
        , id_(0)
      {}

      std::vector<char>                     data_;
      eCAL::Time::ecal_clock::time_point    ecal_publish_time_;
      eCAL::Time::ecal_clock::time_point    ecal_receive_time_;
      std::chrono::steady_clock::time_point system_receive_time_;
      std::string                           topic_name_;
      long long                             clock_;
      long long                             id_;
    };
  }
}