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

#include <cstdint>
#include <list>
#include <map>
#include <sys/types.h>
#include <unordered_map>
#include <vector>
#ifdef __unix__

#include <mutex>
#include <string>
#include <memory>

#include "../include/mma_impl.h"
#include "../include/linux/ressource.h"
#include "../include/linux/pipe_refresher.h"
#include <ecal/app/pb/mma/mma.pb.h>

#pragma once

class MMALinux : public MMAImpl
{
 public:
  /**
  * @brief Constructor
  **/
  MMALinux();

  /**
  * @brief Destructor
  **/
  ~MMALinux();

  std::mutex mutex_;
  std::string os_name_;
  int nr_of_cpu_cores_;

  std::string cpu_pipe_result_;
  std::string network_pipe_result_;
  std::string disk_pipe_result_;
  std::string process_pipe_result_;

  unsigned int cpu_pipe_count_ = 0;
  unsigned int network_pipe_count_ = 0;
  unsigned int disk_pipe_count_ = 0;
  unsigned int process_pipe_count_ = 0;

  unsigned int cpu_prev_count_;
  unsigned int net_prev_count_;
  unsigned int disk_prev_count_;
  unsigned int proc_prev_count_;
  double cpu_prev_idle_time_;

  struct t_netIo
  {
    unsigned long rec;
    unsigned long snd;
  };
  std::unordered_map<std::string, t_netIo> net_prev_map_;

  struct t_procItem
  {
    unsigned long utime{};
    unsigned int count{};
    ResourceLinux::Process process_stats;
  };
  std::map<uint32_t, t_procItem> proc_prev_map_;

  struct t_diskIo
  {
    unsigned long read;
    unsigned long write;
  };
  std::unordered_map<std::string, t_diskIo> disk_prev_map_;

  std::map<uid_t, std::string> uname_map_;
  std::string root_dev_;
  std::string arm_vcgencmd_;
  long page_size_;
  long ticks_per_second_;
  
  /**
  * @brief  Get machine statistics: CPU, Memory, Disk, Network
  *
  * @param state  Machine state reference
  *
  * @return       true if successful, false otherwise
  **/
  bool Get(eCAL::pb::mma::State& state);

 private:

  /**
  * @brief    Get CPU time that is busy. Both user and kernel time.
  *
  * @return   CPU busy time value as double.
  **/
  double GetCPULoad();

  /**
  * @brief    Get total and available memory of the system
  *
  * @return   Memory info
  **/
  ResourceLinux::Memory GetMemory();

  /**
  * @brief    Get current CPU temperature
  *
  * @return   Temperature info
  **/
  ResourceLinux::Temperature GetTemperature();

  /**
  * @brief    Get available removable drives with their statistics
  *
  * @return   List of drives containing each drive
  **/
  ResourceLinux::DiskStatsList GetDisks();

  /**
  * @brief    Get Network Interface Card(NIC) statistics
  *
  * @return   Network structure containing NIC name, bytes sent/sec, bytes received/sec
  **/
  ResourceLinux::NetworkStatsList GetNetworks();

  /**
  * @brief  Fill up state object with current values
  *
  * @param  [out]state    Machine state
  *
  **/

  void SetResourceData(eCAL::pb::mma::State& state);
  int  GetCpuCores(void);

  std::string GetOsName();
  ResourceLinux::ProcessStatsList GetProcesses();


  void OnDataReceived(const std::string& pipe_result, const std::string& command);
  void OpenPipe(const std::string& command, std::string& result);
  std::string FileToString(const std::string& command);

  std::vector<std::string>SplitLine(const std::string& input);
  bool IsDigit(const char c);
  bool FormatListData(std::vector<std::string>& the_list);
  std::list<std::string> TokenizeIntoLines(const std::string& str);
  bool SetDiskInformation(ResourceLinux::DiskStatsList& disks);
  bool SetDiskIOInformation(ResourceLinux::DiskStatsList& disk_stats_info);
  std::string GetArmvcgencmd();
  
  std::unique_ptr<PipeRefresher> cpu_pipe_;
  std::unique_ptr<PipeRefresher> network_pipe_;
  std::unique_ptr<PipeRefresher> disk_pipe_;
  std::unique_ptr<PipeRefresher> process_pipe_;
};

#endif  // __unix__
