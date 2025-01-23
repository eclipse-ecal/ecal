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
 * @file   monitoring.h
 * @brief  eCAL monitoring interface
**/

#pragma once

#include <ecal/os.h>
#include <ecal/types/monitoring.h>
#include <string>

namespace eCAL
{
  namespace Monitoring
  {
    /**
     * @brief Get monitoring subset as serialized protobuf string.
     *
     * @param [out] mon_       Target string to store the monitoring information. 
     * @param       entities_  Entities to get.
     *
     * @return True if succeeded.
    **/
    ECAL_API bool GetMonitoring(std::string& mon_, unsigned int entities_ = Entity::All);
    
    /**
     * @brief Get monitoring as a struct.
     *
     * @param [out] mon_       Target struct to store the monitoring information.
     * @param       entities_  Entities definition.
     *
     * @return True if succeeded.
    **/
    ECAL_API bool GetMonitoring(SMonitoring& mon_, unsigned int entities_ = Entity::All);
  }
  /** @example monitoring_rec.cpp
  * This is an example how the eCAL Monitoring API may be utilized to print monitoring information.
  */
}

