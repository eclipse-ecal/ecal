#pragma once

#include <string>
#include <vector>
#include <cstdint>

struct Service
{
  struct Method
  {
    std::string name;
    std::string request_type;
    std::string response_type;
    int64_t call_count;
  };

  std::string name;
  std::string id;
  std::string host_name;
  std::string process_name;
  int32_t pid;
  std::string unit_name;
  int32_t registration_clock;
  uint32_t tcp_port;
  std::vector<Method> methods;
};
