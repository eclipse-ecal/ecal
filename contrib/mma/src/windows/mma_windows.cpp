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

#ifdef _WIN32

#define _WIN32_DCOM
#include <comdef.h>
#include <Wbemidl.h>
#include <functional>
#include <iostream>
#include <list>
#include <string>
#include <typeinfo>
#include <Windows.h>

#include "../include/windows/mma_windows.h"

#define EMPTY_STRING ""

namespace
{
  /**
  * @brief Extract value from PDH_HCOUNTER provided by Windows PDH API as double
  *
  * @param hnd         The PDH_HCOUNTER which monitors the performance
  * @param name        PDH_HCOUNTER name
  * @param out valout  The double value of the PDH_HCOUNTER
  *
  * @return  True if succeeded otherwise false
  **/

  bool ExtractDoubleVal(const PDH_HCOUNTER& hnd, const std::string& name, double& valout)
  {
    PDH_FMT_COUNTERVALUE fmt_value;

    PDH_STATUS status = PdhGetFormattedCounterValue(hnd, PDH_FMT_DOUBLE, nullptr, &fmt_value);

    if (status != ERROR_SUCCESS)
    {
      Logger::getLogger()->Log(name + ": Getting handle data failed with code " + std::to_string(status));
      return false;
    }

    valout = fmt_value.doubleValue;

    return true;
  }
}

MMAWindows::MMAWindows():
  init_done(false),
  operating_system(),
  nr_of_cpu_cores(),
  total_disks_(),
  total_network_cards_(),
  processes_(),
  memory_(),
  processor_(),
  query_manager_(),
  pdhStatus(),
  h_query_()
{
  init_done = init();
}

bool MMAWindows::init()
{
  try 
  {
    pdhStatus = PdhOpenQuery(nullptr, 0, &h_query_);
    CheckPDH("PdhOpenQuery", pdhStatus);

    processor_ = std::make_shared<Processor>();
    processor_->RefreshData(h_query_, pdhStatus);

    memory_ = std::make_shared<Memory>();

    Disk disk_resource;
    total_disks_ = disk_resource.GetResourceInfo(h_query_, pdhStatus, query_manager_);

    Network network_resource;
    total_network_cards_ = network_resource.GetResourceInfo(h_query_, pdhStatus, query_manager_);

    pdhStatus = PdhCollectQueryData(h_query_);

    CheckPDH("PdhCollectQueryData", pdhStatus);

    processes_ = std::make_shared<Processes>();
    nr_of_cpu_cores = processor_->GetNumbersOfCpuCores(); // the nr. of cores is static show we need to call once this function
    _Out_ LPTSTR lpBuffer = new char[4026];
    _In_  UINT   uSize = 0;
    if (GetSystemWow64Directory(lpBuffer, uSize) == 0)
    {
      operating_system.append("WINDOWS 32BITS");
    }
    else
    {
      operating_system.append("WINDOWS 64BITS");
    }
  }
  catch(...)
  {
    Logger::getLogger()->Log("Initialization failed");
    return false;
  }
  return true;
}

MMAWindows::~MMAWindows()
{
  if (init_done)
  {
    PdhCloseQuery(h_query_);
    CheckPDH("PdhCloseQuery", pdhStatus);
  }
}

bool MMAWindows::Get(eCAL::pb::mma::State& state)
{
  if (init_done)
  {
    try
    {
      state.clear_disks();
      state.clear_networks();

      Disk disk;
      auto disks = disk.GetResourceInfo(h_query_, pdhStatus, query_manager_);
      RefreshResource(total_disks_, disks);

      Network network;
      auto networks = network.GetResourceInfo(h_query_, pdhStatus, query_manager_);
      RefreshResource(total_network_cards_, networks);

      memory_->RefreshData();

      bool error_process_refrshing = processes_->RefeshData();
      SetProcessesData(state, error_process_refrshing);
 
      pdhStatus = PdhCollectQueryData(h_query_);
      CheckPDH("PdhCollectQueryData", pdhStatus);

      SetResourceData(state);

      CleanupResource(disks);
      CleanupResource(networks);

      return true;
    }
    catch (...)
    {
      Logger::getLogger()->Log("Error getting monitoring information");
      init_done = false;
    }
  }
  else
  {
    init_done = init();
  }
  return false;
}

void MMAWindows::SetResourceData(eCAL::pb::mma::State& state)
{
  // set the resource object data
  double cpu_load(0.0);
  ExtractDoubleVal(processor_->GetProcessor(), "Processor", cpu_load);
  state.set_cpu_load(cpu_load);
  state.set_number_of_cpu_cores(nr_of_cpu_cores);
  state.set_operating_system(operating_system);

  eCAL::pb::mma::Memory memory;
  memory.set_available(memory_->GetAvailableMemory());
  memory.set_total(memory_->GetTotalMemory());
  state.mutable_memory()->CopyFrom(memory);

  for (const auto& id : total_disks_)
  {
    double read_val   (0.0);
    bool   read_extr  (ExtractDoubleVal(id->GetRead(),  id->GetName(), read_val));
    double write_val  (0.0);
    bool   write_extr (ExtractDoubleVal(id->GetWrite(), id->GetName(), write_val));
    if (read_extr || write_extr)
    {
      eCAL::pb::mma::Disk* disk_logical = state.add_disks();
      disk_logical->set_name(id->GetName());
      disk_logical->set_read(read_val);
      disk_logical->set_write(write_val);
      disk_logical->set_available(id->GetAvailable_space());
      disk_logical->set_capacity(id->GetTotal_space());
    }
  }

  for (const auto& id : total_network_cards_)
  {
    eCAL::pb::mma::Network* network_card = state.add_networks();
    network_card->set_name(id->GetName());
    network_card->set_receive(id->GetReceiveNetworkSpeed());
    network_card->set_send(id->GetSendNetworkSpeed());
    network_card->set_ip_address(id->GetIP_address());
  }
}

void MMAWindows::SetProcessesData(eCAL::pb::mma::State& state, const bool no_error_occurred)
{
  ProcessMap_ptr_t process_map_ptr = processes_->GetProcessesInfo();
  state.clear_process();
  if ((process_map_ptr != nullptr) && (no_error_occurred == true))
  {
    eCAL::pb::mma::Process* temp_process = nullptr;
    for (auto it = process_map_ptr->cbegin(); it != process_map_ptr->cend(); ++it)
    {
      if (it->second.not_accessible == false)
      {
        temp_process = state.add_process();
        temp_process->set_id(it->first);
        temp_process->set_name(it->second.name);
        temp_process->set_commandline(it->second.command_line);
        temp_process->set_debug_flag(it->second.debug_flag);

        temp_process->mutable_cpu()->set_cpu_creation_time(it->second.cpu_creation_time);
        temp_process->mutable_cpu()->set_cpu_kernel_time(it->second.cpu_time_kernel);
        temp_process->mutable_cpu()->set_cpu_load(it->second.cpu_consumption);
        temp_process->mutable_cpu()->set_cpu_user_time(it->second.cpu_time_user);

        temp_process->mutable_memory()->set_current_working_set_size(it->second.memory_current);
        temp_process->mutable_memory()->set_peak_working_set_size(it->second.memory_peak);
      }
    }
  }
}

void MMAWindows::AddResource(const std::shared_ptr<Resource>& resource)
{
  std::shared_ptr<Resource> resource_buffer;
  std::string object_type = typeid(*resource).name();

  if (object_type == "class Disk")
  {
    resource_buffer = std::make_shared<Disk>();
  }
  else
  {
    if (object_type == "class Network")
    {
      resource_buffer = std::make_shared<Network>();
    }
  }

  if (resource_buffer != nullptr)
  {
    resource_buffer->SetName(resource->GetName());
    resource_buffer->SetDrive_type(resource->GetDrive_type());
    resource_buffer->SetAvailable_space(resource->GetAvailable_space());
    resource_buffer->SetTotal_space(resource->GetTotal_space());
    resource_buffer->SetIP_address(resource->GetIP_address());
    resource_buffer->SetRead(resource->GetRead());
    resource_buffer->SetWrite(resource->GetWrite());
  }

  if (object_type == "class Disk")
    total_disks_.push_back(resource_buffer);
  else
    if (object_type == "class Network")
      total_network_cards_.push_back(resource_buffer);
}

void MMAWindows::RefreshResource(std::list<std::shared_ptr<Resource>>& destination, const std::list<std::shared_ptr<Resource>>& source)
{
  for (auto it = destination.begin(); it != destination.end();)
  {
    if (!Resource::ResourceExists(source, (*it)->GetName()))
    {
      if (query_manager_.RemoveQuery((*it)->GetName()))
      {
        Logger::getLogger()->Log("Query for : " + (*it)->GetName() + " was removed from query manager.");
      }
      PdhRemoveCounter((*it)->GetRead());
      PdhRemoveCounter((*it)->GetWrite());

      Logger::getLogger()->Log((*it)->GetName() + " resource was removed.");

      it = destination.erase(it);
    }
    else
    {
      ++it;
    }
  }

  for (const auto& it : source)
  {
    auto found = Resource::FindResource(destination, it->GetName());

    if (found == destination.end())
    {
      Logger::getLogger()->Log("A new resource was added: " + it->GetName());
      AddResource(it);
    }
    else
    {
      (*found)->SetName(it->GetName());
      (*found)->SetDrive_type(it->GetDrive_type());
      (*found)->SetTotal_space(it->GetTotal_space());
      (*found)->SetAvailable_space(it->GetAvailable_space());
      (*found)->SetIP_address(it->GetIP_address());
      (*found)->SetReceiveNetworSpeed(it->GetReceiveNetworkSpeed());
      (*found)->SetSendNetworkSpeed(it->GetSendNetworkSpeed());
    }
  }
}

void MMAWindows::CleanupResource(std::list<std::shared_ptr<Resource>>& resources)
{
  resources.clear();
}

void MMAWindows::CheckPDH(const char* PDH, const PDH_STATUS& status)
{
  std::string pdh(PDH);
  if (status != ERROR_SUCCESS)
  {
    std::cout << pdh + " failed with code " << status << std::endl;
    Logger::getLogger()->Log(pdh + " failed with code " + std::to_string(status));
  }

  std::string mma_home;

  if (GetEnv("MMA_HOME", mma_home))
  {
    int perfmon_not_valid_errorcode = -2147481643;
    std::string perfmon_not_valid = "Performance counters are broken. \
                                   Please rebuild performance counters according to the perfmon_error_rebuild.txt \
                                   found at: " + mma_home;

    if (status == perfmon_not_valid_errorcode)
    {
      std::cout << perfmon_not_valid << std::endl;
      Logger::getLogger()->Log(perfmon_not_valid);
    }
  }
}

bool MMAWindows::GetEnv(const char* env_name, std::string& env_path)
{
  bool return_value = true;
  const char* mma = std::getenv(env_name);
  if (mma == nullptr)
    return_value = false;
  else
    env_path = std::string(mma);

  return return_value;
}

#endif  // _WIN32
