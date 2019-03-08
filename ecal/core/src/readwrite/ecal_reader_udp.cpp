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
 * @brief  eCAL udp reader
**/

#include "ecal_global_accessors.h"

#include "readwrite/ecal_reader_udp.h"
#include "pubsub/ecal_subgate.h"

namespace eCAL
{
  //////////////////////////////////////////////////////////////////
  // CDataReaderUDP
  //////////////////////////////////////////////////////////////////
  bool CDataReaderUDP::HasSample(const std::string& sample_name_)
  {
    if (!g_subgate()) return(false);
    return(g_subgate()->HasSample(sample_name_));
  }

  size_t CDataReaderUDP::ApplySample(const eCAL::pb::Sample& ecal_sample_, eCAL::pb::eTLayerType layer_)
  {
    if (!g_subgate()) return 0;
    return g_subgate()->ApplySample(ecal_sample_, layer_);
  }
};
