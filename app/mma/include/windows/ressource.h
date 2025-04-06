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

#pragma once

#include <Pdh.h>

#include <cstdint>
#include <list>
#include <memory>
#include <string>
#include <time.h>
#include <stdio.h>
#include <string.h>

#include "../query_manager.h"
#include "../logger.h"

class Resource
{
 public:
  Resource();
  virtual ~Resource();

  std::string GetName() const { return name_; }
  void SetName(const std::string& val) { name_ = val; }

  size_t GetDrive_type() const { return drive_type_; }
  void SetDrive_type(size_t val) { drive_type_ = val; }

  uint64_t GetAvailable_space() const { return available_space_; }
  void SetAvailable_space(uint64_t val) { available_space_ = val; }

  uint64_t GetTotal_space() const { return total_space_; }
  void SetTotal_space(uint64_t val) { total_space_ = val; }

  std::string GetIP_address() const { return ip_address_; }
  void SetIP_address(const std::string& val) { ip_address_ = val; }

  PDH_HCOUNTER GetRead() const { return read_; }
  void SetRead(PDH_HCOUNTER val) { read_ = val; }

  PDH_HCOUNTER GetWrite() const { return write_; }
  void SetWrite(PDH_HCOUNTER val) { write_ = val; }

  double  GetReceiveNetworkSpeed( ) const { return receive_network_speed; }
  void SetReceiveNetworSpeed(double  val) { receive_network_speed = val; }

  double  GetSendNetworkSpeed( ) const { return send_network_speed; }
  void SetSendNetworkSpeed(double val) { send_network_speed = val; }
  
  static bool ResourceExists(const std::list<std::shared_ptr<Resource>>& resources, const std::string& name);
  static std::list<std::shared_ptr<Resource>>::iterator FindResource(std::list<std::shared_ptr<Resource>>& resources, const std::string& name);
  std::string OpenPipe(const char* command);

 protected:
  std::string name_;                 // Name of the resource, e.g: DeviceID of the disk (C:), or NIC name
  size_t      drive_type_;           // Drive type, e.g: 2 (Removable Drive), 3(Local Hard Disk)
  uint64_t    available_space_;      // Available space on the drive
  uint64_t    total_space_;          // Total space of the drive
  std::string ip_address_;           // IP address, only in the case of network interface cards

  PDH_HCOUNTER read_;                // Bytes read per second (disk) 
  PDH_HCOUNTER write_;               // Bytes written per second (disk) 
  double       receive_network_speed;//Bytes received per second
  double       send_network_speed;   // Bytes sent per second 

  const char* pipe_command_;         // String containing the pipe command

  virtual std::list<std::shared_ptr<Resource>> GetResourceInfo(const HQUERY& h_query_, PDH_STATUS& pdhStatus, QueryManager& query_manager, std::string type = "2|3") = 0;
  void CheckPDH(const std::string& PDH, const PDH_STATUS& status);
};
