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
 * @file   ecal_monitoring_pb.h
 * @brief  eCAL monitoring interface using protobuf
**/

#pragma once

#include <ecal/ecal_os.h>
#include <ecal/ecal_monitoring_entity.h>

#ifdef _MSC_VER
#pragma warning(push, 0) // disable proto warnings
#endif
#include <ecal/core/pb/monitoring.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace eCAL
{
  namespace Monitoring
  {
    /**
     * @brief Get monitoring as protobuf message.
     *
     * @param [out] mon_       Protobuf message to store the monitoring information.
     * @param       entities_  Entities definition.
     *
     * @return Zero if succeeded.
    **/
    ECAL_API int GetMonitoring(eCAL::pb::Monitoring& mon_, unsigned int entities_ = Entity::All);

    /**
     * @brief Get logging as protobuf message.
     *
     * @param [out] log_  Protobuf message to store the logging information.
     *
     * @return Zero if succeeded.
    **/
    ECAL_API int GetLogging(eCAL::pb::Logging log_);
  }
}
