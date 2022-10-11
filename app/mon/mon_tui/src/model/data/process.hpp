
#pragma once

#include <string>
#include <cstdint>

struct Process
{
  enum class Severity
  {
    UNKNOWN, HEALTHY, WARNING, CRITICAL, FAILED
  };

  enum class SeverityLevel
  {
    UNKNOWN, LEVEL_1, LEVEL_2, LEVEL_3, LEVEL_4, LEVEL_5
  };

  enum class TimeSyncState
  {
    NONE, REALTIME, REPLAY
  };

  int32_t pid;
  std::string name;
  std::string host_name;
  std::string unit_name;
  std::string params;
  int64_t memory_usage;
  float cpu_usage;
  float user_time;
  int64_t data_sent_bytes;
  int64_t data_recieved_bytes;
  Severity severity;
  SeverityLevel severity_level;
  std::string state_info;
  TimeSyncState time_sync_state;
  std::string time_sync_mod_name;
  std::string component_init_info;
  std::string ecal_runtime_version;
};
