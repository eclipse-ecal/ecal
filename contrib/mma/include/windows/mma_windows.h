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
#pragma  once

#include <Pdh.h>
#include <winioctl.h>
#include <commctrl.h>

#include <map>
#include <list>
#include <string>
#include <atomic>

#include "disk.h"
#include "memory.h"
#include "network.h"
#include "processor.h"
#include "ressource.h"
#include "processes.h"

#include "../logger.h"
#include "../mma_impl.h"
#include "../query_manager.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include "ecal/pb/mma/mma.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

class MMAWindows : public MMAImpl
{
 public:
  /**
 * @brief  Constructor.
 **/
  MMAWindows();

  /**
  * @brief Destructor.
  **/
  ~MMAWindows();

  /**
  * @brief  Get machine statistics: CPU, Memory, Disk, Network
  *
  * @param state  Machine state reference
  *
  * @return true if successful, false otherwise
  **/
  bool Get(eCAL::pb::mma::State& state);

 private:
  HQUERY                                h_query_;
  PDH_STATUS                            pdhStatus;

  QueryManager                          query_manager_;
  std::shared_ptr<Processor>            processor_;
  std::shared_ptr<Memory>               memory_;
  std::shared_ptr<Processes>            processes_;

  std::list<std::shared_ptr<Resource>>  total_network_cards_;
  std::list<std::shared_ptr<Resource>>  total_disks_;
  int                                   nr_of_cpu_cores;
  std::string                           operating_system;

  std::atomic<bool>                     init_done;

  /**
  * @brief Set the MachineMonitoring::State object data
  *
  * @param resource   The resource object which will contain the data, e.g: cpu_load, memory, disks, network
  *
  **/
  void SetResourceData(eCAL::pb::mma::State& resource);

  void SetProcessesData(eCAL::pb::mma::State& state, const bool no_error_occurred);

  void AddResource(const std::shared_ptr<Resource>& resource);
  void RefreshResource(std::list<std::shared_ptr<Resource>>& destination, const std::list<std::shared_ptr<Resource>>& source);
  void CleanupResource(std::list<std::shared_ptr<Resource>>& resources);
  void CheckPDH(const char* PDH, const PDH_STATUS& status);
  bool GetEnv(const char* env_name, std::string& env_path);
  bool init();

};

#endif  // _WIN32
