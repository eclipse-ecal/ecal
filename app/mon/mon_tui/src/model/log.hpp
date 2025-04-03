/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include <thread>

#include <ecal/ecal.h>

#ifdef _MSC_VER
#pragma warning(push, 0) // disable proto warnings
#endif
#include "ecal/core/pb/logging.pb.h"
#include "ecal/core/pb/monitoring.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

struct LogEntry
{
  enum class Level
  {
    DEBUG1, DEBUG2, DEBUG3, DEBUG4, INFO, WARNING, ERROR, FATAL
  };
  int64_t time;
  std::string   host_name;
  int32_t       process_id;
  std::string   process_name;
  std::string   process_path;
  Level         log_level;
  std::string   message;
};

using ModelUpdateCallbackT = std::function<void()>;

class LogModel
{
  int update_time = 50;
  bool is_polling;
  int capacity = 500;

  eCAL::pb::LogMessageList logs;

  std::mutex mtx;
  std::thread update_thread;
  std::vector<ModelUpdateCallbackT> callbacks_;
  std::vector<LogEntry> data;

  void NotifyUpdate()
  {
    for(auto callback: callbacks_)
    {
      callback();
    }
  }

  LogEntry::Level ToLogLevel(int32_t val)
  {
    switch (val)
    {
    case eCAL::Logging::eLogLevel::log_level_debug1:
      return LogEntry::Level::DEBUG1;
    case eCAL::Logging::eLogLevel::log_level_debug2:
      return LogEntry::Level::DEBUG2;
    case eCAL::Logging::eLogLevel::log_level_debug3:
      return LogEntry::Level::DEBUG3;
    case eCAL::Logging::eLogLevel::log_level_debug4:
      return LogEntry::Level::DEBUG4;
    case eCAL::Logging::eLogLevel::log_level_warning:
      return LogEntry::Level::WARNING;
    case eCAL::Logging::eLogLevel::log_level_error:
      return LogEntry::Level::ERROR;
    case eCAL::Logging::eLogLevel::log_level_fatal:
      return LogEntry::Level::FATAL;
    case eCAL::Logging::eLogLevel::log_level_info:
    default:
      return LogEntry::Level::INFO;
    }
  }

  LogEntry ToLogEntry(const eCAL::pb::LogMessage &val)
  {
    return LogEntry {
      val.time(),
      val.host_name(),
      val.process_id(),
      val.unit_name(),
      val.process_name(),
      ToLogLevel(val.level()),
      val.content()
    };
  }

  void Update()
  {
    {
      std::lock_guard<std::mutex> lock{mtx};
      std::string raw_data;
      eCAL::Logging::GetLogging(raw_data);
      logs.ParseFromString(raw_data);

      auto &pb_logs = logs.log_messages();
      auto new_entries_count = pb_logs.size();
      if(new_entries_count == 0)
      {
        return;
      }
      auto new_size = data.size() + new_entries_count;
      int overflow_size = static_cast<int>(new_size) - capacity;

      if(overflow_size > 0)
      {
        data.erase(data.begin(), data.begin() + overflow_size);
      }
      for(auto &l: logs.log_messages()) data.push_back(ToLogEntry(l));
    }

    NotifyUpdate();
  }

  void PollData()
  {
    while(is_polling)
    {
      Update();
      eCAL::Process::SleepMS(update_time);
    }
  }

public:
  LogModel() : is_polling{true},
    update_thread{std::bind(&LogModel::PollData, this)}
  {
  }

  std::vector<LogEntry> Logs()
  {
    std::lock_guard<std::mutex> lock{mtx};
    return data;
  }

  void Clear()
  {
    {
      std::lock_guard<std::mutex> lock{mtx};
      data.clear();
    }
    NotifyUpdate();
  }

  void AddModelUpdateCallback(ModelUpdateCallbackT callback)
  {
    callbacks_.push_back(callback);
  }

  ~LogModel()
  {
    is_polling = false;
    update_thread.join();
  }
};
