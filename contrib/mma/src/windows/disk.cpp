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

#include <iostream>
#include <list>
#include <regex>
#include <string>

#include "../include/windows/disk.h"

Disk::Disk():
  resource_read_info("[LogicalDisk Read]"),
  resource_write_info("[LogicalDisk Write]")
{
  pipe_command_ = "wmic logicaldisk list brief / format:list";
}

Disk::~Disk()
{
}

std::list<std::shared_ptr<Resource>> Disk::GetResourceInfo(const HQUERY& h_query_, PDH_STATUS& pdhStatus, QueryManager& query_manager, std::string type /*= "2|3"*/)
{
  std::list<std::shared_ptr<Resource>> disk_info_list;

  std::string result = OpenPipe(pipe_command_);
  std::smatch match;
  std::regex expression("DeviceID=(.*)\r\nDriveType=(" + type + ")\r\nFreeSpace=(.*)\r\n.*\r\nSize=(.*)");

  while (std::regex_search(result, match, expression))
  {
    if (match.size() > 4)
    {
      std::shared_ptr<Resource> disk_buffer = std::make_shared<Disk>();

      try
      {
        disk_buffer->SetName(match.str(1));
      }
      catch (...)
      {
        disk_buffer->SetName("Uninitialized.");
        Logger::getLogger()->Log("Disk name conversion failed.");
      }

      try
      {
        disk_buffer->SetDrive_type(static_cast<size_t>(stoi(match.str(2))));
      }
      catch (...)
      {
        disk_buffer->SetDrive_type(static_cast<size_t>(-1));
        Logger::getLogger()->Log("Disk drive type conversion failed.");
      }

      try
      {
        disk_buffer->SetAvailable_space(static_cast<uint64_t>(stoull(match.str(3))));
      }
      catch (...)
      {
        disk_buffer->SetAvailable_space(static_cast<uint64_t>(-1));
        Logger::getLogger()->Log("Disk available space conversion failed.");
      }

      try
      {
        disk_buffer->SetTotal_space(static_cast<uint64_t>(stoull(match.str(4))));
      }
      catch (...)
      {
        disk_buffer->SetTotal_space(static_cast<uint64_t>(-1));
        Logger::getLogger()->Log("Disk total space conversion failed.");
      }

      RefreshCounters(disk_buffer, pdhStatus, h_query_, query_manager);

      disk_info_list.push_back(disk_buffer);
    }
    result = match.suffix().str();
  }

  return disk_info_list;
}

void Disk::RefreshCounters(std::shared_ptr<Resource> resource, PDH_STATUS& status, const HQUERY& h_query_, QueryManager& query_manager)
{
  const std::string resource_name       (resource->GetName());
  const std::string read_query          ("\\LogicalDisk(" +   resource_name + ")\\Disk Read Bytes/sec");
  const std::string write_query         ("\\LogicalDisk(" +  resource_name + ")\\Disk Write Bytes/sec");


  if (!query_manager.QueryExists(read_query))
  {
    Logger::getLogger()->Log(resource_name + " added");
    query_manager.AddQuery(read_query);
    status = PdhAddEnglishCounter(h_query_, read_query.c_str(), 0, &read_);
    resource->SetRead(read_);
    CheckPDH(resource_name + resource_read_info, status);
  }
  else
  {
    resource->SetRead(read_);
    CheckPDH(resource_name + resource_read_info, status);
  }

  if (!query_manager.QueryExists(write_query))
  {
    query_manager.AddQuery(write_query);
    status = PdhAddEnglishCounter(h_query_, write_query.c_str(), 0, &write_);
    resource->SetWrite(write_);
    CheckPDH(resource_name + resource_write_info, status);
  }
  else
  {
    resource->SetWrite(write_);
    CheckPDH(resource_name + resource_write_info, status);
  }
}
