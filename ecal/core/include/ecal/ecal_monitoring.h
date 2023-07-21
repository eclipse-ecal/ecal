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
 * @file   ecal_monitoring.h
 * @brief  eCAL monitoring interface
**/

#pragma once

#include <ecal/ecal_os.h>
#include <ecal/ecal_deprecate.h>
#include <ecal/types/monitoring.h>
#include <string>

namespace eCAL
{
  namespace Monitoring
  {
    /**
     * @brief Set topics filter blacklist regular expression.
     *
     * @param filter_  Topic filter as regular expression. 
     *
     * @return Zero if succeeded.
    **/
    ECAL_API int SetExclFilter(const std::string& filter_);

    /**
     * @brief Set topics filter whitelist regular expression.
     *
     * @param filter_  Topic filter as regular expression. 
     *
     * @return Zero if succeeded.
    **/
    ECAL_API int SetInclFilter(const std::string& filter_);

    /**
     * @brief Switch topics filter using regular expression on/off.
     *
     * @param state_  Filter on / off state. 
     *
     * @return Zero if succeeded.
    **/
    ECAL_API int SetFilterState(bool state_);

    /**
     * @brief Get monitoring as serialized protobuf string. 
     *
     * @param [out] mon_  String to store the monitoring information. 
     *
     * @return  Monitoring buffer length or zero if failed. 
    **/
    ECAL_API int GetMonitoring(std::string& mon_);

    /**
     * @brief Get monitoring subset as serialized protobuf string.
     *
     * @param [out] mon_       String to store the monitoring information. 
     * @param       entities_  Entities to get.
     *
     * @return Zero if succeeded.
    **/
    ECAL_API int GetMonitoring(std::string& mon_, unsigned int entities_);
    
    /**
     * @brief Get monitoring as a struct.
     *
     * @param [out] mon_       Target struct to store monitoring information.
     * @param       entities_  Entities definition.
     *
     * @return Number of struct elements if succeeded.
    **/
    ECAL_API int GetMonitoring(eCAL::Monitoring::SMonitoring& mon_, unsigned int entities_ = Entity::All);
    

    /**
     * @brief Get logging as serialized protobuf string. 
     *
     * @param [out] log_  String to store the logging information. 
     *
     * @return  Monitoring buffer length or zero if failed. 
    **/
    ECAL_API int GetLogging(std::string& log_);

    /**
     * @brief Publish monitoring protobuf message (deprecated).
     *
     * @param state_  Switch publishing on/off.
     * @param name_   Monitoring topic name.
     *
     * @return Zero if succeeded.
    **/
    ECAL_DEPRECATE_SINCE_5_12("use GetMonitoring and publish yourself")
    ECAL_API int PubMonitoring(bool state_, std::string name_ = "ecal.monitoring");

    /**
     * @brief Publish logging protobuf message (deprecated).
     *
     * @param state_  Switch publishing on/off.
     * @param name_   Logging topic name.
     *
     * @return Zero if succeeded.
    **/
    ECAL_DEPRECATE_SINCE_5_12("use GetLogging and publish yourself")
    ECAL_API int PubLogging(bool state_, std::string name_ = "ecal.logging");
  }
  /** @example monitoring_rec.cpp
  * This is an example how the eCAL Monitoring API may be utilized to print monitoring information.
  */
}
