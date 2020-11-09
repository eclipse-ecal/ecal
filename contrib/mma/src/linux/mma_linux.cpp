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

#include "../include/linux/mma_linux.h"

#define B_IN_KB 1024.0
#define B_IN_MB 1048576.0

MMALinux::MMALinux():
  cpu_pipe_(std::make_unique<PipeRefresher>("sar -u 1 1",500)),
  network_pipe_(std::make_unique<PipeRefresher>("ifstat 1 1",500)),
  disk_pipe_(std::make_unique<PipeRefresher>("iostat -N 1 2",500)),
  process_pipe_(std::make_unique<PipeRefresher>("ps -aux",500))
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
  if (command == "sar -u 1 1")
    cpu_pipe_result_ = pipe_result;
  if (command == "ifstat 1 1")
    network_pipe_result_ = pipe_result;
  if (command == "iostat -N 1 2")
    disk_pipe_result_ = pipe_result;
  if (command == "ps -aux")
    process_pipe_result_ = pipe_result;

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
  {
    std::lock_guard<std::mutex> guard(mutex);
    local_copy = cpu_pipe_result_;
  }

  double cpu_time = 0.0;

  auto lines = TokenizeIntoLines(local_copy);
  if (lines.size() > 0)
  {
    std::vector<std::string> cpu_line = SplitLine(lines.back());
    double idle_time = static_cast<double>(stod(cpu_line.back()));

    cpu_time = 100.0 - idle_time;
  }
  else
  {
    cpu_time = 0.0;
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
  {
    std::lock_guard<std::mutex> guard(mutex);
    local_copy = network_pipe_result_;
  }

  ResourceLinux::NetworkStatsList networks;
  ResourceLinux::Network network_stats;

  auto lines = TokenizeIntoLines(local_copy);
  if (lines.size() > 0)
  {
    std::vector<std::string> network_names = SplitLine(lines.front());

    // skip the first 2 lines
    auto iterator = lines.begin();
    std::advance(iterator, 2);

    std::vector<std::string> network_io = SplitLine(*iterator);

    int kb_in_index = 0;
    int kb_out_index = 1;

    size_t size = network_names.size();
    for (size_t i = 0; i < size; i++)
    {
      network_stats.name = network_names[i];
      try { network_stats.receive = static_cast<double>(stod(network_io[kb_in_index])); }
      catch (...) { network_stats.receive = 0.00; }

      try { network_stats.send = static_cast<double>(stod(network_io[kb_out_index])); }
      catch (...) { network_stats.send = 0.00; }

      kb_in_index += 2;
      kb_out_index += 2;

      network_stats.receive *= B_IN_KB;
      network_stats.send *= B_IN_KB;
      networks.push_back(network_stats);
    }
  }

  return networks;
}
ResourceLinux::ProcessStatsList MMALinux::GetProcesses()
{
  std::string local_copy;
  {
    std::lock_guard<std::mutex> guard(mutex);
    local_copy = process_pipe_result_;
  }

  ResourceLinux::ProcessStatsList processes;
  ResourceLinux::Process process_stats;

  auto lines = TokenizeIntoLines(local_copy);
  if (lines.size() > 0)
  {
    // skip the first 1 lines
    auto iterator = lines.begin();
    std::advance(iterator, 1);
    while(iterator!=lines.end())
    {
      std::vector<std::string> process_data = SplitLine(*iterator);
      process_stats.id=std::stoi(process_data[1]);
      process_stats.user=process_data[0];
      process_stats.memory.current_used_memory=stoi(process_data[5]);
      process_stats.commandline=process_data[10];
      process_stats.cpu_load.cpu_load=std::stof(process_data[2]);
      processes.push_back(process_stats);
      std::advance(iterator, 1);
    }
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

    // if last character of string is digit, delete the digit
    if (IsDigit(name.back()))
    {
      name.pop_back();
    }

    *iterator = name;

    // format the path to include just the name
    std::size_t found = name.find_last_of("/");
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
  OpenPipe("df -B1", result);

  // partitions for disks
  std::map<std::string, std::vector<std::string>> partition_map;

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
        if ((partition.back() == root) || (partition.back() == home) || (partition.back().find(media) != std::string::npos) || (partition.back() == boot))
        {
          partition_map[partition.back()] = partition;
        }
      }
      else
      {
        return_value = true;
      }
    }
  }

  for (const auto& partition : partition_map)
  {
    ResourceLinux::DiskStats disk_stats;
    disk_stats.name = partition.second[0];
    uint64_t used = stoll(partition.second[1]);
    disk_stats.available = stoll(partition.second[2]);
    disk_stats.capacity = disk_stats.available + used;
    disk_stats.mount_point = partition.second[3];

    disks.push_back(disk_stats);
  }

#ifdef __arm__
  return_value |= MergeBootWithRootARM(disks);
#endif

  return return_value;
}

bool MMALinux::SetDiskIOInformation(ResourceLinux::DiskStatsList& disk_stats_info)
{
  std::string local_copy;
  {
    std::lock_guard<std::mutex> guard(mutex);
    local_copy = disk_pipe_result_;
  }

  bool return_value = true;

  auto lines = TokenizeIntoLines(local_copy);

  auto it = lines.begin();
  std::advance(it, (lines.size() / 2) + 4);
  lines.erase(lines.begin(), it);

  for (const auto& line : lines)
  {
    std::vector<std::string> partition = SplitLine(line);

    for (auto& disk : disk_stats_info)
    {
      if (disk.name.find(partition[0]) != std::string::npos)
      {
        try { disk.read = static_cast<double>(stod(partition[2])); }
        catch (...) { disk.read = 0.00; }

        try { disk.write = static_cast<double>(stod(partition[3])); }
        catch (...) { disk.write = 0.00; }


        disk.read *= B_IN_KB;
        disk.write *= B_IN_KB;
      }
    }
  }

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
  std::list <std::string> cpuinfo_list = TokenizeIntoLines(FileToString("/proc/cpuinfo"));
  std::vector <std::string> cpucore_vect;
  auto idx = 0;
  if(!is_vrm)
  {
    for (auto iter : cpuinfo_list)
    {
      if (idx == 12)
      {
        cpucore_vect = SplitLine(iter);
        break;
      }
      idx++;
    }
    return std::stoi(cpucore_vect[3]);
  }
  else
  {
    auto cpu_cores=0;
    for (auto iter : cpuinfo_list)
    {
        cpucore_vect = SplitLine(iter);
        for(auto iter1:cpucore_vect)
        {
            if(iter1.find("processor")!= std::string::npos)
            {
                cpu_cores++;
            }
        }
    }
   return cpu_cores;
  }
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
