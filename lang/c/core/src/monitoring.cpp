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

/**
 * @file   monitoring.cpp
 * @brief  eCAL monitoring c interface
**/

#include <ecal/ecal.h>
#include <ecal_c/monitoring.h>
#include <ecal_c/types/monitoring.h>

#include "common.h"

#include <map>
#include <cassert>

namespace
{
  unsigned int Convert_Entities(unsigned int entities_c_)
  {
    unsigned int entities{ 0 };
    static const std::map<unsigned int, unsigned int> entity_map
    {
        {eCAL_Monitoring_Entity_None, eCAL::Monitoring::Entity::None},
        {eCAL_Monitoring_Entity_Publisher, eCAL::Monitoring::Entity::Publisher},
        {eCAL_Monitoring_Entity_Subscriber, eCAL::Monitoring::Entity::Subscriber},
        {eCAL_Monitoring_Entity_Server, eCAL::Monitoring::Entity::Server},
        {eCAL_Monitoring_Entity_Client, eCAL::Monitoring::Entity::Client},
        {eCAL_Monitoring_Entity_Process, eCAL::Monitoring::Entity::Process},
        {eCAL_Monitoring_Entity_Host, eCAL::Monitoring::Entity::Host},
    };

    decltype(entities_c_) bit_mask = 1 << 0;
    for (std::size_t i = 0; i < sizeof(decltype(entities_c_)) * 8; ++i)
    {
      entities |= entity_map.at(bit_mask & entities_c_);
      bit_mask <<= 1;
    }

    return entities;
  }
}

#if ECAL_CORE_MONITORING
extern "C"
{
  ECALC_API int eCAL_Monitoring_GetMonitoring(void** monitoring_, size_t* monitoring_length_, const unsigned int* entities_)
  {
    assert(monitoring_ != NULL && monitoring_length_ != NULL);
    assert(*monitoring_ == NULL && *monitoring_length_ == 0);

    std::string buffer;
    if (eCAL::Monitoring::GetMonitoring(buffer, entities_ != NULL ? Convert_Entities(*entities_) : eCAL::Monitoring::Entity::All))
    {
      *monitoring_ = std::malloc(buffer.size());
      if (*monitoring_ != NULL)
      {
        std::memcpy(*monitoring_, buffer.data(), buffer.size());
        *monitoring_length_ = buffer.size();
      }
    }
    return !static_cast<int>(*monitoring_ != NULL);
  }
}
#endif // ECAL_CORE_MONITORING
