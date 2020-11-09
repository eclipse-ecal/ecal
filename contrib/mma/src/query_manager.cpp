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

#include "../include/query_manager.h"

QueryManager::QueryManager()
{
}


QueryManager::~QueryManager()
{
}

void QueryManager::AddQuery(const std::string& name)
{
  query_list_.push_back(name);
}

bool QueryManager::QueryExists(const std::string& name)
{
  bool return_value = false;
  
  for (const auto& query : query_list_)
  {
    if (query == name)
      return_value = true;
  }
  
  return return_value;
}

bool QueryManager::RemoveQuery(const std::string& name)
{
  bool return_value = false;

  for (auto iterator = query_list_.begin(); iterator != query_list_.end();)
  {
    if (iterator->find(name) != std::string::npos)
    {
      iterator = query_list_.erase(iterator);
      return_value = true;
      break;
    }
    else
    {
      ++iterator;
    }
  }

  return return_value;
}
