/* ========================= eCAL LICENSE =================================
 *
 * Copyright 2026 AUMOVIO and subsidiaries. All rights reserved.
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

#include <string>

namespace eCAL
{
  class CTimeBase
  {
  public:
    CTimeBase() = default;
    CTimeBase(const CTimeBase&) = default;
    CTimeBase& operator=(const CTimeBase&) = default;

    CTimeBase(CTimeBase&&) noexcept = default;
    CTimeBase& operator=(CTimeBase&&) noexcept = default;

    virtual ~CTimeBase() = default;

    virtual long long GetNanoseconds() const = 0;
    virtual bool SetNanoseconds(long long time_) const = 0;

    virtual bool IsSynchronized() const = 0;
    virtual bool IsMaster() const = 0;

    virtual void SleepForNanoseconds(long long duration_nsecs_) const = 0;

    virtual void GetStatus(int& error_, std::string* const message_) const = 0;
  };
}
