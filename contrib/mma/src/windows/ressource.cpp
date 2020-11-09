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

#include <list>
#include <string>

#include "../include/windows/ressource.h"

Resource::Resource():name_(""), drive_type_(0), available_space_(0), total_space_(0),
ip_address_(""), read_(nullptr), write_(nullptr), receive_network_speed(0), send_network_speed(0),
pipe_command_(nullptr)
{
}

Resource::~Resource()
{
}

bool Resource::ResourceExists(const std::list<std::shared_ptr<Resource>>& resources, const std::string& name)
{
  for (auto& resource : resources)
  {
    if (name == resource->GetName())
      return true;
  }
  return false;
}

std::list<std::shared_ptr<Resource>>::iterator Resource::FindResource(std::list<std::shared_ptr<Resource>>& resources, const std::string& name)
{
  for (auto it = resources.begin(); it != resources.end(); ++it)
  {
    if (name == (*it)->GetName())
      return it;
  }

  return resources.end();
}

std::string Resource::OpenPipe(const char* command)
{
  FILE* file = _popen(command, "rt");
  std::string result;
  static char buffer[16384];  // Buffer for the pipe result
  if (file != nullptr)
  {
    while (fgets(buffer, sizeof(buffer), file) != nullptr)
      result += buffer;

    _pclose(file);
  }
  strcpy_s(buffer, "");

  return result;
}

void Resource::CheckPDH(const std::string& PDH, const PDH_STATUS& status)
{
  if (status != ERROR_SUCCESS)
  {
    std::string pdh(PDH);
    Logger::getLogger()->Log(pdh + " failed with code " + std::to_string(status));
  }
}
