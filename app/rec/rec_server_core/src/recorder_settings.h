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

#include <string>
#include <chrono>
#include <set>

#include <rec_client_core/record_mode.h>

namespace eCAL
{
  namespace rec_server
  {
    class RecorderSettings
    {
    public:
      RecorderSettings()  {};
      ~RecorderSettings() {};

      void SetMaxPreBufferLength(const std::chrono::steady_clock::duration max_pre_buffer_length) { max_pre_buffer_length_.first = true; max_pre_buffer_length_.second = max_pre_buffer_length; }
      void ClearMaxPreBufferLength() { max_pre_buffer_length_.first = false; max_pre_buffer_length_.second = std::chrono::steady_clock::duration(0); }
      bool IsMaxPreBufferLengthSet() const { return max_pre_buffer_length_.first; }
      std::chrono::steady_clock::duration GetMaxPreBufferLength() const { return max_pre_buffer_length_.second; }

      void SetPreBufferingEnabled(const bool& pre_buffering_enabled) { pre_buffering_enabled_.first = true; pre_buffering_enabled_.second = pre_buffering_enabled; }
      void ClearPreBufferingEnabled() { pre_buffering_enabled_.first = false; pre_buffering_enabled_.second = false; }
      bool IsPreBufferingEnabledSet() const { return pre_buffering_enabled_.first; }
      bool GetPreBufferingEnabled() const { return pre_buffering_enabled_.second; }

      void SetHostFilter(const std::set<std::string>& host_filter) { host_filter_.first = true; host_filter_.second = host_filter; }
      void ClearHostFilter() { host_filter_.first = false; host_filter_.second.clear(); }
      bool IsHostFilterSet() const { return host_filter_.first; }
      std::set<std::string> GetHostFilter() const { return host_filter_.second; }

      void SetRecordMode(const eCAL::rec::RecordMode& record_mode) { record_mode_.first = true; record_mode_.second = record_mode; }
      void ClearRecordMode() { record_mode_.first = false; record_mode_.second = eCAL::rec::RecordMode::All; }
      bool IsRecordModeSet() const { return record_mode_.first; }
      eCAL::rec::RecordMode GetRecordMode() const { return record_mode_.second; }

      void SetListedTopics(const std::set<std::string>& listed_topics) { listed_topics_.first = true; listed_topics_.second = listed_topics; }
      void ClearListedTopics() { listed_topics_.first = false; listed_topics_.second.clear(); }
      bool IsListedTopicsSet() const { return listed_topics_.first; }
      std::set<std::string> GetListedTopics() const { return listed_topics_.second; }

      void SetEnabledAddons(const std::set<std::string>& enabled_addons) { enabled_addons_.first = true; enabled_addons_.second = enabled_addons; }
      void ClearEnabledAddons() { enabled_addons_.first = false; enabled_addons_.second.clear(); }
      bool IsEnabledAddonsSet() const { return enabled_addons_.first; }
      std::set<std::string> GetEnabledAddons() const { return enabled_addons_.second; }

      void SetAllToDefaults()
      {
        max_pre_buffer_length_.first = true; max_pre_buffer_length_.second = std::chrono::steady_clock::duration(0);
        pre_buffering_enabled_.first = true; pre_buffering_enabled_.second = false;
        host_filter_.first           = true; host_filter_.second.clear();
        record_mode_.first           = true; record_mode_.second = eCAL::rec::RecordMode::All;
        listed_topics_.first         = true; listed_topics_.second.clear();
        enabled_addons_.first        = true; enabled_addons_.second.clear();
      }

      void AddSettings(const RecorderSettings& other)
      {
        if (other.IsMaxPreBufferLengthSet())
          SetMaxPreBufferLength(other.GetMaxPreBufferLength());

        if (other.IsPreBufferingEnabledSet())
          SetPreBufferingEnabled(other.GetPreBufferingEnabled());

        if (other.IsHostFilterSet())
          SetHostFilter(other.GetHostFilter());

        if (other.IsRecordModeSet())
          SetRecordMode(other.GetRecordMode());

        if (other.IsListedTopicsSet())
          SetListedTopics(other.GetListedTopics());

        if (other.IsEnabledAddonsSet())
          SetEnabledAddons(other.GetEnabledAddons());
      }

    private:
      std::pair<bool, std::chrono::steady_clock::duration> max_pre_buffer_length_;
      std::pair<bool, bool>                                pre_buffering_enabled_;
      std::pair<bool, std::set<std::string>>               host_filter_;
      std::pair<bool, eCAL::rec::RecordMode>                          record_mode_;
      std::pair<bool, std::set<std::string>>               listed_topics_;
      std::pair<bool, std::set<std::string>>               enabled_addons_;
    };
  }
}