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

#ifdef __unix__

#pragma once

#include <cstdint>
#include <limits>
#include <list>
#include <string>

namespace ResourceLinux
{
struct Memory
{
  uint64_t total;       // Total memory
  uint64_t available;   // Available memory
};

struct Temperature
{
  float cpu_temperature;  // CPU Temperature

  Temperature()
  {
    cpu_temperature = std::numeric_limits<decltype(cpu_temperature)>::min();
  }

  bool IsCPUTempValid()
  {
    return cpu_temperature != std::numeric_limits<decltype(cpu_temperature)>::min();
  }
};

struct DiskStats
{
  std::string name;       // Removable drive name
  uint64_t    capacity;   // Removable drive total space
  uint64_t    available;  // Removable drive available space
  double      read;       // Removable drive read/sec
  double      write;      // Removable drive write/sec
  std::string mount_point;
};

struct Network
{
  std::string name;        // Network Interface Card(NIC) name
  double      send;        // NIC Bytes sent/sec (TX)
  double      receive;     // NIC Bytes received/sec (RX)
  std::string ip_address;  // IP address of the NIC
};

struct ProcessesMemory
{
    uint64_t  current_used_memory;   /// The current used memory in bytes
};

struct ProcessesCPU
{
   float cpu_load;          // xx.yy%
};
struct Process
{
    std::string       name;    //  name of the process
    uint32_t          id;
    std::string       commandline;
    std::string       user;
    ProcessesMemory   memory;
    ProcessesCPU      cpu_load;

};

// type definition for the a list of DiskStats
typedef std::list<DiskStats> DiskStatsList;

// type definition for the list of Networks
typedef std::list<Network> NetworkStatsList;

// type definition for the list of Processes
typedef std::list<Process> ProcessStatsList;

}  // namespace ResourceLinux

#endif  // __unix__
