
#pragma once

#include <string>
#include <cstdint>

struct Host
{
  std::string name;
  int32_t publisher_count;
  int32_t subscriber_count;
  int64_t data_sent_bytes;
  int64_t data_received_bytes;
};
