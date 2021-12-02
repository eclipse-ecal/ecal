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
 * @brief  eCAL inproc reader (this is a skeleton only this time)
**/

#include "readwrite/ecal_reader_inproc.h"

namespace eCAL
{
  template<> std::shared_ptr<CInProcLayer> CReaderLayer<CInProcLayer>::layer(nullptr);

  CInProcLayer::CInProcLayer()
  {
  }

  CInProcLayer::~CInProcLayer()
  {
  }

  // initialize layer
  // will be called one time on eCAL initialization
  void CInProcLayer::Initialize()
  {
  }

  // start layer
  // activate / create a inproc subscription with defined 
  // quality of service settings if supported
  void CInProcLayer::AddSubscription(const std::string& /*host_name_*/, const std::string& /*topic_name_*/, const std::string& /*topic_id_*/, QOS::SReaderQOS /*qos_*/)
  {
  }

  // stop layer
  // deactivate / destroy a specific inproc subscription
  void CInProcLayer::RemSubscription(const std::string& /*host_name_*/, const std::string& /*topic_name_*/, const std::string& /*topic_id_*/)
  {
  }

  void CInProcLayer::SetConnectionParameter(SReaderLayerPar& /*par_*/)
  {
  }

  //////////////////////////////////////////////////////////////////
  // CDataReaderInProc
  //////////////////////////////////////////////////////////////////
  CDataReaderInProc::CDataReaderInProc()
  {
  }

  CDataReaderInProc::~CDataReaderInProc()
  {
  }
}
