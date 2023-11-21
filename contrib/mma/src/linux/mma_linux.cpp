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

#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>

#include "../include/linux/mma_linux.h"

#define B_IN_KB 1024.0
#define B_IN_MB 1048576.0

#define MMA_CPU_CMD "cat /proc/uptime"
#define MMA_NET_CMD "cat /proc/net/dev"
#define MMA_DISK_CMD "cat /proc/diskstats"
#define MMA_PS_CMD "cat /proc/[0-9]*/stat 2>/dev/null"

MMALinux::MMALinux():
  cpu_pipe_(std::make_unique<PipeRefresher>(MMA_CPU_CMD,500)),
  network_pipe_(std::make_unique<PipeRefresher>(MMA_NET_CMD,500)),
  disk_pipe_(std::make_unique<PipeRefresher>(MMA_DISK_CMD,500)),
  process_pipe_(std::make_unique<PipeRefresher>(MMA_PS_CMD,500))
{
  cpu_pipe_->AddCallback(std::bind(&MMALinux::OnDataReceived, this, std::placeholders::_1, std::placeholders::_2));
  network_pipe_->AddCallback(std::bind(&MMALinux::OnDataReceived, this, std::placeholders::_1, std::placeholders::_2));
  disk_pipe_->AddCallback(std::bind(&MMALinux::OnDataReceived, this, std::placeholders::_1, std::placeholders::_2));
  process_pipe_->AddCallback(std::bind(&MMALinux::OnDataReceived, this, std::placeholders::_1, std::placeholders::_2));

  is_vrm = CheckIfIsALinuxVRM();
  nr_of_cpu_cores = GetCpuCores();

}

MMALinux::~MMALinux()
{
}

void MMALinux::OnDataReceived(const std::string& pipe_result, const std::string& command)
{
  std::lock_guard<std::mutex> guard(mutex);
  if (command == MMA_CPU_CMD)
  {
    cpu_pipe_result_ = pipe_result;
    cpu_pipe_count_++;
  }
  else if (command == MMA_NET_CMD)
  {
    network_pipe_result_ = pipe_result;
    network_pipe_count_++;
  }
  else if (command == MMA_DISK_CMD)
  {
    disk_pipe_result_ = pipe_result;
    disk_pipe_count_++;
  }
  else if (command == MMA_PS_CMD)
  {
    process_pipe_result_ = pipe_result;
    process_pipe_count_++;
  }

}

void MMALinux::OpenPipe(const std::string& command, std::string& result)
{
  FILE* file = popen(command.data(), "r");
  char buffer[4096];

  if (file != nullptr)
  {
    while (fgets(buffer, sizeof(buffer), file) != nullptr)
      result += buffer;

    pclose(file);
  }
  strcpy(buffer, "");
}

std::string MMALinux::FileToString(const std::string& command)
{
  FILE* file = fopen(command.data(), "r");
  char buffer[4096];
  std::string result;

  if (file != nullptr)
  {
    while (fgets(buffer, sizeof(buffer), file) != nullptr)
      result += buffer;

    fclose(file);
  }

  return result;
}

double MMALinux::GetCPULoad()
{
  std::string local_copy;
  unsigned int local_count;
  static unsigned int previous_count;
  {
    std::lock_guard<std::mutex> guard(mutex);
    local_copy = cpu_pipe_result_;
    local_count = cpu_pipe_count_;
  }

  double cpu_time = 0.0;

  if (local_copy.length() > 0)
  {
    std::vector<std::string> cpu_line = SplitLine(local_copy);
    const double idle_time = stod(cpu_line.back());
    const unsigned int delta_count = (local_count - previous_count) * nr_of_cpu_cores;
    static double previous_idle_time = idle_time - delta_count / 2;
    const double current_idle = idle_time - previous_idle_time;

    if (delta_count > 0)
      cpu_time = 100.0 * (1 - 2 * current_idle / delta_count);
    previous_idle_time = idle_time;
    previous_count = local_count;
  }

  return cpu_time;
}

ResourceLinux::Memory MMALinux::GetMemory()
{
  // Ubuntu 12.04 and 14.04 provide different information than Ubuntu 16.04

  ResourceLinux::Memory memory;

  std::string memfile_str = FileToString("/proc/meminfo");
  std::unordered_map<std::string, uint64_t> memory_map;

  std::stringstream stream(memfile_str);

  for (int i = 0; i < 5; i++)
  {
    std::string title;
    uint64_t title_data;
    std::string size_type;
    stream >> title >> title_data >> size_type;
    memory_map[title] = title_data;
  }

  if (memory_map.find("MemAvailable:") != memory_map.end())
  {
    memory.available = memory_map["MemAvailable:"];
    memory.total = memory_map["MemTotal:"];
  }
  else
  {
    memory.available = memory_map["MemFree:"] + memory_map["Buffers:"] + memory_map["Cached:"];
    memory.total = memory_map["MemTotal:"];
  }

  memory.total *= B_IN_KB;
  memory.available *= B_IN_KB;

  return memory;
}

ResourceLinux::Temperature MMALinux::GetTemperature()
{
  ResourceLinux::Temperature temperature;

#ifdef __arm__

  std::string result;
  OpenPipe("/opt/vc/bin/vcgencmd measure_temp", result);

  std::smatch match;
  const std::regex expression("\\d{2,}.\\d{1,}");

  if (std::regex_search(result, match, expression) && match.size() > 0)
  {
    temperature.cpu_temperature = stof(match.str(0));
  }

#endif

  return temperature;
}

ResourceLinux::DiskStatsList MMALinux::GetDisks()
{
  ResourceLinux::DiskStatsList disk_stats_info;

  SetDiskInformation(disk_stats_info);

  SetDiskIOInformation(disk_stats_info);

  return disk_stats_info;
}

ResourceLinux::NetworkStatsList MMALinux::GetNetworks()
{
  std::string local_copy;
  unsigned int local_count;
  static unsigned int previous_count;
  typedef struct
  {
    unsigned long rec;
    unsigned long snd;
  } t_io;
  static std::unordered_map<std::string, t_io> previous;
  {
    std::lock_guard<std::mutex> guard(mutex);
    local_copy = network_pipe_result_;
    local_count = network_pipe_count_;
  }

  ResourceLinux::NetworkStatsList networks;
  ResourceLinux::Network network_stats;

  auto lines = TokenizeIntoLines(local_copy);
  if (lines.size() > 0)
  {
    // skip the first 2 lines
    auto iterator = lines.begin();
    std::advance(iterator, 2);

    const unsigned long int delta_count = (local_count - previous_count);
    
    while(iterator != lines.end())
    {
      std::vector<std::string> network_io = SplitLine(*iterator);
      t_io cur = {0, 0};
      t_io prev;
      network_stats.name=network_io[0];

      try { cur.rec = stoul(network_io[1]); }
      catch (...) { }

      try { cur.snd = stoul(network_io[9]); }
      catch (...) { }

      if (previous.find(network_stats.name) != previous.end())
      {
        prev = previous[network_stats.name];
      }
      else
      {
        prev = cur;
      }

      network_stats.receive = (cur.rec - prev.rec) / (0.5 * delta_count);
      network_stats.send = (cur.snd - prev.snd) / (0.5 * delta_count);

      networks.push_back(network_stats);
      previous[network_stats.name] = cur;
      std::advance(iterator, 1);
    }
    previous_count = local_count;
  }

  return networks;
}
ResourceLinux::ProcessStatsList MMALinux::GetProcesses()
{
  std::string local_copy;
  static const long page_size = sysconf(_SC_PAGE_SIZE);
  static const long ticks_per_second = sysconf(_SC_CLK_TCK);
  static unsigned int previous_count;
  static std::unordered_map<uint32_t, unsigned long> previous;
  unsigned int local_count;
  {
    std::lock_guard<std::mutex> guard(mutex);
    local_copy = process_pipe_result_;
    local_count = process_pipe_count_;
  }

  ResourceLinux::ProcessStatsList processes;
  ResourceLinux::Process process_stats;

  auto lines = TokenizeIntoLines(local_copy);
  if (lines.size() > 0)
  {
    auto iterator = lines.begin();
    while(iterator!=lines.end())
    {
      std::vector<std::string> process_data = SplitLine(*iterator);
      process_stats.id=std::stoi(process_data[0]);
      struct stat info;
      std::string filename = "/proc/" + process_data[0];
      if (0 == stat(filename.c_str(), &info))
      {
        struct passwd *pw = getpwuid(info.st_uid);
        if (pw)
          process_stats.user=pw->pw_name;
      }

      process_stats.memory.current_used_memory=stoull(process_data[23]) * page_size;
      if (process_data[1].length() > 2)
        process_stats.commandline=process_data[1].substr(1, process_data[1].length()-2);

      const unsigned int delta_count = (local_count - previous_count) * ticks_per_second * nr_of_cpu_cores;
      const unsigned long cur = std::stoul(process_data[13]);
      unsigned long prev;
      if (previous.find(process_stats.id) != previous.end())
      {
        prev = previous[process_stats.id];
      }
      else
      {
        prev = cur;
      }

      if (delta_count > 0)
        process_stats.cpu_load.cpu_load = (cur - prev) / (0.005 * delta_count);
      processes.push_back(process_stats);
      previous[process_stats.id] = cur;
      std::advance(iterator, 1);
    }
    previous_count = local_count;
  }
  return processes;
}

bool MMALinux::Get(eCAL::pb::mma::State& state)
{
  state.clear_disks();
  state.clear_networks();
  state.clear_process();
  SetResourceData(state);
  return true;
}

std::vector<std::string> MMALinux::SplitLine(const std::string& input)
{
  std::istringstream buffer(input);
  std::vector<std::string> ret((std::istream_iterator<std::string>(buffer)),
    std::istream_iterator<std::string>());
  return ret;
}

bool MMALinux::IsDigit(const char c)
{
  return ('0' <= c && c <= '9');
}

bool MMALinux::FormatListData(std::vector<std::string>& the_list)
{
  bool return_value = true;

  if (!the_list.empty())
  {
    // delete the unnecessary columns from the pipe result
    auto iterator = the_list.begin();
    std::advance(iterator, 1);
    the_list.erase(iterator);

    iterator = the_list.begin();
    std::advance(iterator, 3);
    the_list.erase(iterator);

    iterator = the_list.begin();
    std::string name = *iterator;

    // /dev/root needs special handling
    if (name == "/dev/root")
    {
      static std::string root_dev;
      if (root_dev.empty())
      {
        std::string result;
        OpenPipe("cat /proc/cmdline", result);
        std::size_t found = result.find("root=");
        result = result.substr(found + 5);
        found = result.find(" ");
        root_dev = result.substr(0, found);
      }
      name = root_dev;
    }

    // if last character of string is digit, delete the digit
    if (IsDigit(name.back()))
    {
      // exception: mtdblock
      if (name.find("mtdblock") == std::string::npos)
      {
        name.pop_back();
      }
    }

    *iterator = name;

    // format the path to include just the name
    std::size_t found = name.find_last_of("/");
    // only accept devices which are indicated by the / in name
    if (found == std::string::npos)
      return_value = false;
    *iterator = name.substr(found + 1);
  }
  else
  {
    return_value = false;
  }

  return return_value;
}

std::list<std::string> MMALinux::TokenizeIntoLines(const std::string& str)
{
  std::list<std::string> result;

  std::stringstream ss(str);
  std::string item;

  while (std::getline(ss, item))
  {
    if (item.empty() == false)
      result.push_back(std::move(item));
  }

  return result;
}

bool MMALinux::SetDiskInformation(ResourceLinux::DiskStatsList& disks)
{
  bool return_value = false;

  std::string result;
  OpenPipe("df", result);

  // disks sorted in map
  std::map<std::string, ResourceLinux::DiskStats> disks_map;

  auto lines = TokenizeIntoLines(result);

  auto it = lines.begin();
  std::advance(it, 1);

  lines.erase(lines.begin(), it);

  for (const auto& line : lines)
  {
    std::vector<std::string> partition = SplitLine(line);
    if (FormatListData(partition))
    {
      if (partition.size() == 4)
      {
        const uint64_t KiB = 1024;
        ResourceLinux::DiskStats disk_stats;
        uint64_t used = stoll(partition[1]) * KiB;
        disk_stats.name = partition[0];
        disk_stats.available = stoll(partition[2]) * KiB;
        disk_stats.capacity = disk_stats.available + used;
        disk_stats.mount_point = partition[3];
        // ensure initialization of transfer rates
        disk_stats.read = 0.0;
        disk_stats.write = 0.0;
        if (disks_map.find(partition[0]) == disks_map.end())
        {
          disks_map[partition[0]] = disk_stats;
        }
        else
        {
          // sum up partitions of same disk
          disks_map[partition[0]].available += disk_stats.available;
          disks_map[partition[0]].capacity += disk_stats.capacity;
        }
      }
      else
      {
        return_value = true; //FixMe: what does this return value mean?
      }
    }
  }

  for (const auto& disk : disks_map)
  {
    disks.push_back(disk.second);
  }

#ifdef __arm__
  return_value |= MergeBootWithRootARM(disks);
#endif

  return return_value;
}

bool MMALinux::SetDiskIOInformation(ResourceLinux::DiskStatsList& disk_stats_info)
{
  std::string local_copy;
  unsigned int local_count;
  static unsigned int previous_count;
  typedef struct
  {
    unsigned long read;
    unsigned long write;
  } t_io;
  static std::unordered_map<std::string, t_io> previous;
  {
    std::lock_guard<std::mutex> guard(mutex);
    local_copy = disk_pipe_result_;
    local_count = disk_pipe_count_;
  }

  bool return_value = true;
  const unsigned long int delta_count = (local_count - previous_count);

  auto lines = TokenizeIntoLines(local_copy);

  for (const auto& line : lines)
  {
    std::vector<std::string> partition = SplitLine(line);

    for (auto& disk : disk_stats_info)
    {
      if (disk.name.find(partition[2]) != std::string::npos)
      {
        t_io cur = {0, 0};
        t_io prev;

        try { cur.read = static_cast<double>(stod(partition[5])); }
        catch (...) { }

        try { cur.write = static_cast<double>(stod(partition[9])); }
        catch (...) { }

        if (previous.find(disk.name) != previous.end())
        {
          prev = previous[disk.name];
        }
        else
        {
          prev = cur;
        }

        disk.read = (cur.read - prev.read) * 4 * B_IN_KB / delta_count;
        disk.write = (cur.write - prev.write) * 4 * B_IN_KB / delta_count;

        previous[disk.name] = cur;
      }
    }
  }
  previous_count = local_count;

  return return_value;
}

#ifdef __arm__
bool MMALinux::MergeBootWithRootARM(ResourceLinux::DiskStatsList& disk_stats_info)
#else
bool MMALinux::MergeBootWithRootARM(ResourceLinux::DiskStatsList& /*disk_stats_info*/)
#endif
{
  bool return_val = false;
#ifdef __arm__
  ResourceLinux::DiskStats disk_boot;
  for (const auto& disk : disk_stats_info)
  {
    if (disk.mount_point == boot)
    {
      disk_boot.name = disk.name;
      break;
    }
  }

  for (auto& disk_st : disk_stats_info)
  {
    if (disk_st.mount_point == root)
    {
      disk_st.name = disk_boot.name;
      break;
    }
  }

  for (auto it = disk_stats_info.begin(); it != disk_stats_info.end();)
  {
    if (it->mount_point == boot)
      it = disk_stats_info.erase(it);
    else
      ++it;
  }
#endif
  return return_val;
}

int MMALinux::GetCpuCores(void)
{
  return get_nprocs();
}

bool MMALinux::CheckIfIsALinuxVRM()
{
    std::string result;
    std::lock_guard<std::mutex> guard(mutex);
    OpenPipe("dmesg",result);
    std::list <std::string> dmesg_command_result = TokenizeIntoLines(result);
    for(auto iter1:dmesg_command_result)
    {
        auto temp_vector=SplitLine((iter1));
        for(auto iter2:temp_vector)
        {
            if(iter2.find("VMware")!= std::string::npos)
            {
                return true;
            }
        }
    }
    return false;
}

void MMALinux::SetResourceData(eCAL::pb::mma::State& state)
{
  state.set_cpu_load(GetCPULoad());
  state.set_number_of_cpu_cores(nr_of_cpu_cores);
  state.set_operating_system(is_vrm ? "LINUX VRM": "LINUX NATIVE");

  eCAL::pb::mma::Memory memory;
  ResourceLinux::Memory mem = GetMemory();
  memory.set_total(mem.total);
  memory.set_available(mem.available);
  state.mutable_memory()->CopyFrom(memory);

  ResourceLinux::ProcessStatsList processes=GetProcesses();
  for (const auto& process : processes)
  {
    eCAL::pb::mma::Process* new_process = state.add_process();
    new_process->set_id(process.id);
    new_process->set_commandline(process.commandline);
    new_process->set_user(process.user);

    eCAL::pb::mma::ProcessesMemory proc_memory;
    ResourceLinux::ProcessesMemory proc_mem;
    proc_mem.current_used_memory=process.memory.current_used_memory;
    proc_memory.set_current_working_set_size(proc_mem.current_used_memory);
    new_process->mutable_memory()->CopyFrom(proc_memory);

    eCAL::pb::mma::ProcessesCPU proc_cpu_load;
    ResourceLinux::ProcessesCPU cpu_load;
    cpu_load.cpu_load=process.cpu_load.cpu_load;
    proc_cpu_load.set_cpu_load(cpu_load.cpu_load);
    new_process->mutable_cpu()->CopyFrom(proc_cpu_load);
  }


  ResourceLinux::DiskStatsList disks = GetDisks();

  for (const auto& disk : disks)
  {
    eCAL::pb::mma::Disk* disk_logical = state.add_disks();
    disk_logical->set_name(disk.name);
    disk_logical->set_capacity(disk.capacity);
    disk_logical->set_available(disk.available);
    disk_logical->set_read(disk.read);
    disk_logical->set_write(disk.write);
  }

  ResourceLinux::NetworkStatsList networks = GetNetworks();
  for (const auto& network : networks)
  {
    eCAL::pb::mma::Network* nic = state.add_networks();
    nic->set_name(network.name);
    nic->set_send(network.send);
    nic->set_receive(network.receive);
  }

  ResourceLinux::Temperature temp = GetTemperature();

  if (temp.IsCPUTempValid() == true)
  {
    eCAL::pb::mma::Temperature temperature;
    temperature.set_cpu(temp.cpu_temperature);

    state.mutable_temperature()->CopyFrom(temperature);
  }
  else
  {
    state.clear_temperature();
  }
}
#endif  // __unix__
