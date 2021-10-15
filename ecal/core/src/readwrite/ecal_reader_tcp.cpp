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
 * @brief  tcp reader
**/

#include "ecal_def.h"
#include "ecal_config_hlp.h"
#include "ecal_global_accessors.h"
#include "pubsub/ecal_subgate.h"

#include "readwrite/ecal_writer_base.h"
#include "readwrite/ecal_reader_tcp.h"

namespace eCAL
{
  template<> std::shared_ptr<CTCPLayer> CReaderLayer<CTCPLayer>::layer(nullptr);

  //////////////////////////////////////////////////////////////////
  // CDataReaderTCP
  //////////////////////////////////////////////////////////////////
  CDataReaderTCP::CDataReaderTCP() = default;

  CTCPLayer::CTCPLayer()
  {
  }

  void CTCPLayer::Initialize()
  {
  }
    
  void CTCPLayer::AddSubscription(std::string& topic_name_, std::string& topic_id_, QOS::SReaderQOS /*qos_*/)
  {
  }

  void CTCPLayer::RemSubscription(std::string& topic_name_, std::string& topic_id_)
  {
  }

  void CTCPLayer::SetConnectionParameter(SReaderLayerPar& par_)
  {
    eCAL::pb::ConnnectionPar connection_par;
    if (connection_par.ParseFromString(par_.parameter))
    {
      //////////////////////////////////
      // get parameter from a new writer
      //////////////////////////////////
      // topic name
      auto topic_name = par_.topic_name;
      // topic id
      auto topic_id   = par_.topic_id;
      // host name
      auto host_name  = par_.host_name;
      // port
      auto port = connection_par.layer_par_tcp().port();
    }
    else
    {
      std::cout << "FATAL ERROR: Could not parse layer connection parameter ! Did you mix up different eCAL versions on the same host ?" << std::endl;
      return;
    }
  }

  bool CDataReaderTCP::Create(const std::string& topic_name_)
  {
    // store topic name
    m_topic_name = topic_name_;

    return true;
  }

  bool CDataReaderTCP::Destroy()
  {
    //if(!m_subscriber) return false;
    
	return true;
  }
}
