/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
 * @brief  eCAL time gateway class
**/

#pragma once

#include <memory>
#include <string>

#include "ecal_time_plugin.h"

namespace eCAL
{
  class CTimePlugin;

  class CTimeGate
  {
  public:
    static std::shared_ptr<CTimeGate> CreateTimegate();

    // TODO: these need to be removed
    enum eTimeSyncMode { none, realtime, replay };

  private:
    CTimeGate(CTimePlugin&& time_plugin, std::string plugin_name);

  public:
    ~CTimeGate();

    CTimeGate(const CTimeGate&) = delete;
    CTimeGate& operator=(const CTimeGate&) = delete;

    CTimeGate(CTimeGate&&) noexcept = default;
    CTimeGate& operator=(CTimeGate&&) = delete;

    const std::string& GetName() const;

    long long GetMicroSeconds();
    long long GetNanoSeconds();

    bool SetNanoSeconds(long long time_);

    bool IsSynchronized();
    bool IsMaster();
    
    void SleepForNanoseconds(long long duration_nsecs_);

    void GetStatus(int& error_, std::string* status_message_);

    eTimeSyncMode GetSyncMode() { return(eTimeSyncMode::realtime); };

    CTimePlugin m_time_plugin;
    std::string m_plugin_name;
  };
}
